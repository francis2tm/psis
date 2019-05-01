#include "board_library.h"

int dim = 4;
Board_Place** board;


int play1[2];
int n_corrects;

void initBoard(){
  int count  = 0;
  int i, j;
  char* str_place;

  n_corrects = 0;
  play1[0]= -1;
  
    //Alocar a matriz
	board = (Board_Place**)malloc(sizeof(Board_Place)*dim);
	verifyErr(board, 'm');
	for(i = 0; i < dim; i++){
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

    Play_Response resp;
    resp.code =10;

    if(strcmp(board[x][y].str, "")==0){
        printf("FILLED\n");
        resp.code =0;
    }else{
        if(play1[0]== -1){
            printf("FIRST\n");
            resp.code =1;

            play1[0]=x;
            play1[1]=y;
            resp.play1[0]= play1[0];
            resp.play1[1]= play1[1];
            strcpy(resp.str_play1, board[x][y].str);
        }else{
            char * first_str = board[play1[0]][play1[1]].str;
            char * secnd_str = board[x][y].str;

            if((play1[0]==x) && (play1[1]==y)){
            resp.code =0;
            printf("FILLED\n");
            }else{
                resp.play1[0]= play1[0];
                resp.play1[1]= play1[1];
                strcpy(resp.str_play1, first_str);
                resp.play2[0]= x;
                resp.play2[1]= y;
                strcpy(resp.str_play2, secnd_str);

                if (strcmp(first_str, secnd_str) == 0){
                    printf("CORRECT!!!\n");

                    strcpy(first_str, "");
                    strcpy(secnd_str, "");

                    n_corrects +=2 ;
                    if (n_corrects == dim*dim)
                        resp.code = 3;
                    else
                    resp.code = 2;
                }else{
                    printf("INCORRECT\n");
                    resp.code = -2;
                }
                play1[0]= -1;
            }
        }
    }
    return resp;
}