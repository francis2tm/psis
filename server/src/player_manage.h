#ifndef _PLAYER_MANAGE_H
#define _PLAYER_MANAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "board_library.h"


typedef struct Node_Client_Struct{
    int sock_fd;
    sem_t* sem_pointer;
    struct Node_Client_Struct* next;
    struct Node_Client_Struct* prev;
}Node_Client;

typedef struct Score_Node_Struct{
    int sock_fd;
    struct Score_Node_Struct* next;
}Score_Node;

typedef struct Score_List_Struct{
    Score_Node* head;
    int top_score;
    int count;
    sem_t* sem_pointer;
}Score_List;



void createPlayer(int client_fd);
void insertPlayer(Node_Client* aux);
void deleteNode(Node_Client* deletingNode);
void deleteList();
void generateColor(char color[]);
void broadcastBoard(Play_Response resp, char* buff_send);
void insertScore(int _sock_fd);
void deleteScore();
void tryUpdateScore(int n_corrects, int sock_fd);

#endif