#include "utils.h"
#include "board_library.h"
#include "server.h"
#include "player_manage.h"


extern int dim;
extern pthread_mutex_t mutex_color;
extern pthread_rwlock_t rwlock_stack_head;
extern pthread_rwlock_t rwlock_stack;
extern pthread_rwlock_t rwlock_score;
extern pthread_mutex_t mutex_reset;
extern Board_Place** board;


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
	if(pthread_rwlock_init(&rwlock_score, NULL)){
		fprintf(stderr, "Mutex init ");
		exit(-1);
	}
	if(pthread_mutex_init(&mutex_reset, NULL)){
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

//Faz o pthread_rwlock_(op), sendo op rdlock, wrlock ou unlock
void rwLock(char op, pthread_rwlock_t* lock){
	int err = 0;

	switch (op){
		case R_LOCK:
			err = pthread_rwlock_rdlock(lock);
			break;
		case W_LOCK:
			err = pthread_rwlock_wrlock(lock);
			break;

		case UNLOCK:
			err = pthread_rwlock_unlock(lock);
			break;
	}

	if(err){
		fprintf(stderr, "Erro com lock/unlock rw_lock");
		exit(EXIT_FAILURE);
	}
}

void mutex(char op, pthread_mutex_t* lock){
	int err = 0;

	switch (op){
		case LOCK:
			err = pthread_mutex_lock(lock);
			break;

		case UNLOCK:
			err = pthread_mutex_unlock(lock);
			break;
	}

	if(err){
		fprintf(stderr, "Erro com lock/unlock mutex");
		exit(EXIT_FAILURE);
	}
}

void semaphore(char op, sem_t* sem){
	int err = 0;

	switch (op){
		case WAIT:
			err = sem_wait(sem);
			break;

		case POST:
			err = sem_post(sem);
			break;
	}

	if(err){												//Não fazemos exit() porque pode acontecer um sem_post a uma thread q tenha acabado de sair
		fprintf(stderr, "Erro com wait/post sem");
	}
}