#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "utils.h"
#include "board_library.h"

#define SERVER_ADDR "../server_sock"

void* playerHandler(int client_fd);

int dim = 4;
Board_Place** board = NULL;
int n_corrects;

int main(){
	struct sockaddr_un local_addr;
	struct sockaddr_un client_addr;
	socklen_t size_addr;
	pthread_t thread_id;
	int client_fd;
	int server_fd;

	unlink(SERVER_ADDR);

	initBoard();

	if((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		perror("socket: ");
		exit(-1);
	}

	//Setup socket

	local_addr.sun_family = AF_UNIX;
	strcpy(local_addr.sun_path, SERVER_ADDR);

	if(bind(server_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0){
		perror("bind");
		exit(-1);
	}
	printf(" socket created and binded \n");

	if(listen(server_fd, 2) == -1){
		perror("listen");
		exit(-1);
	}

	//Main thread só aceita novas conexões
	while(1){
		printf(" Ready to accept connections\n");	
		if((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &size_addr)) == -1){
			perror("accept");
			exit(-1);
		}
		printf("acceped connection from %s\n", client_addr.sun_path);
		if(pthread_create(&thread_id, NULL, (void*)playerHandler, (void*)client_fd)){
			perror("Couldn't create thread\n");
			exit(-1);
		}
	}
	return EXIT_SUCCESS;
}

//Handler para cada jogador, é executado pela thread associada ao jogador
void* playerHandler(int client_fd){
	int play_recv[2];
	char* buff_send = NULL;
	Play_Response resp;
	char n_play = 0;       //0- n fez nenhuma jogada 1- ja fez 1 jogada

	resp.code = 0;
    resp.play1[0] = -1;

	buff_send = (char*)malloc(sizeof(Play_Response));
	verifyErr(buff_send, 'm');				//Verifica se malloc foi bem sucedido

	while(read(client_fd, play_recv, sizeof(int)*2) > 0){
		printf("(%d,%d)\n", play_recv[0], play_recv[1]);
		
		boardPlay(&resp, &n_play, play_recv[0], play_recv[1]);
		memcpy(buff_send, &resp, sizeof(Play_Response));
		write(client_fd, buff_send, sizeof(Play_Response));
		if(resp.code == -2){
			sleep(2);
			resp.code = -4;
			memcpy(buff_send, &resp, sizeof(Play_Response));
			// METER ALGO A VERIFICAR SE O CLIENTE SE DISCONECTOR ANTES DE FAZER O WRITE 
			write(client_fd, buff_send, sizeof(Play_Response));
			
		}
	}
	free(buff_send);
	printf("Client disconnected\n");
	return 0;
}