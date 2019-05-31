#ifndef _COM_H
#define _COM_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include "client.h"

int initSocket();
void handShake(int sock_fd);
int enviar(int sock_fd, int play[]);

#endif