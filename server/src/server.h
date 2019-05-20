#ifndef _SERVER_H
#define _SERVER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define MAX_DIM 36 	//sqrt(26*26*2) = 36.77  ~ 36 (para ser uma matriz quadrada)
#define IS_EVEN(x) ( ((x)%2 ) ? (0) : (1))		//Retorna 1 se for x for par, 0 se for ímpar 
#define MAX_PLAYERS 34                          //Limitado pelo algoritmo de geração de cores generateColor()

#endif