#ifndef _BOARD_LIBRARY_H
#define _BOARD_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Board_Place{
  char v[3];
} Board_Place;

typedef struct Play_Response{
  int code;  
            // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - diffrent
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
}Play_Response;

int linearConv(int i, int j);   
                              
char* getBoardPlaceStr(int i, int j);
void initBoard(int dim);
Play_Response boardPlay (int x, int y);

#endif