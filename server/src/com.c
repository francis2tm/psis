/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: SERVIDOR
* FICHEIRO: com.c
*
* Funcionalidades: 
*	-Enviar a dimensão do tabuleiro para os clientes;
*	-Iniciar socket para comunicação cliente <-> servidor;
*	-Receber a jogada do cliente;
*	-Enviar o processamento da jogada para o cliente;
*	-Notificar tidas as threads que vamos fazer restart do jogo;
*	-Notifica o cliente vencedor bem como os restantes;
*	-Envia o estado do tabuleiro a um novo jogador;
*****************************************************************************/
#include "com.h"
#include "utils.h"
#include "server.h"

extern int dim;
extern pthread_mutex_t mutex_color;
extern pthread_rwlock_t rwlock_stack_head;
extern pthread_rwlock_t rwlock_stack;
extern pthread_rwlock_t rwlock_score;
extern Score_List score;
extern Board_Place** board;
extern Node_Client* head;


/******************************************************************************
* void broadcastBoard(Play_Response resp, char* buff_send)
*
* Argumentos: Play_Response resp: informação a enviar;
*			  void* buff_send: pointer para o stream de bites a enviar, servindo 
*			  como intermediário;
*
* Retorno:  void;
* 
* Descrição: Faz broadcast das alterações da board a todos os clientes, 
* iterando pela pilha de jogadores.
*****************************************************************************/

void broadcastBoard(Play_Response resp, char* buff_send){
    Node_Client* aux;

	//Meter a ordem dos bits bem para network, Como o resp não está a ser passado por referência, os seus valores locais a esta função podem ser alterados
	resp.play1[0] = htonl(resp.play1[0]);
	resp.play1[1] = htonl(resp.play1[1]); 

	resp.play2[0] = htonl(resp.play2[0]);
    resp.play2[1] = htonl(resp.play2[1]); 

	memcpy(buff_send, &resp, sizeof(Play_Response));

	rwLock(R_LOCK, &rwlock_stack_head);
	if(head != NULL){												//Só enviar se houver clientes
		//Enviar à head
		enviar(head->sock_fd, buff_send, sizeof(Play_Response));

		rwLock(R_LOCK, &rwlock_stack);						//Bloquear para leitura a lista toda
		aux = head->next;
		rwLock(UNLOCK, &rwlock_stack_head);					//Já podemos desbloquear a head

		//Enviar ao resto da lista
		while(aux != NULL){
			enviar(aux->sock_fd, buff_send, sizeof(Play_Response));
			aux = aux->next;
		}
		rwLock(UNLOCK, &rwlock_stack);
	}else{
		rwLock(UNLOCK, &rwlock_stack_head);
	}    
}

/******************************************************************************
* int sendActualBoard(int client_fd, char* buff_send)
*
* Argumentos: int client_fd:
*			  char* buff_send: 
*
* Retorno:  int err: sucesso ou insucesso do envio;
* 
* Descrição: Envia o estado do board atual a um novo jogador, retorna se houve 
* erro ao enviar.
*****************************************************************************/
int sendActualBoard(int client_fd, char* buff_send){
    Play_Response resp;
	int err = 0;

    for(int i = 0; i < dim; i++){
        for(int j = 0; j < dim; j++){
            mutex(LOCK, &(board[i][j].mutex_board));
            if(board[i][j].is_up){                                  //Só enviar cartas que estão para cima
				resp.code = board[i][j].code;
				cpy3CharVec(board[i][j].owner_color, resp.color);
				strcpy(resp.str_play1, board[i][j].str);
				mutex(UNLOCK, &(board[i][j].mutex_board));

				resp.play1[0] = htonl(i);
				resp.play1[1] = htonl(j);
				resp.play2[0] = htonl(-1);

				memcpy(buff_send, &resp, sizeof(Play_Response));
				err = enviar(client_fd, buff_send, sizeof(Play_Response));
            }else{
				mutex(UNLOCK, &board[i][j].mutex_board);
			}
        }
    }
	//Avisar o servidor que acabou a transmissão do board atual
	resp.play2[0] = htonl(0);
	memcpy(buff_send, &resp, sizeof(Play_Response));
	err = enviar(client_fd, buff_send, sizeof(Play_Response));
	return err;
}

/******************************************************************************
* int initSocket(socklen_t size_addr)
*
* Argumentos: 
*
* Retorno:  int server_fd: identificador da socket criada
* 
* Descrição: Função que cria uma socket e inicializa os devidos parâmetros 
* verificando a existência de erros nalgumas destas inicializações
*****************************************************************************/
int initSocket(struct sockaddr_in* local_addr){
	int server_fd;
	
	
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){										//Verificar se não houve erro a criar a socket
		perror("socket: ");
		exit(-1);
	}

	local_addr->sin_family = AF_INET;
    local_addr->sin_addr.s_addr = htonl(INADDR_ANY); 
    local_addr->sin_port = htons(PORT); 

	if(bind(server_fd, (struct sockaddr*)local_addr, sizeof(*local_addr)) < 0){					//Verificar se não houve erro a fazer bind
		perror("bind");
		exit(-1);
	}
	printf(" socket created and binded \n");

	if(listen(server_fd, 2) == -1){																//Verificar se não houve erro a fazer listen
		perror("listen");
		exit(-1);
	}

	return server_fd;
}

