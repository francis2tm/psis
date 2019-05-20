#include "board_library.h"
#include "utils.h"
#include "player_manage.h"

extern int dim;
extern Board_Place** board;
extern pthread_mutex_t mutex_n_players;
extern int n_players;

//Aloca e preenche o board com strings
void initBoard(void){
    int count  = 0;
    int i, j;
    char* str_place;
  
    //Alocar a matriz
	board = (Board_Place**)malloc(sizeof(Board_Place*)*dim);
	verifyErr(board);
	for(i = 0; i < dim; i++){
        board[i] = NULL;
		board[i] = (Board_Place*)malloc(sizeof(Board_Place)*dim);
		if(board == NULL){
			verifyErr(board[i]);
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
            }while(str_place[0] != '\0');
            str_place[0] = c1;
            str_place[1] = c2;
            str_place[2] = '\0';
            do{
                i = rand() % dim;
                j = rand() % dim;
                str_place = board[i][j].str;
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
        cpy3CharVec(resp.color, board[x][y].owner_color);		//Copiar vetor resp.color para vetor board[x][y].owner_color
    }
}

//Verificar a jogada recebida, a ver se é preciso ignorar: se (x,y) estão dentro do dim e se ha mais do que 1 jogador
int checkPlay(int x, int y){
    //Verificar se é o único jogador
    pthread_mutex_lock(&mutex_n_players);
    if(n_players == 1){
        pthread_mutex_unlock(&mutex_n_players);
        return 1;
    }
    pthread_mutex_unlock(&mutex_n_players);

    //Verificar se as coordenadas recebidas estão dentro do board
    if(x >= dim || y >= dim || x < 0 || y < 0){                       
        return 1;
    }
    return 0;
}