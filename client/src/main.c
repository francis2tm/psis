#include "main.h"

Board_Place** board = NULL;;
int dim = 4;


int main(){
	
	SDL_Event event;
	int done = 0;

	if(SDL_Init( SDL_INIT_VIDEO) < 0){
		 printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		 exit(-1);
	}
	if(TTF_Init() == -1){
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}
	
	createBoardWindow(300, 300);
	initBoard();

	while (!done){
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT:{
					done = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					int board_x, board_y;
					getBoardCard(event.button.x, event.button.y, &board_x, &board_y);

					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
					Play_Response _resp = boardPlay(board_x, board_y);
					switch (_resp.code){
						case 1:		//Primeira jogada
							paintCard(_resp.play1[0], _resp.play1[1], 7, 200, 100);
							writeCard(_resp.play1[0], _resp.play1[1], _resp.str_play1, 200, 200, 200);
							board[board_x][board_y].is_up = 1;
							break;
						case 3:		//Acabou o jogo (todas as cartas tao up)
						  done = 1;
						case 2:		//Jogar 2x e acertar na combinação
							paintCard(_resp.play1[0], _resp.play1[1], 107, 200, 100);
							writeCard(_resp.play1[0], _resp.play1[1], _resp.str_play1, 0, 0, 0);
							paintCard(_resp.play2[0], _resp.play2[1], 107, 200, 100);
							writeCard(_resp.play2[0], _resp.play2[1], _resp.str_play2, 0, 0, 0);
							board[board_x][board_y].is_up = 1;
							break;
						case -2:	//Jogar 2x e falhar na combinação
							paintCard(_resp.play1[0], _resp.play1[1], 107, 200, 100);
							writeCard(_resp.play1[0], _resp.play1[1], _resp.str_play1, 255, 0, 0);
							paintCard(_resp.play2[0], _resp.play2[1], 107, 200, 100);
							writeCard(_resp.play2[0], _resp.play2[1], _resp.str_play2, 255, 0, 0);
							board[board_x][board_y].is_up = 1;
							sleep(2);
							paintCard(_resp.play1[0], _resp.play1[1], 255, 255, 255);
							paintCard(_resp.play2[0], _resp.play2[1], 255, 255, 255);
							board[_resp.play1[0]][_resp.play1[1]].is_up = 0;
							board[board_x][board_y].is_up = 0;
							break;
						case -1:
							paintCard(_resp.play1[0], _resp.play1[1], 255, 255, 255);
							board[_resp.play1[0]][_resp.play1[1]].is_up = 0;
							break;
					}
				}
			}
		}
	}
	printf("fim\n");
	//closeBoardWindows();

	//Free no board
	/*for(int i = 0; i < dim; i++){
		free(board[i]);
	}
	free(board);*/
}
 