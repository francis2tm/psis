#include "client.h"
#include "com.h"

int dim;

int main(){
	int sock_fd;
	int play[2];

	unlink(CLIENT_ADDR);

	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){			//Caso uma thread tá a ler o nó de um jogador que acabou de se disconectar e a thread do jogador que se disconectou não tem tempo de eliminar o node correspondente
		fprintf(stderr, "Couldn't define signal handler\n");
		exit(-1);
	}

	sock_fd = initSocket();

	handShake(sock_fd);													//Primeira mensagem, receber dim

	while(1){
		play[0] = rand() % dim;
		sleep(1);
		play[1] = rand() % dim;
		if(enviar(sock_fd, play) < 0){
			return EXIT_FAILURE;
		}
		printf("Sent play\n");
	}
	
	close(sock_fd);
	printf("Server Disconnected\n");
	return EXIT_SUCCESS;
}

