#ifndef UI_LIBRARY_H
#define UI_LIBRARY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>


void writeCard(int  board_x, int board_y, char * text, int r, int g, int b);
void paintCard(int  board_x, int board_y , int r, int g, int b);
void clearCard(int  board_x, int board_y);
void getBoardCard(int mouse_x, int mouse_y, int * board_x, int *board_y);
<<<<<<< HEAD
int createBoardWindow(int width, int height);
=======
int createBoardWindow(int width, int height,  int dim);
>>>>>>> fa22c4381a3c322722a3b62ab079ebc4bd2ec497
void closeBoardWindows();

#endif