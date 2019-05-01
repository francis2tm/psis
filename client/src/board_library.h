#ifndef _BOARD_LIBRARY_H
#define _BOARD_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
<<<<<<< HEAD
#include "utlis.h"

typedef struct Board_Place{
  char str[3];
  char is_up;
}Board_Place;
=======

typedef struct Board_Place{
  char v[3];
} Board_Place;
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497

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

<<<<<<< HEAD
void initBoard(void);
=======
int linearConv(int i, int j);   
                              
char* getBoardPlaceStr(int i, int j);
void initBoard(int dim);
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497
Play_Response boardPlay (int x, int y);

#endif