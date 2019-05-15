#ifndef _PLAYER_MANAGE_H
#define _PLAYER_MANAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "board_library.h"


typedef struct Node_Client_Struct{
    int sock_fd;
    struct Node_Client_Struct* next;
    struct Node_Client_Struct* prev;
}Node_Client;

/*typedef struct Score_Struct{
    unsigned int max_score;
    Node_Client** score_head;
    pthread_mutex_t mutex_score;
}Score;*/

void createPlayer(int client_fd);
Node_Client* insertPlayer(int sock_fd);
void deleteNode(Node_Client* deletingNode);
void deleteList();
void generateColor(char color[]);
void broadcastBoard(Play_Response resp, char* buff_send);

#endif