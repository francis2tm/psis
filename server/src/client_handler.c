/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: SERVIDOR
* FICHEIRO: client_handler.c
*
* Funcionalidades: 
*	- Threads responsáveis por ativar os timers;
*	- Threads que irão estar constantemente a monitorizar jogadas.
*****************************************************************************/
#include "client_handler.h"
#include "utils.h"
#include "com.h"
#include "server.h"

extern Board_Place** board;
extern int dim;
extern pthread_mutex_t mutex_reset;
extern pthread_rwlock_t rwlock_score;
extern pthread_rwlock_t rwlock_end;
extern volatile char end_flag;
extern char reset_flag;
extern Score_List score;

/******************************************************************************
* void* playerHandler(Node_Client* client_data)
*
* Argumentos: Node_Client* client_data: Nó com informação do jogador;
*
* Retorno: void;
*
* Descrição: Função que trata de toda a gestão do jogador.
*****************************************************************************/
void* playerHandler(Node_Client* client_data){
	int play_recv[2];			//Coordenadas (x,y) recebidas pelo cliente
	char n_play = 0;			//0- n fez nenhuma jogada (ou acabou de fazer 2 jogadas) | 1- ja fez 1 jogada 
	int client_fd = client_data->sock_fd;
	Cmn_Thr_Data common_data = {.buff_send  = NULL, .resp.code = 0, .sock_fd = client_fd, .resp.play1[0] = -1, .n_corrects = 0};		//Inicializar o resp
	pthread_t thread_id;
	struct pollfd poll_sock_fd = {.fd = client_fd, .events = POLLIN};				//Estrutura para o poll()

	client_data->sem_pointer = &common_data.sem;

	insertPlayer(client_data);														//Inserir o player na pilha de jogadores

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

	//Ciclo até o cliente estiver conectado e enquanto não for para terminar o servidor
	while(receber(client_fd, play_recv) > 0){
		rwLock(R_LOCK, &rwlock_end);
		if(end_flag){
			rwLock(UNLOCK, &rwlock_end);
			break;
		}
		rwLock(UNLOCK, &rwlock_end);

		//Verifica se a jogada é válida
		if(checkPlay(play_recv[0], play_recv[1])){
			continue;
		}
		
		mutex(LOCK, &common_data.mutex_timer);									//Vamos mudar o common_data.resp, logo, bloquear a estrutura
		//Processar a jogada recebida
		if(common_data.resp.code != -2 && common_data.resp.code != 4){				//Descartar as jogadas feitas durante os 2s ou durante o timer do reset de 10s
			boardPlay(&common_data.resp, &n_play, &common_data.n_corrects, play_recv[0], play_recv[1]);	//Processar a play recebida pelo jogador
			mutex(UNLOCK, &common_data.mutex_timer);
			
			broadcastBoard(common_data.resp, common_data.buff_send);						//Mandar a todos os jogadores a play processada (enviar o resp)
			
			if(n_play == 1){											//Se tivermos na primeira jogada
				if(!poll(&poll_sock_fd, 1, 5000)){						//Ativar o timer 5s
					mutex(LOCK, &common_data.mutex_timer);							//Teoricamente a thread_timer não irá estar ativa, mas evitar race condition no resp
					
					mutex(LOCK, &board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);
					fillCard(common_data.resp, 0, common_data.resp.play1[0], common_data.resp.play1[1]);
					mutex(UNLOCK, &board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);

					common_data.resp.code = -1;
					broadcastBoard(common_data.resp, common_data.buff_send); 				//Mandar alterações do board a todos os jogadores

					common_data.resp.code = 0;
					mutex(UNLOCK, &common_data.mutex_timer);							//Desbloquear o resp
					
					n_play = 0;
					printf("poll timed out\n");
				}
				continue;
			}else if(common_data.resp.code == -2){												//Se o jogador errou na combinação, ativar timer 2s
				semaphore(POST, &common_data.sem);														//Ativar thread de timer 
				continue;
			}
			
			mutex(LOCK, &mutex_reset);
			if(common_data.resp.code == 3){														//O boardPlay() garante que só 1 thread consegue o resp.code = 3
				reset_flag = 1;
				mutex(UNLOCK, &mutex_reset);
				semaphore(POST, &common_data.sem);														//Ativar thread de timer
			}else{
				mutex(UNLOCK, &mutex_reset);
			} 
		}else{
			mutex(UNLOCK, &common_data.mutex_timer);
		}
	}
	
	//Cliente disconectou-se ou o servidor está a terminar
	//Destruição da thread timer
	mutex(LOCK, &common_data.mutex_timer);	//Ter a certeza que o thread de timer não fica com nenhuma lock antes de morrer -> esperar que ela faça tudo o que tem a fazer
	common_data.n_corrects = CANCEL_TIMER_THREAD;
	mutex(UNLOCK, &common_data.mutex_timer);		//Não é preciso pois a lock vai ser agora destruída, é meramente uma formalidade
	semaphore(POST, &common_data.sem);							//Ativar a thread timer para ela se auto destruir
	pthread_join(thread_id, NULL);

	rwLock(R_LOCK, &rwlock_end);
	if(!end_flag){										//Só vale a pena fazer estas operações se o servidor não estiver a terminar
		rwLock(UNLOCK, &rwlock_end);
		deleteNode(client_data);						//Apaga nó correspondente ao player da lista de players
		//Se o jogador tiver saído durante o timer de 5s, temos de virar a carta para baixo
		if(n_play == 1){
			mutex(LOCK, &board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);
			fillCard(common_data.resp, 0, common_data.resp.play1[0], common_data.resp.play1[1]);
			mutex(UNLOCK, &board[common_data.resp.play1[0]][common_data.resp.play1[1]].mutex_board);
			common_data.resp.code = -1;
			broadcastBoard(common_data.resp, common_data.buff_send); 				//Mandar alterações do board a todos os jogadores
		}
	}else{
		rwLock(UNLOCK, &rwlock_end);
	}

	//Libertar recursos
	sem_destroy(&common_data.sem);
	pthread_mutex_destroy(&common_data.mutex_timer);
	free(common_data.buff_send);
	printf("Client disconnected\n");
	return NULL;
}

