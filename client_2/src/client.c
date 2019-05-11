#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "UI_library.h"
#include "utils.h"

#define SERVER_ADDR "../server_sock"
#define CLIENT_ADDR "../client_2_sock"

typedef struct Play_Response{
  int code;  
            // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -1 2nd - virar primeira jogada para baixo
            // -2 2nd - combinação errada, meter as cartas vermelhas durante 2s
            // -4 2nd - combinação errada, virar as cartas para branco  ao fim dos 2s
  int play1[2];
  int play2[2];
  char color[3];
  char str_play1[3], str_play2[3];
}Play_Response;

void* recvHandler(int sock_fd);
void updateBoard(Play_Response resp);

int dim = 4;

int main(){
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	int sock_fd; 
	pthread_t thread_id;
	int play[2];

	SDL_Event event;
	int done = 0;

	unlink(CLIENT_ADDR);

	if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
		perror("socket: ");
		exit(-1);
	}

	client_addr.sun_family = AF_UNIX;
	strcpy(client_addr.sun_path, CLIENT_ADDR);

	
	if(bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1){
		perror("bind");
		exit(-1);
	}

	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SERVER_ADDR);

	if(connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		printf("Error connecting\n");
		exit(-1);
	}

	if(pthread_create(&thread_id, NULL, (void*)recvHandler, (void*)sock_fd)){
			perror("Couldn't create thread\n");
			exit(-1);
	}

	if(SDL_Init( SDL_INIT_VIDEO) < 0){
		 printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		 exit(-1);
	}
	if(TTF_Init() == -1){
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}
	
	createBoardWindow(300, 300);

	while (!done){
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT:{
					done = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					getBoardCard(event.button.x, event.button.y, &play[0], &play[1]);
					write(sock_fd, play, sizeof(int)*2);
				}
			}
		}
		SDL_PumpEvents();
	}
	printf("fim\n");
	closeBoardWindows();
}

void* recvHandler(int sock_fd){
	char* buff_recv = NULL;
	Play_Response resp;

	buff_recv = (char*)malloc(sizeof(Play_Response));
	verifyErr(buff_recv, 'm');				//Verifica se malloc foi bem sucedido

	while(read(sock_fd, buff_recv, sizeof(Play_Response)) > 0){
		memcpy((void*)&resp, buff_recv, sizeof(Play_Response));
		updateBoard(resp);
	}

	free(buff_recv);

	return NULL;
}

void updateBoard(Play_Response resp){
	switch (resp.code){
		case 1:		//Primeira jogada
			paintCard(resp.play1[0], resp.play1[1], resp.color[0], resp.color[1], resp.color[2]);
			writeCard(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
			break;
		case 3:		//Acabou o jogo (todas as cartas tao up)
			//done = 1;
		case 2:		//Jogar 2x e acertar na combinação
			paintCard(resp.play1[0], resp.play1[1], resp.color[0], resp.color[1], resp.color[2]);
			writeCard(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
			paintCard(resp.play2[0], resp.play2[1], resp.color[0], resp.color[1], resp.color[2]);
			writeCard(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
			break;
		case -2:	//Jogar 2x e falhar na combinação
			paintCard(resp.play1[0], resp.play1[1], resp.color[0], resp.color[1], resp.color[2]);
			writeCard(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
			paintCard(resp.play2[0], resp.play2[1], resp.color[0], resp.color[1], resp.color[2]);
			writeCard(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
			break;
		case -4:
			paintCard(resp.play1[0], resp.play1[1], 255, 255, 255);
			paintCard(resp.play2[0], resp.play2[1], 255, 255, 255);
			break;
		case -1:
			paintCard(resp.play1[0], resp.play1[1], 255, 255, 255);
			break;
	}
}