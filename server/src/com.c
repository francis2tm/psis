#include "com.h"
#include "utils.h"


extern int dim;
extern pthread_mutex_t mutex_color;
extern pthread_rwlock_t rwlock_stack_head;
extern pthread_rwlock_t rwlock_stack;
extern pthread_rwlock_t rwlock_score;
extern Score_List score;
extern Board_Place** board;
extern Node_Client* head;

//Faz broadcast das alterações da board a todos os jogadores, iterando pela pilha de jogadores
void broadcastBoard(Play_Response resp, char* buff_send){
    Node_Client* aux;

	//Meter a ordem dos bits bem para network, Como o resp não está a ser passado por referência, os seus valores locais a esta função podem ser alterados
	resp.play1[0] = htonl(resp.play1[0]);
	resp.play1[1] = htonl(resp.play1[1]); 

	resp.play2[0] = htonl(resp.play2[0]);
    resp.play2[1] = htonl(resp.play2[1]); 


	memcpy(buff_send, &resp, sizeof(Play_Response));

	pthread_rwlock_rdlock(&rwlock_stack_head);
	if(head != NULL){												//Só enviar se houver clientes
		//Enviar à head
		enviar(head->sock_fd, buff_send, sizeof(Play_Response));

		pthread_rwlock_rdlock(&rwlock_stack);						//Bloquear para leitura a lista toda
		aux = head->next;
		pthread_rwlock_unlock(&rwlock_stack_head);					//Já podemos desbloquear a head

		//Enviar ao resto da lista
		while(aux != NULL){
			enviar(aux->sock_fd, buff_send, sizeof(Play_Response));
			aux = aux->next;
		}
		pthread_rwlock_unlock(&rwlock_stack);
	}else{
		pthread_rwlock_unlock(&rwlock_stack_head);
	}    
}

//Envia o estado do board atual a um novo jogador, retorna se houve erro ao mandar
int sendActualBoard(int client_fd, char* buff_send){
    Play_Response resp;
	int err = 0;

    for(int i = 0; i < dim; i++){
        for(int j = 0; j < dim; j++){
            pthread_mutex_lock(&board[i][j].mutex_board);
            if(board[i][j].is_up){                                  //Só enviar cartas que estão para cima
				resp.code = board[i][j].code;
				cpy3CharVec(board[i][j].owner_color, resp.color);
				strcpy(resp.str_play1, board[i][j].str);
				pthread_mutex_unlock(&board[i][j].mutex_board);

				resp.play1[0] = htonl(i);
				resp.play1[1] = htonl(j);
				resp.play2[0] = htonl(-1);

				memcpy(buff_send, &resp, sizeof(Play_Response));
				err = enviar(client_fd, buff_send, sizeof(Play_Response));
            }else{
				pthread_mutex_unlock(&board[i][j].mutex_board);
			}
        }
    }
	//Avisar o servidor que acabou a transmissão do board atual
	resp.play2[0] = htonl(0);
	memcpy(buff_send, &resp, sizeof(Play_Response));
	err = enviar(client_fd, buff_send, sizeof(Play_Response));
	return err;
}

//Incializa a socket
int initSocket(socklen_t size_addr){
	int server_fd;
	struct sockaddr_un local_addr;
	
	if((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){										//Verificar se não houve erro a criara socket
		perror("socket: ");
		exit(-1);
	}

	local_addr.sun_family = AF_UNIX;
	strcpy(local_addr.sun_path, SERVER_ADDR);

	if(bind(server_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0){					//Verificar se não houve erro a fazer bind
		perror("bind");
		exit(-1);
	}
	printf(" socket created and binded \n");

	if(listen(server_fd, 2) == -1){																//Verificar se não houve erro a fazer listen
		perror("listen");
		exit(-1);
	}

	return server_fd;
}

//Envia data processada ao cliente com o correspondente sock_fd, retorna -1 se obteve algum erro
int enviar(int sock_fd, void* buff, size_t size){
	if(write(sock_fd, buff, size) < 0){
		fprintf(stderr, "Unable to send data to socket");
		return -1;
	}
	return 0;
}

int receber(int client_fd, int play_recv[]){
	int return_read = 0;

	return_read = read(client_fd, play_recv, sizeof(int)*2);
	play_recv[0] = ntohl(play_recv[0]);
	play_recv[1] = ntohl(play_recv[1]);
	
	return return_read;
}

//Enviar o dim e board atual e verificar se houve erro a enviar, se sim, sair da thread
void sendDim(int client_fd, Cmn_Thr_Data* common_data, Node_Client* client_data){
	int aux_dim;

	aux_dim = htonl(dim);
	if(enviar(client_fd, (void*)&aux_dim, sizeof(int)) < 0 || sendActualBoard(client_fd, common_data->buff_send) < 0){
		deleteNode(client_data);
		sem_destroy(&common_data->sem);
		pthread_mutex_destroy(&common_data->mutex_timer);
		free(common_data->buff_send);
		pthread_exit(NULL);
	}
}

//Notificar todas as threads do reset do jogo
void broadcastThreads(int _sockfd){
	Node_Client* aux;

	pthread_rwlock_rdlock(&rwlock_stack_head);
	if(head != NULL){												//Só enviar se houver clientes
		//Notificar a head
		if(head->sock_fd != _sockfd){								//Não fazer sem_post à thread "responsável" pelo reset
			sem_post(head->sem_pointer);
		}
		

		pthread_rwlock_rdlock(&rwlock_stack);						//Bloquear para leitura a lista toda
		aux = head->next;
		pthread_rwlock_unlock(&rwlock_stack_head);					//Já podemos desbloquear a head

		//Notificar o resto da lista
		while(aux != NULL){
			if(aux->sock_fd != _sockfd){								//Não fazer sem_post à thread "responsável" pelo reset
				sem_post(aux->sem_pointer);
			}
			aux = aux->next;
		}
		pthread_rwlock_unlock(&rwlock_stack);
	}else{
		pthread_rwlock_unlock(&rwlock_stack_head);
	}
}

void sendToWinners(Play_Response resp, char* buff_send){
	Score_Node* aux;

	memcpy(buff_send, &resp, sizeof(Play_Response));

	pthread_rwlock_rdlock(&rwlock_score);
	for(aux = score.head; aux != NULL; aux = aux->next){
		enviar(aux->sock_fd, buff_send, sizeof(Play_Response));
	}

	pthread_rwlock_unlock(&rwlock_score);
}