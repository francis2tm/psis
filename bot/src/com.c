#include "com.h"


extern int dim;

int initSocket(){
    struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
    int sock_fd;

    if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
		fprintf(stderr, "socket: ");
		exit(-1);
	}

	client_addr.sun_family = AF_UNIX;
	strcpy(client_addr.sun_path, CLIENT_ADDR);

	
	if(bind(sock_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1){
		perror("bind");
		exit(-1);
	}

	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SERVER_ADDR);

	if(connect(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		printf("Error connecting\n");
		exit(-1);
	}

    return sock_fd;
}

void handShake(int sock_fd){
    int aux_dim;
	if(read(sock_fd, &aux_dim, sizeof(int)) <= 0){
		printf("Hand shake not possible\n");
		exit(-1);
	}

    dim = ntohl(aux_dim);
    
    printf("Hand Shake done, dim=%d \n", dim);
}

int enviar(int sock_fd, int play[]){

    play[0] = htonl(play[0]);
	play[1] = htonl(play[1]);
	
	if(write(sock_fd, play, sizeof(int)*2) < 0){
		fprintf(stderr, "Unable to send data to socket");
		return -1;
	}

	return 0;
}