/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: BOT
* FICHEIRO: client.c
*
* Funcionalidades: 
*	- Criar thread do bot;
*	- Enviar para o servidor uma peça do tabuleiro aleatória;
*	- Receber a resposta do servidor à escolha feita.
*****************************************************************************/
#include "client.h"
#include "com.h"

int dim;							//Tamanho do board
int sock_fd;						//Identificador da socket cliente <-> servidor

/******************************************************************************
* int main()
*
* Argumentos: int argc, char** argv;
*
* Descrição: Função que cria a thread do bot e recebe o processamento das jogadas
* vindas do servidor.
*****************************************************************************/
int main(int argc, char** argv){
	pthread_t thread_id;
	char* buff_recv = NULL;
	Play_Response resp = {.play2[0] = 0};

	if(argc != 2){
		fprintf(stderr, "Invalid args\n");
		exit(EXIT_FAILURE);
	}

	sock_fd = initSocket(argv[1]);

	buff_recv = (char*)malloc(sizeof(Play_Response));
	verifyErr(buff_recv);												//Verifica se malloc foi bem sucedido

	handShake(sock_fd);													//Primeira mensagem, receber dim	

	//Criar thread que recebe eventos SDL
	if(pthread_create(&thread_id, NULL, (void*)sendHandler, (void*)sock_fd)){
		fprintf(stderr, "Couldn't create thread\n");
		exit(-1);
	}

	//Criar main loop de receção das jogadas processadas
	while(receber(sock_fd, &resp, buff_recv) > 0){
		processResp(resp);
	}

	free(buff_recv);
	printf("Fim\n");

	return EXIT_SUCCESS;
}

/******************************************************************************
* void* sendHandler(int sock_fd)
*
* Argumentos: int sock_fd: identificador da socket bot <-> servidor;
*
* Retorno: void;

* Descrição: Thread que gera numero aleatórios que correspondem às coordenadas x
* (play[0]) e y (play[1]) e as envia ao servidor para serem processadas.
*****************************************************************************/
void* sendHandler(int sock_fd){
	int play[2];
	
	while(1){
		play[0] = rand()%dim;
		sleep(1);
		play[1] = rand()%dim;
		if(enviar(sock_fd, play) < 0){
			break;
		}
	}
					
	shutdown(sock_fd, SHUT_RDWR);				//Serve somente para a thread que está a dormir no read() ficar desbloqueada 
	close(sock_fd);
	return NULL;
}

/******************************************************************************
* void processResp(Play_Response resp)
*
* Argumentos: Play_Response resp: código do evento a realizar
*
* Retorno:  void;
* 
* Descrição: Função que processa as respostas recebidas do servidor
*****************************************************************************/
void processResp(Play_Response resp){
	switch (resp.code){
		case 1:		//Primeira jogada
			printf("FIRST\n");
			break;
		case 3:		//Acabou o jogo (todas as cartas tao up)
		case 2:		//Jogar 2x e acertar na combinação
			printf("CORRECT\n");
			break;
		case -2:	//Jogar 2x e falhar na combinação
			printf("WRONG\n");
			break;
		case 10:
			printf("CONGRATS! YOU WON!!!\n");
			break;
		case 11:
			printf("Game over\n");
			break;

	}
}