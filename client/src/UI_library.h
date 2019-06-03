/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					            Inês Moreira Nº 88050
*
* SECÇÃO: CLIENTE
* FICHEIRO: UI_library.h
*
* Descrição: Contém a declaração das funções presentes no ficheiro "UI_library.c".
*****************************************************************************/

#ifndef UI_LIBRARY_H
#define UI_LIBRARY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>

void writeCard(int  board_x, int board_y, char * text, int r, int g, int b);
void paintCard(int  board_x, int board_y , int r, int g, int b);
void getBoardCard(int mouse_x, int mouse_y, int * board_x, int *board_y);
int createBoardWindow();
void closeBoardWindows(void);
void resetBoard();

#endif