#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SERVER_ADDR "../server_sock"


void verifyErr(void *p);
void initSync();
void cpy3CharVec(char* src, char* dest);
void processArgs(int argc, char** argv);
void initSigHandlers();

#endif