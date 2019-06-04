/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: SERVIDOR
* FICHEIRO: player_manage.c
*
* Funcionalidades: 
*	-Criar um novo jogador na lista de jogadores;
*	-Eliminar um jogador da lista de jogadores;
*	-Gerar cores para os jogadores;
*	-Determinar o(s) vencedor(es);
*	-Criar lista com o(s) jogador(es) vencedor(es);
*	-Eliminar lista com o o(s) jogador(es) vencedor(es).
*****************************************************************************/

#include "player_manage.h"
#include "client_handler.h"
#include "utils.h"
#include "server.h"

extern Node_Client* head;
extern char last_color[3];
extern char prox_RGB;
extern pthread_rwlock_t rwlock_stack_head;
extern pthread_mutex_t mutex_color;
extern pthread_rwlock_t rwlock_stack;
extern pthread_rwlock_t rwlock_score;
extern Score_List score;
extern int n_players;

/******************************************************************************
* void createPlayer(int client_fd);
*
* Argumentos: int client_fd: identificador do novo cliente;
*
* Retorno:  void;
* 
* Descrição: Função que faz o processo geral de criar um novo jogador criando assim
* uma thread para este novo jogador.
*****************************************************************************/
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

/******************************************************************************
* void insertPlayer(Node_Client* aux)
*
* Argumentos: Node_Client* aux: nó do novo cliente a inserir;
*
* Retorno:  void;
* 
* Descrição: Função que adiciona um cliente à lista de clientes (inserção pela
* head).
*****************************************************************************/
void insertPlayer(Node_Client* aux){

	aux->prev = NULL;								//Meter a NULL pq inserimos pela cabeça

    //Sync para a head da pilha
    rwLock(W_LOCK, &rwlock_stack_head);
	if(head != NULL){
		head->prev = aux;
	}

    aux->next = head;
    head = aux;
	n_players++;								//Incrementar a variável que contém o numero de jogadores online
    rwLock(UNLOCK, &rwlock_stack_head);
}

/******************************************************************************
* void deleteNode(Node_Client* deletingNode)
*
* Argumentos: Node_Client* deletingNode: nó do cliente que de se desconectou;
*
* Retorno:  void;
* 
* Descrição: Função que após um cliente se desconectar, o elimina da lista de
* jogadores;
*****************************************************************************/
void deleteNode(Node_Client* deletingNode){

	//Ver se estamos a apagar a head, para isso temos que dar lock na rwlock da head
	rwLock(R_LOCK, &rwlock_stack_head);
	if(deletingNode == head){				//deletingNode = head
		rwLock(UNLOCK, &rwlock_stack_head);
		
		rwLock(W_LOCK, &rwlock_stack_head);
		if(head->next != NULL){				//Ver se não estamos a apagar o único elemento da lista (simultaneamente head e tail)
			rwLock(W_LOCK, &rwlock_stack);
			head->next->prev = NULL;
		}else{
			rwLock(W_LOCK, &rwlock_stack);
		}
		head = head->next;
		n_players--;										//Decrementar a variável que contém o numero de jogadores online
		rwLock(UNLOCK, &rwlock_stack);
		rwLock(UNLOCK, &rwlock_stack_head);
		free(deletingNode);
		return;
	}

	rwLock(UNLOCK, &rwlock_stack_head);

	//Se não tivermos a apagar a head, temos de dar lock no rwlock geral do stack
	rwLock(W_LOCK, &rwlock_stack);
	if(deletingNode->next == NULL){			//deletingNode = último node
		deletingNode->prev->next = NULL;
	}else{									//deletingNode = node do meio do stack
		deletingNode->prev->next = deletingNode->next;
		deletingNode->next->prev = deletingNode->prev;
	}
	n_players--;										//Decrementar a variável que contém o numero de jogadores online
	rwLock(UNLOCK, &rwlock_stack);

	close(deletingNode->sock_fd);
	free(deletingNode);			//free fora da região crítica porque não é necessário estar
}

