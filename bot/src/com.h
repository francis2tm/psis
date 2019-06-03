/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: BOT
* FICHEIRO: com.h
*
* Descrição: Contém a declaração das funções presentes no ficheiro "com.c".
*****************************************************************************/

#ifndef _COM_H
#define _COM_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include "client.h"

int initSocket(char* server_ip);
void handShake(int sock_fd);
int enviar(int sock_fd, int play[]);
int receber(int client_fd, Play_Response* resp, void* buff);

#endif