#include "player_manage.h"
#include "client_handler.h"
#include "utils.h"

extern Node_Client* head;
extern char last_color[3];
extern char prox_RGB;
extern pthread_rwlock_t rwlock_stack_head;
extern pthread_mutex_t mutex_color;
extern pthread_rwlock_t rwlock_stack;
extern pthread_rwlock_t rwlock_score;
extern Score_List score;
extern int n_players;

//Função geral que faz o processo completo de criar um jogador
void createPlayer(int client_fd){
	pthread_t thread_id;
	Node_Client* client_data = NULL;

	client_data = (Node_Client*)malloc(sizeof(Node_Client));
	verifyErr(client_data);

	client_data->sock_fd = client_fd;

	if(pthread_create(&thread_id, NULL, (void*)playerHandler, (void*)client_data)){			//Verificar se não houve erro a criar threads
		fprintf(stderr, "Couldn't create thread\n");
		exit(-1);
	}
	if(pthread_detach(thread_id)){															//Fazer detach na thread criada
		fprintf(stderr, "Couldn't detach thread\n");
		exit(-1);
	}

}

//Adiciona um nó na lista de players
void insertPlayer(Node_Client* aux){

	aux->prev = NULL;								//Meter a NULL pq inserimos pela cabeça

    //Sync para a head da pilha
    pthread_rwlock_wrlock(&rwlock_stack_head);
	if(head != NULL){
		head->prev = aux;
	}

    aux->next = head;
    head = aux;
	n_players++;								//Incrementar a variável que contém o numero de jogadores online
    pthread_rwlock_unlock(&rwlock_stack_head);
}

//Apaga 1 elemento da lista de clientes, a função foi escrita de modo a que thread tenha os locks o menos tempo possível 
void deleteNode(Node_Client* deletingNode){

	//Ver se estamos a apagar a head, para isso temos que dar lock na rwlock da head
	pthread_rwlock_rdlock(&rwlock_stack_head);
	if(deletingNode == head){				//deletingNode = head
		pthread_rwlock_unlock(&rwlock_stack_head);
		
		pthread_rwlock_wrlock(&rwlock_stack_head);
		if(head->next != NULL){				//Ver se não estamos a apagar o único elemento da lista (simultaneamente head e tail)
			pthread_rwlock_wrlock(&rwlock_stack);
			head->next->prev = NULL;
		}else{
			pthread_rwlock_wrlock(&rwlock_stack);
		}
		head = head->next;
		n_players--;										//Decrementar a variável que contém o numero de jogadores online
		pthread_rwlock_unlock(&rwlock_stack);
		pthread_rwlock_unlock(&rwlock_stack_head);
		free(deletingNode);
		return;
	}

	pthread_rwlock_unlock(&rwlock_stack_head);

	//Se não tivermos a apagar a head, temos de dar lock no rwlock geral do stack
	pthread_rwlock_wrlock(&rwlock_stack);
	if(deletingNode->next == NULL){			//deletingNode = último node
		deletingNode->prev->next = NULL;
	}else{									//deletingNode = node do meio do stack
		deletingNode->prev->next = deletingNode->next;
		deletingNode->next->prev = deletingNode->prev;
	}
	n_players--;										//Decrementar a variável que contém o numero de jogadores online
	pthread_rwlock_unlock(&rwlock_stack);

	close(deletingNode->sock_fd);
	free(deletingNode);			//free fora da região crítica porque não é necessário estar
}

//Apaga a lista de clientes
void deleteList(){
    Node_Client* aux;
    Node_Client* next;

	pthread_rwlock_wrlock(&rwlock_stack);
    for(aux = head; aux != NULL; aux = next){
        next = aux->next;
		shutdown(aux->sock_fd, SHUT_RDWR);				//Serve somente para a thread correspondente à socket ficar desbloqueada no read()
		close(aux->sock_fd);
        free(aux);
    }
	n_players = 0;
	pthread_rwlock_unlock(&rwlock_stack);
}

//Gera uma cor com uma ordem predefinida -> este algoritmo gera 34 cores diferentes, logo, o numero max de jogadores é 34
void generateColor(char color[]){
	pthread_mutex_lock(&mutex_color);
	switch(prox_RGB){
	case 0:
		last_color[0] += 20;
		prox_RGB = 1;
		break;

	case 1:
		last_color[1] += 20;
		prox_RGB  = 2;
		break;

	case 2:
		last_color[2] += 20;
		prox_RGB  = 3;
		break;

	case 3:
		last_color[2] += 20;
		prox_RGB  = 4;
		break;

	case 4:
		last_color[1] += 20;
		prox_RGB  = 5;
		break;

	case 5:
		last_color[0] += 20;
		prox_RGB  = 0;
		break;
	}
	cpy3CharVec(last_color, color);		//Copiar vetor last_color para vetor color
	pthread_mutex_unlock(&mutex_color);
}

void insertScore(int _sock_fd){
	Score_Node* aux = NULL;

	aux = (Score_Node*)malloc(sizeof(Score_Node));
	verifyErr(aux);

	aux->sock_fd = _sock_fd;

	aux->next = score.head;
    score.head = aux;
}

void deleteScore(){
	Score_Node* aux;
	Score_Node* next;

	for(aux = score.head; aux != NULL; aux = next){
        next = aux->next;
        free(aux);
    }
	score.head = NULL;
}

//Função invocada por todas as threads no final do jogo que determina o(s) vencedor(es)
void tryUpdateScore(int n_corrects, int sock_fd){

	pthread_rwlock_rdlock(&rwlock_score);
	if(n_corrects > score.top_score){				//Temos de meter novo record na lista de jogadores vencedores (head desta lista -> score.head)
		pthread_rwlock_unlock(&rwlock_score);
		
		pthread_rwlock_wrlock(&rwlock_score);
		deleteScore();
		insertScore(sock_fd);
		score.top_score = n_corrects;
		score.count++;
		pthread_rwlock_unlock(&rwlock_score);

	}else if(n_corrects == score.top_score){
		pthread_rwlock_unlock(&rwlock_score);

		pthread_rwlock_wrlock(&rwlock_score);
		insertScore(sock_fd);
		score.count++;
		pthread_rwlock_unlock(&rwlock_score);

	}else{
		pthread_rwlock_unlock(&rwlock_score);

		pthread_rwlock_wrlock(&rwlock_score);
		score.count++;
		pthread_rwlock_unlock(&rwlock_score);
	}


	pthread_rwlock_rdlock(&rwlock_stack);			//Para ler o n_players, visto que players podem-se disconectar durante o processo de ranking pontuacao
	pthread_rwlock_rdlock(&rwlock_score);
	if(score.count >= n_players){					//Já todos os scores de todos os clientes foram processados
		sem_post(score.sem_pointer);				//Avisar a thread "responsável" que o vencedor já foi escolhido
	}

	pthread_rwlock_unlock(&rwlock_score);			
	pthread_rwlock_unlock(&rwlock_stack);
}