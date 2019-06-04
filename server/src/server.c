#include "server.h"
#include "com.h"
#include "utils.h"
#include "player_manage.h"
#include "board_library.h"


int dim = 4;												//Dimensão do board
Board_Place** board = NULL;									//Duplo ponteiro, posteriormente será usado para alcocar dinâmicamente em função do dim para gerar uma matriz
char last_color[3] = {10, 25, 18};							//Última cor gerada pelo último cliente a ter sido conectado
char prox_RGB = 0;											//Variável global utilizada pela generateColor()
Node_Client* head = NULL;									//Head do stack dos jogadores
int n_players = 0;											//Numero total de jogadores conectado
Score_List score = {.top_score = 0, .head = NULL, .count = 0};//Número de peças locked ao longo do jogo & score máximo (somente atualizado no final do jogo)
char reset_flag = 0;										//Flag que fica a 1 durante o período (10s) do rest

volatile char end_flag = 0;									//Flag que indica quando for para terminar o servidor

pthread_rwlock_t rwlock_end;
pthread_rwlock_t rwlock_score;
pthread_mutex_t mutex_color;
pthread_mutex_t mutex_reset;
pthread_rwlock_t rwlock_stack_head;
pthread_rwlock_t rwlock_stack;

int main(int argc, char** argv){
	struct sockaddr_in local_addr;
	struct sockaddr_in client_addr;
	socklen_t size_addr = 0;
	int client_fd;
	int server_fd;
	

	initSigHandlers();					//Definir os sig handlers
	processArgs(argc, argv);			//Processar e verificar o dim recebido pelos args

	initBoard(0);						//Alocar e preencher a board
	initSync();							//Inicializar sync (locks)
	server_fd = initSocket(&local_addr);	//Setup socket

	//Main thread só aceita novas conexões
	while(!end_flag){
		printf("Ready to accept connections\n");	
		if((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &size_addr)) == -1){	//Verficiar se não houve erro a fazer accept
			if(end_flag){						//N é preciso rw_lock_rdlock pois esta thread é a unica que pode escrever
				break;
			}
			perror("accept");
			exit(-1);
		}

		rwLock(R_LOCK, &rwlock_stack);
		if(n_players < MAX_PLAYERS){
			rwLock(UNLOCK, &rwlock_stack);
			createPlayer(client_fd);		//Criar um novo jogador
			printf("accepted connection\n");
		}else{
			rwLock(UNLOCK, &rwlock_stack);
			close(client_fd);				//Disconectar cliente quando n_players é max
		}
	}

	if(head != NULL){				//Só apagar se a head tiver elementos
		deleteList();				//Apagar a lista de jogadores
	}

	//Destruir as estruturas para a sincronização global
	pthread_mutex_destroy(&mutex_color);
	pthread_rwlock_destroy(&rwlock_score);
	pthread_mutex_destroy(&mutex_reset);
	pthread_rwlock_destroy(&rwlock_stack_head);
	pthread_rwlock_destroy(&rwlock_stack);
	for(int i = 0; i < dim; i++){
		for(int j = 0; j < dim; j++){
			pthread_mutex_destroy(&board[i][j].mutex_board);
		}
	}
	deleteBoard();					//Libertar o board
	close(server_fd);
	
	sleep(1);
	return EXIT_SUCCESS;
}

//Signal Handler para o SIGINT
void handleSigInt(){ 
	rwLock(W_LOCK, &rwlock_end);
	end_flag = 1;
	rwLock(UNLOCK, &rwlock_end);
} 