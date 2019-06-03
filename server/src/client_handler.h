/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: SERVIDOR
* FICHEIRO: client_handler.h
*
*****************************************************************************/
#ifndef _CLIENT_HANDLER_H
#define _CLIENT_HANDLER_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <poll.h>
#include "board_library.h"
#include "player_manage.h"

typedef struct Cmn_Thr_Data_Struct{
	Play_Response resp;			//Estrutura que é enviada ao cliente
	char* buff_send;		//Buffer auxiliar para enviar data ao cliente
	int sock_fd;
	int n_corrects;			//Score do jogador
	sem_t sem;
	pthread_mutex_t mutex_timer;
}Cmn_Thr_Data;

#define CANCEL_TIMER_THREAD -1

void* playerHandler(Node_Client* client_data);
void* timerHandler(Cmn_Thr_Data* common_data);
void resetMaster(Cmn_Thr_Data* common_data);
void resetSlave(Cmn_Thr_Data* common_data);

#endif