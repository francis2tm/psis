#include "board_library.h"

extern int dim;
extern Board_Place** board;

int n_corrects;
Play_Response resp;
char n_play = 0;       //0- n fez nenhuma jogada 1- ja fez 1 jogada

void initBoard(void){
    int count  = 0;
    int i, j;
    char* str_place;

    resp.code = 0;
    resp.play1[0] = -1;
  
    //Alocar a matriz
	board = malloc(sizeof(Board_Place)*dim);
	verifyErr(board, 'm');
	for(i = 0; i < dim; i++){
        board[i] = NULL;
		board[i] = (Board_Place*)malloc(sizeof(Board_Place)*dim);
		if(board == NULL){
			verifyErr(board[i], 'm');
		}
	}

	//Inicializar a matriz
	for(i = 0; i < (dim); i++){
		for(j = 0; j < dim; j++){
			board[i][j].str[0] = '\0';
			board[i][j].is_up = 0;
	 	}
	}

    for (char c1 = 'a' ; c1 < ('a'+dim); c1++){
        for (char c2 = 'a' ; c2 < ('a'+dim); c2++){
            do{
                i = rand() % dim;
                j = rand() % dim;
                str_place = board[i][j].str;
                printf("%d %d -%s-\n", i, j, str_place);
            }while(str_place[0] != '\0');
            str_place[0] = c1;
            str_place[1] = c2;
            str_place[2] = '\0';
            do{
                i = rand() % dim;
                j = rand() % dim;
                str_place = board[i][j].str;
                printf("%d %d -%s-\n", i, j, str_place);
            }while(str_place[0] != '\0');
            str_place[0] = c1;
            str_place[1] = c2;
            str_place[2] = '\0';
            count += 2;
            if (count == dim*dim){
                return;
            }
        }
    }
}

Play_Response boardPlay(int x, int y){

    if(board[x][y].is_up){
        if(resp.code == 1){    //Se for a segunda jogada e for para virar a primeira carta para baixo
            resp.code = -1;
            n_play = 0;
        }else{
            printf("FILLED\n");
            resp.code = 0;
        }
    }else{
        if(!n_play){            //Se for a primeira jogada
            printf("FIRST\n");
            resp.code = 1;
            n_play = 1;
            resp.play1[0] = x;
            resp.play1[1] = y;
            strcpy(resp.str_play1, board[x][y].str);
        }else{                  //Se for a segunda jogada
            char* first_str = board[resp.play1[0]][resp.play1[1]].str;
            char* secnd_str = board[x][y].str;

            strcpy(resp.str_play1, first_str);
            resp.play2[0] = x;
            resp.play2[1] = y;
            strcpy(resp.str_play2, secnd_str);

            if (!strcmp(first_str, secnd_str)){
                printf("CORRECT!!!\n");

                n_corrects += 2;
                if (n_corrects == dim*dim){
                    resp.code = 3;
                }else{
                    resp.code = 2;
                }
            }else{
                printf("INCORRECT\n");
                resp.code = -2;
            }
            n_play = 0;
        }
    }
    return resp;
}