/******************************************************************************
* int enviar(int sock_fd, void* buff, size_t size)
*
* Argumentos: int sock_fd: identificador da socket por onde vamos enviar;
*			  void* buff: pointer para o stream de bites recebido, servindo 
*			  como intermediário;
*			  size_t size: tamanho da informação a enviar.
*
* Retorno:  -1 em erro;
*			 0 em sucesso;
* 
* Descrição: Função que envia a informação processada para o cliente com o 
* respetivo sock_fd retornando -1 em caso de erro.
*****************************************************************************/
int enviar(int sock_fd, void* buff, size_t size){
	if(write(sock_fd, buff, size) < 0){
		fprintf(stderr, "Unable to send data to socket");
		return -1;
	}
	return 0;
}

/******************************************************************************
* int receber(int client_fd, int play_recv[])
*
* Argumentos: int cliend_fd: socket onde a informação é recebida;
*			  int play_recv[]: guardar a informação recebida (posição 0 é
* coordenada x, posição 1 é a coordenada y);
*
* Retorno:  int return_read: flag que verifica o sucesso do recebimento da 
* 			informação enviada pelo servidor;
* 
* Descrição: Função que recebe as coordenadas (x,y) e as guarda para
* processamento.
*****************************************************************************/
int receber(int client_fd, int play_recv[]){
	int return_read = 0;

	return_read = read(client_fd, play_recv, sizeof(int)*2);
	play_recv[0] = ntohl(play_recv[0]);
	play_recv[1] = ntohl(play_recv[1]);
	
	return return_read;
}

/******************************************************************************
* void sendDim(int client_fd, Cmn_Thr_Data* common_data, Node_Client* client_data)
*
* Argumentos: int cliend_fd: socket onde a informação vai ser enviada;
*			  Cmn_Thr_Data* common_data: 
*			  Node_Client* client_data: 
*
* Retorno:  void;
* 
* Descrição: Função que envia a dimensão do tabuleira para o cliente. Verifica
* se exitiu erro no envio, e caso tal tenha acontecido vai sair da thread.
*****************************************************************************/
void sendDim(int client_fd, Cmn_Thr_Data* common_data, Node_Client* client_data){
	int aux_dim;

	aux_dim = htonl(dim);
	if(enviar(client_fd, (void*)&aux_dim, sizeof(int)) < 0 || sendActualBoard(client_fd, common_data->buff_send) < 0){
		deleteNode(client_data);
		sem_destroy(&common_data->sem);
		pthread_mutex_destroy(&common_data->mutex_timer);
		free(common_data->buff_send);
		pthread_exit(NULL);
	}
}

/******************************************************************************
* void broadcastThreads(int _sockfd)
*
* Argumentos: int _sockfd: identificador da socket para os clientes;
*
* Retorno:  void;
* 
* Descrição: Função que notifica as threads que vamos fazer restart ao jogo 
* (notifica todas as threads menos a(s) do(s) jogador(es) que ganhou(ganharam), 
* pois esta(s) já sabe(m) que o jogo terminou). De notar que esta função é
* de comunicação inter-threads e não servidor-cliente
*****************************************************************************/
void broadcastThreads(int _sockfd){
	Node_Client* aux;

	rwLock(R_LOCK, &rwlock_stack_head);
	if(head != NULL){												//Só enviar se houver clientes
		//Notificar a head
		if(head->sock_fd != _sockfd){								//Não fazer sem_post à thread "responsável" pelo reset
			semaphore(POST, head->sem_pointer);
		}
		

		rwLock(R_LOCK, &rwlock_stack);								//Bloquear para leitura a lista toda
		aux = head->next;
		rwLock(UNLOCK, &rwlock_stack_head);							//Já podemos desbloquear a head

		//Notificar o resto da lista
		while(aux != NULL){
			if(aux->sock_fd != _sockfd){							//Não fazer sem_post à thread "responsável" pelo reset
				semaphore(POST, aux->sem_pointer);
			}
			aux = aux->next;
		}
		rwLock(UNLOCK, &rwlock_stack);
	}else{
		rwLock(UNLOCK, &rwlock_stack_head);
	}
}

/******************************************************************************
* void sendGameOver(Play_Response resp, char* buff_send, int loser_fd)
*
* Argumentos: Play_Response resp: mensagem original;
*			  void* buff: pointer para o stream de bites a enviar servindo 
*			  como intermediário;
*			  int loser_fd: identificador dos jogadores que perderam;
*
* Retorno:  void;
* 
* Descrição: Função que notifica os clientes que estes ganharam ou perderam.
*****************************************************************************/
void sendGameOver(Play_Response resp, char* buff_send, int loser_fd){
	Score_Node* aux;

	memcpy(buff_send, &resp, sizeof(Play_Response));

	if(resp.code == 10){												//Se for para enviar aos vencedores
		rwLock(R_LOCK, &rwlock_score);
		for(aux = score.head; aux != NULL; aux = aux->next){
			enviar(aux->sock_fd, buff_send, sizeof(Play_Response));
		}

		rwLock(UNLOCK, &rwlock_score);
	}else{																//Se for para enviar para um loser
		enviar(loser_fd, buff_send, sizeof(Play_Response));
	}
	
}