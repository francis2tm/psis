#ifndef _BOARD_LIBRARY_H
#define _BOARD_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

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
            // -2 2nd - combinação errada, meter as cartas vermelhas durante 2s
            // -4 2nd - combinação errada, virar as cartas para branco  ao fim dos 2s
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
}Play_Response;

void initBoard(void);
void boardPlay(Play_Response* resp, char* n_play, int x, int y);

#endif