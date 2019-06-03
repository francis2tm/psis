/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					            Inês Moreira Nº 88050
*
* SECÇÃO: SERVIDOR
* FICHEIRO: board_library.h
*
* Descrição: Contém a declaração das funções presentes no ficheiro "board_library.c"
* bem como a declaração da estrutura responsável pelo armazenamento das 
* escolhas do jogador e do dono de uma carta no caso de esta já estar virada
*****************************************************************************/
#ifndef _BOARD_LIBRARY_H
#define _BOARD_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>


typedef struct Board_Place{
    char str[3];    
    char is_up;     //0 - se a carta estiver virada para baixo (carta branca)
                    //1 se a carta estiver UP ou LOCKED (carta não branca)
    char owner_color[3];
    int code;
    pthread_mutex_t mutex_board;
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
    int play1[2];   //Guarda as coordenadas (x,y) da primeira jogada
    int play2[2];   //Guarda as coordenadas (x,y) da segunda jogada
    char color[3];  //Guarda a cor do jogador que fez a jogada
    char str_play1[3];  //String correspondente às coordenadas da play1
    char str_play2[3];  //String correspondente às coordenadas da play2
}Play_Response;

void initBoard(char flag);
void boardPlay(Play_Response* resp, char* n_play, int* n_corrects, int x, int y);
void fillCard(Play_Response resp, char value, int x, int y);
int checkPlay(int x, int y);
void deleteBoard();

#endif