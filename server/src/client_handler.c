#include "client_handler.h"
#include "utils.h"
#include "com.h"

extern Board_Place** board;
extern int dim;

//Handler para cada jogador, é executado pela thread associada ao jogador
void* playerHandler(Node_Client* client_data){
	int play_recv[2];			//Coordenadas (x,y) recebidas pelo cliente
	char n_play = 0;			//0- n fez nenhuma jogada (ou acabou de fazer 2 jogadas) | 1- ja fez 1 jogada 
	int n_corrects = 0;
	int client_fd = client_data->sock_fd;
	Cmn_Thr_Data common_data = {.buff_send  = NULL, .resp.code = 0, .sock_fd = client_fd, .resp.play1[0] = -1};		//Inicializar o resp
	pthread_t thread_id;
	struct pollfd poll_sock_fd = {.fd = client_fd, .events = POLLIN};				//Estrutura para o poll()

	generateColor(common_data.resp.color);	//Gerar a cor para o jogador, resp.color {R,G,B} que contém a cor do jogador, apartir de agora todos os resp desta thread terão esta cor

	//Alocar memória para buff_send
	common_data.buff_send = (char*)malloc(sizeof(Play_Response));
	verifyErr(common_data.buff_send);				//Verifica se malloc foi bem sucedido

	//Inicializar sync
	if(pthread_mutex_init(&common_data.mutex_timer, NULL)){
		fprintf(stderr, "Mutex init ");
		exit(-1);
	}

	if(sem_init(&common_data.sem, 0, 0)){
		fprintf(stderr, "Semaphore init ");
		exit(-1);
	}

	//Enviar dimensão do board
	sendDim(client_fd, &common_data, client_data);

	//Criar thread para o timer de 2s
	if(pthread_create(&thread_id, NULL, (void*)timerHandler, (void*)&common_data)){			//Verificar se não houve erro a criar threads
		fprintf(stderr, "Couldn't create thread\n");
		exit(-1);
	}

	//Ciclo até o cliente estiver conectado
	while(receber(client_fd, play_recv) > 0){
		
		//Verifica se a jogada é válida
		if(checkPlay(play_recv[0], play_recv[1])){
			continue;
		}
		//Processar a jogada recebida
		if(common_data.resp.code != -2){													//Este if serve para descartar as jogadas feitas durante os 2s
			pthread_mutex_lock(&common_data.mutex_timer);									//Vamos mudar o common_data.resp, logo, bloquear a estrutura
			boardPlay(&common_data.resp, &n_play, &n_corrects, play_recv[0], play_recv[1]);	//Processar a play recebida pelo jogador
			pthread_mutex_unlock(&common_data.mutex_timer);
			
			broadcastBoard(common_data.resp, common_data.buff_send);						//Mandar a todos os jogadores a play processada (enviar o resp)
			
			if(common_data.resp.code == -2){												//Se o jogador errou na combinação, ativar timer 2s
				sem_post(&common_data.sem);													//Ativar thread de timer 
			}

			if(n_play == 1){																//Se tivermos na primeira jogada
				if(!poll(&poll_sock_fd, 1, 5000)){											//Ativar o timer 5s
					pthread_mutex_lock(&common_data.mutex_timer);							//Teoricamente a thread_timer não irá estar ativa, mas evitar race condition no resp
					
					pthread_mutex_lock(&board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);
					fillCard(common_data.resp, 0, common_data.resp.play1[0], common_data.resp.play1[1]);
					pthread_mutex_unlock(&board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);

					common_data.resp.code = -1;
					broadcastBoard(common_data.resp, common_data.buff_send); 				//Mandar alterações do board a todos os jogadores

					common_data.resp.code = 0;
					pthread_mutex_unlock(&common_data.mutex_timer);							//Desbloquear o resp
					
					n_play = 0;
					printf("poll timed out\n");
				}
			}
		}
	}
	//Cliente disconectou-se
	pthread_cancel(thread_id);
	pthread_join(thread_id, NULL);

	deleteNode(client_data);				//Apaga nó correspondente ao player da lista de players

	//Se o jogador tiver saído durante o timer de 5s, temos de virar a carta para baixo
	if(n_play == 1){
		pthread_mutex_lock(&board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);
		fillCard(common_data.resp, 0, common_data.resp.play1[0], common_data.resp.play1[1]);
		pthread_mutex_unlock(&board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);
		printf("Entrei\n");

		common_data.resp.code = -1;
		broadcastBoard(common_data.resp, common_data.buff_send); 				//Mandar alterações do board a todos os jogadores
	}

	updateNumPlayers(0);					//Decrementar a variável que contém o numero de jogadores online

	//Libertar recursos
	sem_destroy(&common_data.sem);
	pthread_mutex_destroy(&common_data.mutex_timer);
	free(common_data.buff_send);
	printf("Client disconnected\n");
	return NULL;
}

void* timerHandler(Cmn_Thr_Data* common_data){
	while(1){
		sem_wait(&common_data->sem);							//Bloquear esta thread até que um timer seja necessário
		
		sleep(2);

		pthread_mutex_lock(&common_data->mutex_timer);			//Mutex para não deixar que o resp ser mudado pela outra thread
		//Voltar as cartas erradas para baixo
		pthread_mutex_lock(&board[common_data->resp.play1[0]][common_data->resp.play1[1]].mutex_board);
		fillCard(common_data->resp, 0, common_data->resp.play1[0], common_data->resp.play1[1]);
		pthread_mutex_unlock(&board[common_data->resp.play1[0]][common_data->resp.play1[1]].mutex_board);

		pthread_mutex_lock(&board[common_data->resp.play2[0]][common_data->resp.play2[1]].mutex_board);
		fillCard(common_data->resp, 0, common_data->resp.play2[0], common_data->resp.play2[1]);
		pthread_mutex_unlock(&board[common_data->resp.play2[0]][common_data->resp.play2[1]].mutex_board);

		//Enviar a todos os jogadores as cartas viradas para baixo
		common_data->resp.code = -4;
		broadcastBoard(common_data->resp, common_data->buff_send); //Mandar alterações do board a todos os jogadores

		//Jogador pode voltar a fazer jogadas
		common_data->resp.code = 0;
		pthread_mutex_unlock(&common_data->mutex_timer);
	}
}