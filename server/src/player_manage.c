#include "player_manage.h"
#include "client_handler.h"
#include "utils.h"

extern Node_Client* head;
extern char last_color[3];
extern char prox_RGB;
extern pthread_rwlock_t rwlock_stack_head;
extern pthread_mutex_t mutex_color;
extern pthread_rwlock_t rwlock_stack;
extern pthread_mutex_t mutex_n_players;
extern int n_players;

//Função geral que faz o processo completo de criar um jogador
void createPlayer(int client_fd){
	pthread_t thread_id;
	Node_Client* client_data;
	
	client_data = insertPlayer(client_fd);													//Inserir o player na pilha de jogadores

	if(pthread_create(&thread_id, NULL, (void*)playerHandler, (void*)client_data)){			//Verificar se não houve erro a criar threads
		fprintf(stderr, "Couldn't create thread\n");
		exit(-1);
	}
	if(pthread_detach(thread_id)){															//Fazer detach na thread criada
		fprintf(stderr, "Couldn't detach thread\n");
		exit(-1);
	}
	updateNumPlayers(1);																	//Incrementar a variável que contém o numero de jogadores online
}

//Adiciona um nó na lista de players
Node_Client* insertPlayer(int sock_fd){
    Node_Client* aux = NULL;

    aux = (Node_Client*)malloc(sizeof(Node_Client));
    verifyErr(aux);

    aux->sock_fd = sock_fd;
	aux->prev = NULL;

    //Sync para a head da pilha
    pthread_rwlock_wrlock(&rwlock_stack_head);
	if(head != NULL){
		head->prev = aux;
	}

    aux->next = head;
    head = aux;
    pthread_rwlock_unlock(&rwlock_stack_head);
	return aux;
}

//Apaga 1 elemento da lista de clientes, a função foi escrita de modo a que thread tenha os locks o menos tempo possível 
void deleteNode(Node_Client* deletingNode){

	//Ver se estamos a apagar a head, para isso temos que dar lock na rwlock da head
	pthread_rwlock_rdlock(&rwlock_stack_head);
	if(deletingNode == head){				//deletingNode = head
		pthread_rwlock_unlock(&rwlock_stack_head);
		pthread_rwlock_wrlock(&rwlock_stack_head);
		if(head->next != NULL){				//Ver se não estamos a apagar o único elemento da lista (simultaneamente head e tail)
			head->next->prev = NULL;
		}
		head = head->next;
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
	pthread_rwlock_unlock(&rwlock_stack);
	free(deletingNode);			//free fora da região crítica porque não é necessário estar
}

//Apaga a lista de clientes
void deleteList(){
    Node_Client* aux;
    Node_Client* next;

    for(aux = head; aux != NULL; aux = next){
        next = aux->next;
        free(aux);
    }
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

void updateNumPlayers(char value){

	if(value){				//Se for para incrementar
		pthread_mutex_lock(&mutex_n_players);
		n_players++;

	}else{					//Se for para decrementar
		pthread_mutex_lock(&mutex_n_players);
		n_players--;
	}
	pthread_mutex_unlock(&mutex_n_players);
}