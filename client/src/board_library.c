#include "board_library.h"

<<<<<<< HEAD
int dim = 4;
Board_Place** board;


int play1[2];
int n_corrects;

void initBoard(){
=======
int dim_board;
Board_Place* board;
int play1[2];
int n_corrects;

int linearConv(int i, int j){
  return j*dim_board+i;
}
char* getBoardPlaceStr(int i, int j){
  return board[linearConv(i, j)].v;
}

void initBoard(int dim){
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497
  int count  = 0;
  int i, j;
  char* str_place;

  n_corrects = 0;
  play1[0]= -1;
<<<<<<< HEAD
  
    //Alocar a matriz
	board = (Board_Place**)malloc(sizeof(Board_Place)*dim);
	verifyErr(board, 'm');
	for(i = 0; i < dim; i++){
		board[i] = (Board_Place*)malloc(sizeof(Board_Place)*dim);
		if(board == NULL){
			verifyErr(board[i], 'm');
		}
	}
=======
  board = malloc(sizeof(Board_Place)* dim *dim);
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497

	//Inicializar a matriz
	for(i = 0; i < (dim); i++){
		for(j = 0; j < dim; j++){
			board[i][j].str[0] = '\0';
			board[i][j].is_up = 0;
	 	}
	}

<<<<<<< HEAD
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
=======
  for (char c1 = 'a' ; c1 < ('a'+dim_board); c1++){
    for (char c2 = 'a' ; c2 < ('a'+dim_board); c2++){
      do{
        i = rand() % dim_board;
        j = rand() % dim_board;
        str_place = getBoardPlaceStr(i, j);
        printf("%d %d -%s-\n", i, j, str_place);
      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do{
        i = rand() % dim_board;
        j = rand() % dim_board;
        str_place = getBoardPlaceStr(i, j);
        printf("%d %d -%s-\n", i, j, str_place);
      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;
      if (count == dim_board*dim_board)
        return;
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497
    }
}

Play_Response boardPlay(int x, int y){
<<<<<<< HEAD

    Play_Response resp;
    resp.code =10;

    if(strcmp(board[x][y].str, "")==0){
        printf("FILLED\n");
        resp.code =0;
    }else{
        if(play1[0]== -1){
            printf("FIRST\n");
            resp.code =1;
=======

  Play_Response resp;
  resp.code =10;
  
  if(strcmp(getBoardPlaceStr(x, y), "")==0){
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
        strcpy(resp.str_play1, getBoardPlaceStr(x, y));
      }else{
        char * first_str = getBoardPlaceStr(play1[0], play1[1]);
        char * secnd_str = getBoardPlaceStr(x, y);

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
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497

            play1[0]=x;
            play1[1]=y;
            resp.play1[0]= play1[0];
            resp.play1[1]= play1[1];
            strcpy(resp.str_play1, board[x][y].str);
        }else{
            char * first_str = board[play1[0]][play1[1]].str;
            char * secnd_str = board[x][y].str;

<<<<<<< HEAD
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
=======
            strcpy(first_str, "");
            strcpy(secnd_str, "");

            n_corrects +=2;
            if (n_corrects == dim_board* dim_board)
                resp.code =3;
            else
              resp.code =2;
          }else{
            printf("INCORRECT");
            resp.code = -2;
          }
          play1[0]= -1;
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497
        }
    }
<<<<<<< HEAD
    return resp;
}
=======
  return resp;
} 
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497
