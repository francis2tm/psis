/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: BOT
* FICHEIRO: com.c
*
* Funcionalidades: 
*	-Inicializar socket que efetua a ligação cliente e servidor;
*	-Receber a dimensão do board enviada pelo servidor;
*	-Enviar as coordenadas do board selecionado pelo jogador para o cliente;
*	-Receber as informaçãoes necessárias enviadas pelo servidor depois do
* processamento de uma jogada.
*****************************************************************************/

#include "com.h"

extern int dim;

/******************************************************************************
* int initSocket()
*
* Argumentos: void
*
* Retorno:  int sock_fd: identificador da socket criada
* 
* Descrição: Função que cria uma socket e inicializa os devidos parâmetros 
* verificando a existência de erros nalgumas destas inicializações
*****************************************************************************/
int initSocket(char* server_ip){
    struct sockaddr_in server_addr;
    int sock_fd;

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){										//Verificar se não houve erro a criar a socket
		perror("socket: ");
		exit(-1);
	}

	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(PORT);


	if(connect(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		printf("Error connecting\n");
		exit(-1);
	}

    return sock_fd;
}

/******************************************************************************
* void handShake(int sock_fd) 
*
* Argumentos: int sock_fd: identificador da socket por onde vamos receber;
*
* Retorno:  void;
* 
* Descrição: Função que recebe a informação da dimensão do board (e verifica 
* se esta foi recebida corretamente).
*****************************************************************************/
void handShake(int sock_fd){
    int aux_dim;
	if(read(sock_fd, &aux_dim, sizeof(int)) <= 0){
		printf("Hand shake not possible\n");
		exit(-1);
	}

    dim = ntohl(aux_dim);
    
    printf("Hand Shake done, dim=%d \n", dim);
}

/******************************************************************************
* int enviar(int sock_fd, int play[])
*
* Argumentos: int sock_fd: identificador da socket por onde vamos enviar;
			  int play[]: coordanada (x,y) do board a enviar para a socket;
*
* Retorno:  -1 em erro;
*			 0 em sucesso;
* 
* Descrição: Função que recebendo as coordenadas que o jogador selecionou do
* 			 board as envia para o servidor para posteriormente as processar.
*****************************************************************************/
int enviar(int sock_fd, int play[]){

    play[0] = htonl(play[0]);
	play[1] = htonl(play[1]);
	
	if(write(sock_fd, play, sizeof(int)*2) < 0){
		fprintf(stderr, "Unable to send data to socket");
		return -1;
	}

	return 0;
}

/******************************************************************************
* int receber(int client_fd, Play_Response* resp, void* buff)
*
* Argumentos: int cliend_fd: socket onde a informação é recebida;
*			  void* buff: pointer para o stream de bites recebido, servindo 
*			  como intermediário;
*			  Play_Response* resp: novo resp depois do processamento efectuado 
*			  pelo servidor (cópia em memória do buff)
*
* Retorno:  int return_read: flag que verifica o sucesso do recebimento da 
* 			informação enviada pelo servidor
* 
* Descrição: Função que recebe o resp do servidor depois do processamento 
* 			 da jogada efetuada 
*****************************************************************************/
int receber(int client_fd, Play_Response* resp, void* buff){
	int return_read = 0;

    return_read = read(client_fd, buff, sizeof(Play_Response));

    memcpy((void*)resp, buff, sizeof(Play_Response));

    //Meter a ordem dos bits bem para network
	resp->play1[0] = ntohl(resp->play1[0]);
	resp->play1[1] = ntohl(resp->play1[1]);

	resp->play2[0] = ntohl(resp->play2[0]);
    resp->play2[1] = ntohl(resp->play2[1]);

    return return_read;
}