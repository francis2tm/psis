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
	sem_t sem;
	pthread_mutex_t mutex_timer;
}Cmn_Thr_Data;

#define CANCEL_TIMER_THREAD -200

void* playerHandler(Node_Client* client_data);
void* timerHandler(Cmn_Thr_Data* common_data);

#endif