/******************************************************************************
* void deleteList()
*
* Argumentos: void;
*
* Retorno:  void;
* 
* Descrição: Função que apaga a lista de clientes.
*****************************************************************************/
void deleteList(){
    Node_Client* aux;
    Node_Client* next;

	rwLock(W_LOCK, &rwlock_stack);
    for(aux = head; aux != NULL; aux = next){
        next = aux->next;
		shutdown(aux->sock_fd, SHUT_RDWR);				//Serve somente para a thread correspondente à sockeada no read()t ficar desbloque
		close(aux->sock_fd);
        free(aux);
    }
	n_players = 0;
	rwLock(UNLOCK, &rwlock_stack);
}

/******************************************************************************
* void generateColor(char color[])
*
* Argumentos: char color[]: vetor com nova cor;
*
* Retorno:  void;
* 
* Descrição: Gera uma cor para um jogador com uma dada ordem (67 cores no máximo).
*****************************************************************************/
void generateColor(char color[]){
	mutex(LOCK, &mutex_color);
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
	mutex(UNLOCK, &mutex_color);
}

/******************************************************************************
* void insertScore(int _sock_fd)
*
* Argumentos: int _sock_fd: idenificador do jogador;
*
* Retorno: void;
* 
* Descrição: Insere o score de um novo jogador na lista corresponde ao record.
*****************************************************************************/
void insertScore(int _sock_fd){
	Score_Node* aux = NULL;

	aux = (Score_Node*)malloc(sizeof(Score_Node));
	verifyErr(aux);

	aux->sock_fd = _sock_fd;

	aux->next = score.head;
    score.head = aux;
}

/******************************************************************************
* void deleteScore()
*
* Argumentos: void;
*
* Retorno: void;
* 
* Descrição: Elimina a lista antiga com o(s) jogador(es) do score máximo antigo.
*****************************************************************************/
void deleteScore(){
	Score_Node* aux;
	Score_Node* next;

	for(aux = score.head; aux != NULL; aux = next){
        next = aux->next;
        free(aux);
    }
	score.head = NULL;
}

/******************************************************************************
* void tryUpdateScore(int n_corrects, int sock_fd)
*
* Argumentos: int n_corrects: pontuação do jogador;
*			  int sock_fd: identificador do jogador;
*
* Retorno: void;
* 
* Descrição: Função invocada por todas as threads no final do jogo que determina
* o(s) jogador(es) vencedores.
*****************************************************************************/
void tryUpdateScore(int n_corrects, int sock_fd){

	rwLock(R_LOCK, &rwlock_score);
	if(n_corrects > score.top_score){				//Temos de meter novo record na lista de jogadores vencedores (head desta lista -> score.head)
		rwLock(UNLOCK, &rwlock_score);
		
		rwLock(W_LOCK, &rwlock_score);
		deleteScore();
		insertScore(sock_fd);
		score.top_score = n_corrects;
		score.count++;
		rwLock(UNLOCK, &rwlock_score);

	}else if(n_corrects == score.top_score){
		rwLock(UNLOCK, &rwlock_score);

		rwLock(W_LOCK, &rwlock_score);
		insertScore(sock_fd);
		score.count++;
		rwLock(UNLOCK, &rwlock_score);

	}else{
		rwLock(UNLOCK, &rwlock_score);

		rwLock(W_LOCK, &rwlock_score);
		score.count++;
		rwLock(UNLOCK, &rwlock_score);
	}


	rwLock(R_LOCK, &rwlock_stack);						//Para ler o n_players, visto que players podem-se disconectar durante o processo de ranking pontuacao
	rwLock(R_LOCK, &rwlock_score);
	if(score.count >= n_players){						//Já todos os scores de todos os clientes foram processados
		semaphore(POST, score.sem_pointer);				//Avisar a thread "responsável" que o vencedor já foi escolhido
	}

	rwLock(UNLOCK, &rwlock_score);			
	rwLock(UNLOCK, &rwlock_stack);
}