#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <semaphore.h>


void verifyErr(void *p);
void initSync();
void cpy3CharVec(char* src, char* dest);
void processArgs(int argc, char** argv);
void initSigHandlers();
void rwLock(char op, pthread_rwlock_t* lock);
void mutex(char op, pthread_mutex_t* lock);
void semaphore(char op, sem_t* sem);

#endif