#include "handler.h"

extern Board_Place** board;
extern int dim;

//Handler para cada jogador, é executado pela thread associada ao jogador
void* playerHandler(Node_Client* client_data){
	int play_recv[2];			//Coordenadas (x,y) recebidas pelo cliente
	char n_play;				//0- n fez nenhuma jogada (ou acabou de fazer 2 jogadas) | 1- ja fez 1 jogada 
	Cmn_Thr_Data common_data = {.buff_send  = NULL};
	int n_corrects = 0;
	int client_fd = client_data->sock_fd;
	pthread_t thread_id;

	//Inicializar o resp
	common_data.resp.code = 0;
	common_data.sock_fd = client_fd;				
    common_data.resp.play1[0] = -1;

	generateColor(common_data.resp.color);	//Gerar a cor para o jogador, resp.color {R,G,B} que contém a cor do jogador, apartir de agora todos os resp desta thread terão esta cor

	//Alocar memória para buff_send
	common_data.buff_send = (char*)malloc(sizeof(Play_Response));
	verifyErr(common_data.buff_send, 'm');				//Verifica se malloc foi bem sucedido

	if(pthread_mutex_init(&common_data.mutex_timer, NULL)){
		perror("Mutex init ");
		exit(-1);
	}

	if(sem_init(&common_data.sem, 0, 0)){
		perror("Semaphore init ");
		exit(-1);
	}

	//Enviar o dim
	write(client_fd, (void*)&dim, sizeof(int));

	//Enviar board atual
	sendActualBoard(client_fd, common_data.buff_send);

	if(pthread_create(&thread_id, NULL, (void*)timerHandler, (void*)&common_data)){			//Verificar se não houve erro a criar threads
		perror("Couldn't create thread\n");
		exit(-1);
	}

	//Ciclo até o cliente estiver conectado
	while(read(client_fd, play_recv, sizeof(int)*2) > 0){
		if(common_data.resp.code != -2){													//Este if serve para descartar as jogadas feitas durante os 2s
			pthread_mutex_lock(&common_data.mutex_timer);
			printf("(%d,%d)\n", play_recv[0], play_recv[1]);
			boardPlay(&common_data.resp, &n_play, &n_corrects, play_recv[0], play_recv[1]);	//Processar a play recebida pelo jogador
			
			pthread_mutex_unlock(&common_data.mutex_timer);
			if(n_play == 1){																//Ativar timer 5s
				sem_post(&common_data.sem);													//Ativar thread de timer
			}
			
			memcpy(common_data.buff_send, &common_data.resp, sizeof(Play_Response));
			broadcastBoard(common_data.resp, common_data.buff_send);						//Mandar a todos os jogadores a play processada (enviar o resp)
			
			if(common_data.resp.code == -2){												//Se o jogador errou na combinação, ativar timer 2s
				sem_post(&common_data.sem);													//Ativar thread de timer 
			}
		}
	}

	//Libertar recursos
	//NÃO ESQUECER DE PTHREAD_JOIN
	sem_destroy(&common_data.sem);
	pthread_mutex_destroy(&common_data.mutex_timer);
	free(common_data.buff_send);
	deleteNode(client_data);
	printf("Client disconnected\n");
	return NULL;
}

void sendActualBoard(int client_fd, char* buff_send){
    Play_Response resp;

    for(int i = 0; i < dim; i++){
        for(int j = 0; j < dim; j++){
            pthread_mutex_lock(&board[i][j].mutex_board);
            if(board[i][j].is_up){                                  //Só enviar cartas que estão para cima
				resp.code = board[i][j].code;
				resp.color[0] = board[i][j].owner_color[0];
				resp.color[1] = board[i][j].owner_color[1];
				resp.color[2] = board[i][j].owner_color[2];
				strcpy(resp.str_play1, board[i][j].str);
				pthread_mutex_unlock(&board[i][j].mutex_board);

				resp.play1[0] = i;
				resp.play1[1] = j;
				resp.play2[0] = -1;

				memcpy(buff_send, &resp, sizeof(Play_Response));
				write(client_fd, buff_send, sizeof(Play_Response));
            }else{
				pthread_mutex_unlock(&board[i][j].mutex_board);
			}
        }
    }
	//Avisar o servidor que acabou a transmissão do board atual
	resp.play2[0] = 0;
	memcpy(buff_send, &resp, sizeof(Play_Response));
	write(client_fd, buff_send, sizeof(Play_Response));
}

void* timerHandler(Cmn_Thr_Data* common_data){
	/*struct pollfd poll_sock_fd[1];

	poll_sock_fd[0].fd = common_data->sock_fd;

	poll_sock_fd[0].events = POLLIN;*/

	while(1){
		sem_wait(&common_data->sem);							//Bloquear esta thread até que um timer seja necessário
		
		pthread_mutex_lock(&common_data->mutex_timer);

		if(common_data->resp.code == 1){						//Timer de 5s
			pthread_mutex_unlock(&common_data->mutex_timer);	//Livrar do mutex antes de adormecer
			/*printf("Entrei no poll\n");
			if(!poll(poll_sock_fd, 1, 5000)){
				printf("poll timed out\n");
			}
			printf("Bazei do poll\n");*/
		}else if(common_data->resp.code == -2){					//Timer de 2s
			pthread_mutex_unlock(&common_data->mutex_timer);	//Livrar do mutex antes de adormecer
			sleep(2);

			//Voltar as cartas erradas para baixo
			pthread_mutex_lock(&board[common_data->resp.play1[0]][common_data->resp.play1[1]].mutex_board);
			//board[common_data->resp.play1[0]][common_data->resp.play1[1]].is_up = 0;

			fillCard(common_data->resp, 0, common_data->resp.play1[0], common_data->resp.play1[1]);
			
			pthread_mutex_unlock(&board[common_data->resp.play1[0]][common_data->resp.play1[1]].mutex_board);

			pthread_mutex_lock(&board[common_data->resp.play2[0]][common_data->resp.play2[1]].mutex_board);

            //board[common_data->resp.play2[0]][common_data->resp.play2[1]].is_up = 0;

			fillCard(common_data->resp, 0, common_data->resp.play2[0], common_data->resp.play2[1]);

			pthread_mutex_unlock(&board[common_data->resp.play2[0]][common_data->resp.play2[1]].mutex_board);

			common_data->resp.code = -4;
			memcpy(common_data->buff_send, &common_data->resp, sizeof(Play_Response));
			
			broadcastBoard(common_data->resp, common_data->buff_send); //Mandar alterações do board a todos os jogadores

			pthread_mutex_lock(&common_data->mutex_timer);
			common_data->resp.code = 0;
			pthread_mutex_unlock(&common_data->mutex_timer);
		}else{														//Teoricamente nnc acontece, mas evitar deadlocks
			pthread_mutex_unlock(&common_data->mutex_timer); 		
		}
	}
}