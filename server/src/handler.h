#ifndef _HANDLER_H
#define _HANDLER_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <poll.h>
#include "player_manage.h"
#include "board_library.h"


typedef struct Cmn_Thr_Data_Struct{
	Play_Response resp;			//Estrutura que é enviada ao cliente
	char* buff_send;		//Buffer auxiliar para enviar data ao cliente
	int sock_fd;
	sem_t sem;
	pthread_mutex_t mutex_timer;
}Cmn_Thr_Data;

void* playerHandler(Node_Client* client_data);
void* timerHandler(Cmn_Thr_Data* common_data);
void sendActualBoard(int client_fd, char* buff_send);


#endif