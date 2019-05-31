#include "client.h"
#include "com.h"

int dim;
int sock_fd;

int main(){
	pthread_t thread_id;
	char* buff_recv = NULL;
	Play_Response resp = {.play2[0] = 0};

	unlink(CLIENT_ADDR);

	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){			//Caso uma thread tá a ler o nó de um jogador que acabou de se disconectar e a thread do jogador que se disconectou não tem tempo de eliminar o node correspondente
		fprintf(stderr, "Couldn't define signal handler\n");
		exit(-1);
	}

	initSDL();

	sock_fd = initSocket();

	buff_recv = (char*)malloc(sizeof(Play_Response));
	verifyErr(buff_recv);												//Verifica se malloc foi bem sucedido

	handShake(sock_fd);													//Primeira mensagem, receber dim

	createBoardWindow();
	

	//Receber estado atual do board
	while(receber(sock_fd, &resp, buff_recv) > 0){
		if(resp.play2[0] == -1){
			setActualBoard(resp);
		}else{
			break;		//Sair de preencher o estado atual da board quando resp.play2[0] != -1
		}
	}

	//Criar thread que recebe eventos SDL
	if(pthread_create(&thread_id, NULL, (void*)sdlHandler, (void*)sock_fd)){
		fprintf(stderr, "Couldn't create thread\n");
		exit(-1);
	}

	//Criar main loop de receção das jogadas processadas
	while(receber(sock_fd, &resp, buff_recv) > 0){
		updateBoard(resp);
	}

	free(buff_recv);
	printf("Fim\n");

	return EXIT_SUCCESS;
}

void* sdlHandler(int sock_fd){
	int play[2];
	SDL_Event event;
	int done = 0;
	
	while (!done){
		while (SDL_PollEvent(&event)){
			SDL_PumpEvents();
			switch (event.type){
				case SDL_QUIT:{
					done = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					getBoardCard(event.button.x, event.button.y, &play[0], &play[1]);
					if(enviar(sock_fd, play) < 0){
						closeBoardWindows();
						return NULL;
					}
					break;
				}
			}
			SDL_PumpEvents();
		}
	}
	shutdown(sock_fd, SHUT_RDWR);				//Serve somente para a thread que está a dormir no read() ficar desbloqueada 
	close(sock_fd);
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
		case -1:	//Virar a carta da primeira jogada para cima caso o timer 5s tenha acabado ou o jogador na 2a jogada clicou numa carta UP
			paintCard(resp.play1[0], resp.play1[1], 255, 255, 255);
			break;
		case 4:		//Começar novo jogo, reset do board
			resetBoard();
			break;
		case 10:
			printf("Game Over\n");
			printf("CONGRATS! YOU WON!!!\n");
			break;
		case 11:
			printf("Game over, you aren't the winner\n");
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