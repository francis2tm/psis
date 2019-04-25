#include "board_library.h"

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
  int count  = 0;
  int i, j;
  char* str_place;

  dim_board= dim;
  n_corrects = 0;
  play1[0]= -1;
  board = malloc(sizeof(Board_Place)* dim *dim);

  for( i=0; i < (dim_board*dim_board); i++){
    board[i].v[0] = '\0';
  }

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
    }
  }
}

Play_Response boardPlay(int x, int y){

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

          if (strcmp(first_str, secnd_str) == 0){
            printf("CORRECT!!!\n");

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
        }
      }
    }
  return resp;
} 