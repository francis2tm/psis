#include "board_library.h"

extern int dim;
extern Board_Place** board;

//Aloca e preenche o board com strings
void initBoard(void){
    int count  = 0;
    int i, j;
    char* str_place;
  
    //Alocar a matriz
	board = (Board_Place**)malloc(sizeof(Board_Place*)*dim);
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

//Processa as jogada de um determinado jogador que enviou ao servidor as coordenadas (x,y)
void boardPlay(Play_Response* resp, char* n_play, int* n_corrects, int x, int y){
    pthread_mutex_lock(&board[x][y].mutex_board);
    if(board[x][y].is_up){
        if(resp->code == 1){    //Se for a segunda jogada e for para virar a primeira carta para baixo
            resp->code = -1;
            *n_play = 0;
            fillCard(*resp, 0, resp->play1[0], resp->play1[1]);
        }else{
            printf("FILLED\n");
            resp->code = 0;
        }
    }else{
        if(!*n_play){            //Se for a primeira jogada
            printf("FIRST\n");
            resp->code = 1;
            *n_play = 1;
            resp->play1[0] = x;
            resp->play1[1] = y;
            strcpy(resp->str_play1, board[x][y].str);
            fillCard(*resp, 1, x, y);
        }else{                  //Se for a segunda jogada
            char* first_str = board[resp->play1[0]][resp->play1[1]].str;
            char* secnd_str = board[x][y].str;

            strcpy(resp->str_play1, first_str);
            resp->play2[0] = x;
            resp->play2[1] = y;
            strcpy(resp->str_play2, secnd_str);
            
            if (!strcmp(first_str, secnd_str)){
                printf("CORRECT!!!\n\n");
                *n_corrects += 2;
                if (*n_corrects == dim*dim){
                    resp->code = 3;
                }else{
                    resp->code = 2;
                }
                fillCard(*resp, 1, x, y);
                fillCard(*resp, 1, resp->play1[0], resp->play1[1]);
            }else{
                printf("INCORRECT\n\n");
                resp->code = -2;
                fillCard(*resp, 1, x, y);
                fillCard(*resp, 1, resp->play1[0], resp->play1[1]);
            }
            *n_play = 0;
        }
    }
    pthread_mutex_unlock(&board[x][y].mutex_board);
}

//Preenche uma carta do board com o respetivo dono
void fillCard(Play_Response resp, char value, int x, int y){
    board[x][y].is_up = value;
    board[x][y].code = resp.code;
    if(value){                      //Só meter preencher cor se for para virar a carta para cima e se for a primeira jogada
        board[x][y].owner_color[0] = resp.color[0];
        board[x][y].owner_color[1] = resp.color[1];
        board[x][y].owner_color[2] = resp.color[2];
    }
}