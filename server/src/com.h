#ifndef _COM_H
#define _COM_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include "player_manage.h"
#include "client_handler.h"

void broadcastBoard(Play_Response resp, char* buff_send);
int sendActualBoard(int client_fd, char* buff_send);
int initSocket(socklen_t size_addr);
int enviar(int sock_fd, void* buff, size_t size);
int receber(int client_fd, int play_recv[]);
void sendDim(int client_fd, Cmn_Thr_Data* common_data, Node_Client* client_data);
void broadcastThreads(int _sockfd);
void sendGameOver(Play_Response resp, char* buff_send, int loser_fd);

#endif