#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include "utils.h"
#include "board_library.h"
#include "player_manage.h"

#define SERVER_ADDR "../server_sock"

void* playerHandler(Node_Client* client_data);

int dim = 4;								//Dimensão do board
Board_Place** board = NULL;					//Duplo ponteiro, posteriormente será usado para alcocar dinâmicamente em função do dim para gerar uma matriz
char last_color[3] = {25, 5, 10};			//Última cor gerada pelo último cliente a ter sido conectado
char prox_RGB = 0;							//Variável global utilizada pela generateColor()
Node_Client* head = NULL;					//Head do stack dos jogadores

pthread_mutex_t mutex_color;
pthread_rwlock_t rwlock_stack_head;
pthread_rwlock_t rwlock_stack;

int main(){
	struct sockaddr_un local_addr;
	struct sockaddr_un client_addr;
	socklen_t size_addr;
	pthread_t thread_id;
	int client_fd;
	int server_fd;
	int i, j;
	Node_Client* client_data;

	signal(SIGPIPE, SIG_IGN);	//Caso um cliente se disconecte enquanto a respectiva thread está no sleep(2) ou se está uma thread com a read lock da lista de jogadores e a thread do jogador que se disconectou não tem tempo de eliminar o node correspondente

	unlink(SERVER_ADDR);

	initBoard();		//Alocar e preencher a board

	//Inicializar sync
	if(pthread_mutex_init(&mutex_color, NULL)){
		perror("Mutex init ");
		exit(-1);
	}
	if(pthread_rwlock_init(&rwlock_stack_head, NULL)){
		perror("rw_lock init ");
		exit(-1);
	}
	if(pthread_rwlock_init(&rwlock_stack, NULL)){
		perror("rw_lock init ");
		exit(-1);
	}

	for(i = 0; i < dim; i++){
		for(j = 0; j < dim; j++){
			pthread_mutex_init(&board[i][j].mutex_board, NULL);
		}
	}

	//Setup socket
	if((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){										//Verificar se não houve erro a criara socket
		perror("socket: ");
		exit(-1);
	}

	local_addr.sun_family = AF_UNIX;
	strcpy(local_addr.sun_path, SERVER_ADDR);

	if(bind(server_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0){					//Verificar se não houve erro a fazer bind
		perror("bind");
		exit(-1);
	}
	printf(" socket created and binded \n");

	if(listen(server_fd, 2) == -1){																//Verificar se não houve erro a fazer listen
		perror("listen");
		exit(-1);
	}

	//Main thread só aceita novas conexões
	while(1){
		printf(" Ready to accept connections\n");	
		if((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &size_addr)) == -1){	//Verficiar se não houve erro a fazer accept
			perror("accept");
			exit(-1);
		}
		printf("accepted connection from %s\n", client_addr.sun_path);

		client_data = insertPlayer(client_fd);													//Inserir o player na pilha de jogadores

		if(pthread_create(&thread_id, NULL, (void*)playerHandler, (void*)client_data)){			//Verificar se não houve erro a criar threads
			perror("Couldn't create thread\n");
			exit(-1);
		}
	}

	if(head != NULL){				//Só apagar se a head tiver elementos
		deleteList();				//Apagar a lista de jogadores
	}

	//Destruir as estruturas para a sincronização
	pthread_mutex_destroy(&mutex_color);		
	pthread_rwlock_destroy(&rwlock_stack_head);
	pthread_rwlock_destroy(&rwlock_stack);
	for(i = 0; i < dim; i++){
		for(j = 0; j < dim; j++){
			pthread_mutex_destroy(&board[i][j].mutex_board);
		}
	}

	return EXIT_SUCCESS;
}

//Handler para cada jogador, é executado pela thread associada ao jogador
void* playerHandler(Node_Client* client_data){
	int play_recv[2];			//Coordenadas (x,y) recebidas pelo cliente		
	char* buff_send = NULL;		//Buffer auxiliar para enviar data ao cliente
	Play_Response resp;			//Estrutura que é enviada ao cliente
	char n_play = 0;			//0- n fez nenhuma jogada 1- ja fez 1 jogada
	int n_corrects = 0;
	int client_fd = client_data->sock_fd;

	//Inicializar o resp
	resp.code = 0;				
    resp.play1[0] = -1;

	generateColor(resp.color);	//Gerar a cor para o jogador, resp.color {R,G,B} que contém a cor do jogador, apartir de agora todos os resp desta thread terão esta cor

	//Alocar memória para buff_send
	buff_send = (char*)malloc(sizeof(Play_Response));
	verifyErr(buff_send, 'm');				//Verifica se malloc foi bem sucedido


	//Ciclo até o cliente estiver conectado
	while(read(client_fd, play_recv, sizeof(int)*2) > 0){						
		boardPlay(&resp, &n_play, &n_corrects, play_recv[0], play_recv[1]);	//Processar a play recebida pelo jogador
		memcpy(buff_send, &resp, sizeof(Play_Response));
		broadcastBoard(resp, buff_send);									//Mandar a todos os jogadores a play processada (enviar o resp)
		if(resp.code == -2){	//Se  jogador errou na combinação
			sleep(2);			//Fazer o timer de 2s com um sleep() -> ignorar jogadas do jogador
			pthread_mutex_lock(&board[resp.play1[0]][resp.play1[1]].mutex_board);
			board[resp.play1[0]][resp.play1[1]].is_up = 0;
            board[play_recv[0]][play_recv[1]].is_up = 0;
			pthread_mutex_unlock(&board[resp.play1[0]][resp.play1[1]].mutex_board);
			resp.code = -4;
			memcpy(buff_send, &resp, sizeof(Play_Response));
			broadcastBoard(resp, buff_send);			//Mandar alterações do board a todos os jogadores		
		}
	}
	free(buff_send);
	deleteNode(client_data);
	printf("Client disconnected\n");
	return NULL;
}