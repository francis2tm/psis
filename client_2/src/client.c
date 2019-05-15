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

void* sdlHandler(int sock_fd);
void handShake(int sock_fd);
void setActualBoard(Play_Response resp);
void updateBoard(Play_Response resp);

int dim;

int main(){
	int sock_fd;
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	pthread_t thread_id;
	char* buff_recv = NULL;
	Play_Response resp = {.play2[0] = 0};

	unlink(CLIENT_ADDR);

	if(SDL_Init( SDL_INIT_VIDEO) < 0){
		 printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		 exit(-1);
	}
	if(TTF_Init() == -1){
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
		perror("socket: ");
		exit(-1);
	}

	client_addr.sun_family = AF_UNIX;
	strcpy(client_addr.sun_path, CLIENT_ADDR);

	
	if(bind(sock_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1){
		perror("bind");
		exit(-1);
	}

	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SERVER_ADDR);

	if(connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		printf("Error connecting\n");
		exit(-1);
	}

	buff_recv = (char*)malloc(sizeof(Play_Response));
	verifyErr(buff_recv, 'm');				//Verifica se malloc foi bem sucedido

	handShake(sock_fd);						//Primeira mensagem, receber dim

	createBoardWindow(300, 300);			//Criar board em função do dim

	//Receber estado atual do board
	while(read(sock_fd, buff_recv, sizeof(Play_Response)) > 0){
		memcpy((void*)&resp, buff_recv, sizeof(Play_Response));
		if(resp.play2[0] == -1){
			printf("(%d,%d)\n", resp.play1[0], resp.play2[1]);
			setActualBoard(resp);
		}else{
			break;		//Sair de preencher o estado atual da board quando resp.play2[0] != -1
		}
	}

	//Criar thread que recebe eventos SDL
	if(pthread_create(&thread_id, NULL, (void*)sdlHandler, (void*)sock_fd)){
		perror("Couldn't create thread\n");
		exit(-1);
	}

	//Criar main loop de receção das jogadas processadas
	while(read(sock_fd, buff_recv, sizeof(Play_Response)) > 0){
		memcpy((void*)&resp, buff_recv, sizeof(Play_Response));
		updateBoard(resp);
	}

	free(buff_recv);
	printf("Server Disconnected\n");

	return EXIT_SUCCESS;
}

void* sdlHandler(int sock_fd){
	int play[2];
	SDL_Event event;
	int done = 0;
	
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
		case -4:	//No fim do timer 2s, voltar a meter as cartas para baixo
			paintCard(resp.play1[0], resp.play1[1], 255, 255, 255);
			paintCard(resp.play2[0], resp.play2[1], 255, 255, 255);
			break;
		case -1:
			paintCard(resp.play1[0], resp.play1[1], 255, 255, 255);
			break;
	}
}

void setActualBoard(Play_Response data){
	switch (data.code){
		case 1:		//Carta resultante de first pick
			paintCard(data.play1[0], data.play1[1], data.color[0], data.color[1], data.color[2]);
			writeCard(data.play1[0], data.play1[1], data.str_play1, 200, 200, 200);
			break;

		case 2:		//Carta locked
			paintCard(data.play1[0], data.play1[1], data.color[0], data.color[1], data.color[2]);
			writeCard(data.play1[0], data.play1[1], data.str_play1, 0, 0, 0);
			break;
		case -2:	//Carta errada
			paintCard(data.play1[0], data.play1[1], data.color[0], data.color[1], data.color[2]);
			writeCard(data.play1[0], data.play1[1], data.str_play1, 255, 0, 0);
			break;
	}
}

void handShake(int sock_fd){

	if(read(sock_fd, &dim, sizeof(int)) > 0){
		printf("Hand Shake done, dim=%d \n", dim);
	}else{
		printf("Hand shake not possible\n");
		exit(-1);
	}
}
