#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_ADDR "server_sock"

#define CLIENT_ADDR "client_sock"

int main(){
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	char buff[4];
	int nbytes;


	unlink(CLIENT_ADDR);

	int sock_fd= socket(AF_UNIX, SOCK_STREAM, 0);

	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}



	printf(" socket created \n");

	client_addr.sun_family = AF_UNIX;
	strcpy(client_addr.sun_path, CLIENT_ADDR);

	int err = bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
}
	printf(" socket with adress \n");


	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SERVER_ADDR);

	int err_c = connect(sock_fd, (const struct sockaddr *) &server_addr,
							sizeof(server_addr));
	if(err_c==-1){
				printf("Error connecting\n");
				exit(-1);
	}
	printf("connected %d\n", err_c);
	int len_message;
	while(1){
		fgets(buff, 100, stdin);
		write(sock_fd, buff, strlen(buff)+1);
		read(sock_fd, buff, sizeof(buff));
		printf("valor recebido do servidor %s\n", buff);
	}
	exit(0);
}