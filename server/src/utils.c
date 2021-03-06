/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: SERVIDOR
* FICHEIRO: utils.c
*
* Funcionalidades: 
*	- Verificar erros de alocação de memória;
*   - Verificar validade dos argumentos de entrada;
*   - Inicialização de variáveis de sincronização;
*   - Ignorar sinais;
*	- Generalização de variáveis de sincronização;
*****************************************************************************/
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


/******************************************************************************
* void verifyErr(void *p)
*
* Argumentos: *p:  ponteiro genérico
*
* Retorno:  void
*
* Descrição: Verifica se um ponteiro aponta para null ou não. Função utilizada 
* para verificar o sucesso de mallocs ou de abertura de ficheiros
*****************************************************************************/
void verifyErr(void *p){
	if(p == NULL){
		fprintf(stderr, "Erro a alocar memoria");
		exit(EXIT_FAILURE);
	}
}

/******************************************************************************
* void initSync()
*
* Argumentos: null;
*
* Retorno:  void;
*
* Descrição: Inicializar variáveis de sincronização e verificar se foram bem 
* inicializadas;
*****************************************************************************/
void initSync(){

	if(pthread_mutex_init(&mutex_color, NULL)){
		fprintf(stderr, "Mutex init ");
		exit(EXIT_FAILURE);
	}
	if(pthread_rwlock_init(&rwlock_stack_head, NULL)){
		fprintf(stderr, "rw_lock init ");
		exit(EXIT_FAILURE);
	}
	if(pthread_rwlock_init(&rwlock_stack, NULL)){
		fprintf(stderr, "rw_lock init ");
		exit(EXIT_FAILURE);
	}
	if(pthread_rwlock_init(&rwlock_score, NULL)){
		fprintf(stderr, "Mutex init ");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&mutex_reset, NULL)){
		fprintf(stderr, "Mutex init ");
		exit(EXIT_FAILURE);
	}


	for(int i = 0; i < dim; i++){
		for(int j = 0; j < dim; j++){
			if(pthread_mutex_init(&board[i][j].mutex_board, NULL)){
				fprintf(stderr, "mutex init\n");
				exit(EXIT_FAILURE);
			}
		}
	}	
}

/******************************************************************************
* inline void cpy3CharVec(char* src, char* dest)
*
* Argumentos: char* src: de onde quero copiar;
*			  char* dest: para onde quero copiar;
*
* Retorno:  void;
*
* Descrição: Função que copia a informação da cor do jogador (3 RGB) para a pintar 
*****************************************************************************/
inline void cpy3CharVec(char* src, char* dest){
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

/******************************************************************************
* void processArgs(int argc, char** argv)
*
* Argumentos: int argc: numero de argumentos;
*			  char** argv: argumentos;
*
* Retorno:  void;
*
* Descrição: Função que verifica se os argumentos recebidos são 2 e se o 
* segundo argumento (dimensão do board) é maior que 2 e menor que a dimensão 
* máxima permitida. Se tal acontecer guarda a dimensão do baord inserida.
*****************************************************************************/
void processArgs(int argc, char** argv){
	if(argc != 2){	//Só pode haver o argumento a indiar a dimensão
		perror("Invalid input arguments");
		exit(EXIT_FAILURE);
	}
	dim = atoi(argv[1]);

	if(!IS_EVEN(dim) || dim > MAX_DIM || dim < 2){
		fprintf(stderr, "Invalid input dimension\n");
		exit(EXIT_FAILURE);
	}
}

/******************************************************************************
* void initSigHandlers()
*
* Argumentos: null;
*
* Retorno:  void;
*
* Descrição: Define os 2 signal handlers, isto é ignorar SIGPIPE para garantir 
* longevidade do servidor e SIGINT para desligar o servidor.
*****************************************************************************/
void initSigHandlers(){
	struct sigaction a;

	a.sa_handler = handleSigInt;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);

	if(sigaction( SIGINT, &a, NULL ) == -1){			//Definir comportamento quando o processo "apanha" o SIGINT (CTRL+C), serve para terminarmos o servidor
		fprintf(stderr, "Couldn't define signal handler\n");
		exit(EXIT_FAILURE);
	}	
	
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){			//Caso uma thread tá a ler o nó de um jogador que acabou de se disconectar e a thread do jogador que se disconectou não tem tempo de eliminar o node correspondente
		fprintf(stderr, "Couldn't define signal handler\n");
		exit(EXIT_FAILURE);
	}
}

/******************************************************************************
* void rwLock(char op, pthread_rwlock_t* lock)
*
* Argumentos: char op: escolha entre 'rdlock', 'rdlock' e 'unlock';
*			  pthread_rwlock_t* lock: identificador;
*
* Retorno:  void;
*
* Descrição: Função que faz pthread_rwlock_(op), sendo op rdlock, rdlock ou 
* unlock. Verifica se tanto os locks como unlocks foram efectuados com sucesso
*****************************************************************************/
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
		//fprintf(stderr, "Erro com lock/unlock rwlock sem");
		exit(EXIT_FAILURE);
	}
}

/******************************************************************************
* void mutex(char op, pthread_mutex_t* lock)
*
* Argumentos: char op: escolha entre 'lock' e 'unlock'
*			  pthread_mutex_t* lock: identificador;
*
* Retorno:  void;
*
* Descrição: Função que faz pthread_mutex_(op), sendo op lock, ou unlock. 
* Verifica se tanto os locks como unlocks foram efectuados com sucesso.
*****************************************************************************/
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
		//fprintf(stderr, "Erro lock/unlock com mutex");
		exit(EXIT_FAILURE);
	}
}

/******************************************************************************
* void semaphore(char op, sem_t* sem)
*
* Argumentos: char op: escolha entre 'wait' e 'post'
*			  sem_t* sem* lock: identificador;
*
* Retorno:  void;
*
* Descrição: Função que faz sem_(op), sendo op wait, ou post dependendo se
* queremos incrementar ou decrementar o semáforo. Verifica se tanto os sem_wait
* como sem_post foram efectuados com sucesso.
*****************************************************************************/
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
		exit(EXIT_FAILURE);
	}
}