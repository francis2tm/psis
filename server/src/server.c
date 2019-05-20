#include "server.h"
#include "com.h"
#include "utils.h"
#include "player_manage.h"
#include "board_library.h"


int dim = 4;								//Dimensão do board
Board_Place** board = NULL;					//Duplo ponteiro, posteriormente será usado para alcocar dinâmicamente em função do dim para gerar uma matriz
char last_color[3] = {10, 25, 18};			//Última cor gerada pelo último cliente a ter sido conectado
char prox_RGB = 0;							//Variável global utilizada pela generateColor()
Node_Client* head = NULL;					//Head do stack dos jogadores
int n_players = 0;			//Estrutura que contém o numero de jogadores e o respetivo mutex

pthread_mutex_t mutex_color;
pthread_rwlock_t rwlock_stack_head;
pthread_rwlock_t rwlock_stack;
pthread_mutex_t mutex_n_players;

int main(int argc, char** argv){
	struct sockaddr_un client_addr;
	socklen_t size_addr;
	int client_fd;
	int server_fd;

	//signal(SIGPIPE, SIG_IGN);	//Caso uma thread tá a ler o nó de um jogador que acabou de se disconectar e a thread do jogador que se disconectou não tem tempo de eliminar o node correspondente

	processArgs(argc, argv);			//Processar e verificar o dim recebido pelos args

	unlink(SERVER_ADDR);

	initBoard();						//Alocar e preencher a board
	initSync();							//Inicializar sync
	server_fd = initSocket(size_addr);	//Setup socket

	//Main thread só aceita novas conexões
	while(1){
		printf(" Ready to accept connections\n");	
		if((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &size_addr)) == -1){	//Verficiar se não houve erro a fazer accept
			perror("accept");
			exit(-1);
		}

		pthread_mutex_lock(&mutex_n_players);
		if(n_players < MAX_PLAYERS){
			pthread_mutex_unlock(&mutex_n_players);
			createPlayer(client_fd);		//Criar um novo jogador
			printf("accepted connection\n");
		}else{
			pthread_mutex_unlock(&mutex_n_players);
		}
	}

	if(head != NULL){				//Só apagar se a head tiver elementos
		deleteList();				//Apagar a lista de jogadores
	}

	//Destruir as estruturas para a sincronização global
	pthread_mutex_destroy(&mutex_color);		
	pthread_rwlock_destroy(&rwlock_stack_head);
	pthread_rwlock_destroy(&rwlock_stack);
	pthread_mutex_destroy(&mutex_n_players);
	for(int i = 0; i < dim; i++){
		for(int j = 0; j < dim; j++){
			pthread_mutex_destroy(&board[i][j].mutex_board);
		}
	}
	return EXIT_SUCCESS;
}