/******************************************************************************
* void* timerHandler(Cmn_Thr_Data* common_data)
*
* Argumentos: Cmn_Thr_Data* common_data: informação da jogada;
*
* Retorno: void;
*
* Descrição: Thread que ativa os timers dependendo se é 2 ou 10 segundos,
* tratando também do reset do jogo.
*****************************************************************************/
void* timerHandler(Cmn_Thr_Data* common_data){
	while(1){
		semaphore(WAIT, &common_data->sem);							//Bloquear esta thread até que um timer seja necessário
		mutex(LOCK, &common_data->mutex_timer);			//Mutex para não deixar que o resp ser mudado pela outra thread
		if(common_data->n_corrects == CANCEL_TIMER_THREAD){		//Flag para esta thread se auto-destruir
			mutex(UNLOCK, &common_data->mutex_timer);
			break;
		}

		if(common_data->resp.code == -2){
			mutex(UNLOCK, &common_data->mutex_timer);			//Mutex para não deixar que o resp ser mudado pela outra thread

			sleep(2);
			mutex(LOCK, &common_data->mutex_timer);			//Mutex para não deixar que o resp ser mudado pela outra thread
			//Voltar as cartas erradas para baixo
			mutex(LOCK, &board[common_data->resp.play1[0]][common_data->resp.play1[1]].mutex_board);
			fillCard(common_data->resp, 0, common_data->resp.play1[0], common_data->resp.play1[1]);
			mutex(UNLOCK, &board[common_data->resp.play1[0]][common_data->resp.play1[1]].mutex_board);

			mutex(LOCK, &board[common_data->resp.play2[0]][common_data->resp.play2[1]].mutex_board);
			fillCard(common_data->resp, 0, common_data->resp.play2[0], common_data->resp.play2[1]);
			mutex(UNLOCK, &board[common_data->resp.play2[0]][common_data->resp.play2[1]].mutex_board);

			//Enviar a todos os jogadores as cartas viradas para baixo
			common_data->resp.code = -4;
			broadcastBoard(common_data->resp, common_data->buff_send); //Mandar alterações do board a todos os jogadores

			//Jogador pode voltar a fazer jogadas
			common_data->resp.code = 0;
			mutex(UNLOCK, &common_data->mutex_timer);
		}else if(common_data->resp.code == 3){					//Quando esta é a thread timer da thread que recebeu a última jogada (esta é a única que tem resp.code = 3)									
			common_data->resp.code = 4;							//Basicamente usar o resp.code como flag para a thread que recebe jogadas as descartar
			mutex(UNLOCK, &common_data->mutex_timer);			//Na boa unlock estar aqui pois a outra thread nunca vai mudar common_data com resp.code = 4
			resetMaster(common_data);
		}else if(reset_flag){									//Quando todas as N-1 timer threads tiverem que fazer o reset do seu respetivo cliente
			common_data->resp.code = 4;							//Basicamente usar o resp.code como flag para a thread que recebe jogadas as descartar
			mutex(UNLOCK, &common_data->mutex_timer);			//Na boa unlock estar aqui pois a outra thread nunca vai mudar common_data com resp.code = 4
			resetSlave(common_data);
		}else{													//Garantir que não há deadlock
			mutex(UNLOCK, &common_data->mutex_timer);
		}
		
	}
	return NULL;
}

