#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#include "utils.h"
#include "player_manage.h"
#include "board_library.h"

#define SERVER_ADDR "../server_sock"

int dim = 4;								//Dimensão do board
Board_Place** board = NULL;					//Duplo ponteiro, posteriormente será usado para alcocar dinâmicamente em função do dim para gerar uma matriz
char last_color[3] = {25, 5, 10};			//Última cor gerada pelo último cliente a ter sido conectado
char prox_RGB = 0;							//Variável global utilizada pela generateColor()
Node_Client* head = NULL;					//Head do stack dos jogadores
unsigned int max_score = 0;

pthread_mutex_t mutex_color;
pthread_rwlock_t rwlock_stack_head;
pthread_rwlock_t rwlock_stack;

int main(){
	struct sockaddr_un local_addr;
	struct sockaddr_un client_addr;
	socklen_t size_addr;
	int client_fd;
	int server_fd;
	int i, j;

	//signal(SIGPIPE, SIG_IGN);	//Caso uma thread tá a ler o nó de um jogador que acabou de se disconectar e a thread do jogador que se disconectou não tem tempo de eliminar o node correspondente

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

		createPlayer(client_fd);

		printf("accepted connection from %s\n", client_addr.sun_path);

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