#include "utils.h"
#include "board_library.h"
#include "server.h"
#include "player_manage.h"


extern int dim;
extern pthread_mutex_t mutex_color;
extern pthread_rwlock_t rwlock_stack_head;
extern pthread_rwlock_t rwlock_stack;
extern Board_Place** board;
extern pthread_mutex_t mutex_n_players;

/*****************************************************************************************************
 * verifyErr ()
 *  Arguments: p: ponteiro genérico
 *  Returns: void
 *  Description: Verifica se um ponteiro aponta para null ou não. Função utilizada para verificar o sucesso
 * de mallocs ou de abertura de ficheiros
 ****************************************************************************************************/
void verifyErr(void *p){
	if(p == NULL){
		fprintf(stderr, "Erro a alocar memoria");
		exit(0);
	}
}

//Inicializar variáveis de sincronização
void initSync(){

	if(pthread_mutex_init(&mutex_color, NULL)){
		fprintf(stderr, "Mutex init ");
		exit(-1);
	}
	if(pthread_rwlock_init(&rwlock_stack_head, NULL)){
		fprintf(stderr, "rw_lock init ");
		exit(-1);
	}
	if(pthread_rwlock_init(&rwlock_stack, NULL)){
		fprintf(stderr, "rw_lock init ");
		exit(-1);
	}
	if(pthread_mutex_init(&mutex_n_players, NULL)){
		fprintf(stderr, "Mutex init ");
		exit(-1);
	}

	for(int i = 0; i < dim; i++){
		for(int j = 0; j < dim; j++){
			if(pthread_mutex_init(&board[i][j].mutex_board, NULL)){
				fprintf(stderr, "mutex init\n");
				exit(-1);
			}
		}
	}	
}

//Copia um vetor de 3chars src para outro vetor de 3chars dest (estes vetores não são strings)
inline void cpy3CharVec(char* src, char* dest){
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

//Verifica se os argumentos de entrada são válidos e depois processa-os
void processArgs(int argc, char** argv){
	if(argc != 2){	//Só pode haver o argumento a indiar a dimensão
		perror("Invalid input arguments");
		exit(1);
	}
	dim = atoi(argv[1]);

	if(!IS_EVEN(dim) || dim > MAX_DIM || dim < 2){
		fprintf(stderr, "Invalid input dimension\n");
		exit(-1);
	}
}

//Define os 2 signal handlers: Ignorar SIGPIPE para garantir longevidade do servidor e SIGINT para desligar o servidor
void initSigHandlers(){
	struct sigaction a;

	a.sa_handler = handleSigInt;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);

	if(sigaction( SIGINT, &a, NULL ) == -1){			//Definir comportamento quando o processo "apanha" o SIGINT (CTRL+C), serve para terminarmos o servidor
		fprintf(stderr, "Couldn't define signal handler\n");
		exit(-1);
	}	
	
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){			//Caso uma thread tá a ler o nó de um jogador que acabou de se disconectar e a thread do jogador que se disconectou não tem tempo de eliminar o node correspondente
		fprintf(stderr, "Couldn't define signal handler\n");
		exit(-1);
	}
}