/******************************************************************************
* void resetMaster(Cmn_Thr_Data* common_data)
*
* Argumentos: Cmn_Thr_Data* common_data: informação da jogada;
*
* Retorno: void;
*
* Descrição: Thread que terminou o jogo que irá tratar do reset do jogo. Também
* irá notificar as outras threads e inicializar o contador dos 10 segundos.
*****************************************************************************/
void resetMaster(Cmn_Thr_Data* common_data){
	
	score.sem_pointer = &common_data->sem;				//Meter o ponteiro do sem da thread master reset dentro da estrutura score, n é preciso sync pq ainda nenhuma das outras slave threads foram ativadas
	score.top_score = 0;								// Meter a 0 pq esta variavel foi usada para contar o nº jogadas durante o jogo
	tryUpdateScore(common_data->n_corrects, common_data->sock_fd);
	broadcastThreads(common_data->sock_fd);				//Notificar as outras threads do reset, meter as outras threads a descartar jogadas de jogadores
	semaphore(WAIT, &common_data->sem);						//Esperar que as threads escolham o(s) vencedor(es)

	mutex(LOCK, &common_data->mutex_timer);
	common_data->resp.code = 11;
	sendGameOver(common_data->resp, common_data->buff_send, common_data->sock_fd);	//Notificar ao jogador desta thread que o jogo acabou
	common_data->resp.code = 10;
	sendGameOver(common_data->resp, common_data->buff_send, 0);	//Notificar os vencedores
	common_data->resp.code = 4;
	mutex(UNLOCK, &common_data->mutex_timer);

	printf("Inicializando timer 10s\n");
	sleep(10);											//Timer
	initBoard(1);										//Limpa o board e repreenche-la
	broadcastBoard(common_data->resp, common_data->buff_send); 	//Mandar todos os jogadores limpar os seus boards

	broadcastThreads(common_data->sock_fd);				//Notificar todas a threads que o reset acabou

	mutex(LOCK, &common_data->mutex_timer);
	common_data->resp.code = 0;
	if(common_data->n_corrects != CANCEL_TIMER_THREAD){	//Apenas se não for para terminar, visto que usamos a n_corrects como flag para a thread timer se desligar
		common_data->n_corrects = 0;
	}
	mutex(UNLOCK, &common_data->mutex_timer);

	mutex(LOCK, &mutex_reset);
	reset_flag = 0;
	mutex(UNLOCK, &mutex_reset);

	rwLock(W_LOCK, &rwlock_score);
	score.top_score = 0;
	score.count = 0;
	deleteScore();
	rwLock(UNLOCK, &rwlock_score);
	printf("Novo jogo ready\n");
}

/******************************************************************************
* void resetSlave(Cmn_Thr_Data* common_data)
*
* Argumentos: Cmn_Thr_Data* common_data: informação da jogada;
*
* Retorno: void;
*
* Descrição: Threads que foram notificadas por uma thread irmã que têm que fazer 
* reset.
*****************************************************************************/
void resetSlave(Cmn_Thr_Data* common_data){
	tryUpdateScore(common_data->n_corrects, common_data->sock_fd);
			
	//Notificar o jogador desta thread que o jogo acabou
	mutex(LOCK, &common_data->mutex_timer);							//Mutex para não deixar que o resp ser mudado pela outra thread
	common_data->resp.code = 11;
	sendGameOver(common_data->resp, common_data->buff_send, common_data->sock_fd);	//Notificar o jogador desta thread que o jogo acabou	
	common_data->resp.code = 4;
	mutex(UNLOCK, &common_data->mutex_timer);


	semaphore(WAIT, &common_data->sem);					//Adormecer thread até que o timer 10s do reset tenha acabado (esperar pela notificação da thread reset master)
	
	mutex(LOCK, &common_data->mutex_timer);
	common_data->resp.code = 0;
	if(common_data->n_corrects != CANCEL_TIMER_THREAD){	//Apenas se não for para terminar, visto que usamos a n_corrects como flag para a thread timer se desligar
		common_data->n_corrects = 0;
	}
	mutex(UNLOCK, &common_data->mutex_timer);
}