#ifndef _BOARD_LIBRARY_H
#define _BOARD_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utlis.h"

typedef struct Board_Place{
  char str[3];
  char is_up;
}Board_Place;

typedef struct Play_Response{
  int code;  
            // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -1 2nd - virar primeira jogada para baixo
            // -2 2nd - diffrent
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
}Play_Response;

void initBoard(void);
Play_Response boardPlay (int x, int y);

#endif