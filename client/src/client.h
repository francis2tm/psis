/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					            Inês Moreira Nº 88050
*
* SECÇÃO: CLIENTE
* FICHEIRO: client.h
*
* Descrição: Contém a declaração das funções presentes no ficheiro "client.c"
* bem como a declaração da estrutura responsável pelo armazenamento das 
* escolhas do jogador
*****************************************************************************/

#ifndef _CLIENT_H
#define _CLIENT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include "UI_library.h"
#include "utils.h"


#define MAX_SIZE_WINDOW 800

#define PORT 3000

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
  char color[3];
  char str_play1[3], str_play2[3];
}Play_Response;

void* sdlHandler(int sock_fd);
void setActualBoard(Play_Response resp);
void updateBoard(Play_Response resp);

#endif