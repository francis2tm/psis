#include "main.h"

#define SOCK_ADDRESS "server_sock"

Board_Place** board = NULL;
int dim = 4;

int main(){
	int main_sock;
	struct sockaddr_un local_addr;
	struct sockaddr_un client_addr;
	socklen_t size_addr;

	char buff[100];
	int nbytes;

	main_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(main_sock < 0){
		perror("socket: ");
		exit(-1);
	}

	//Setup socket

	local_addr.sun_family = AF_UNIX;
	strcpy(local_addr.sun_path, SOCK_ADDRESS);

	if(bind(main_sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0){
		perror("bind");
		exit(-1);
	}
	printf(" socket created and binded \n");

	if(listen(main_sock, 2) == -1){
		perror("listen");
		exit(-1);
	}

	while(1){
		printf("%d Ready to accept connections\n");
		int client_fd = accept(main_sock, (struct sockaddr*)&client_addr, &size_addr);
		if(client_fd == -1) {
			perror("accept");
			exit(-1);
		}
		printf("acceped connection from %s\n", client_addr.sun_path);
		int err_rcv;
		int len_message;
		while((err_rcv = recv(client_fd, buff, 100, 0)) >0 ){
			printf("%d %d - %s\n", getpid(), err_rcv, buff);
			len_message = getpid();
			write(client_fd, &len_message, sizeof(len_message));
		}
	}
		//printf("existing %d\n", err_rcv);
	unlink(SOCK_ADDRESS);
	return EXIT_SUCCESS;
}


/*int main_2(){
	
	int done = 0;

	initBoard();

	while (!done){
		int board_x, board_y;

		Play_Response _resp = boardPlay(board_x, board_y);
		switch (_resp.code){
			case 1:		//Primeira jogada
				paintCard(_resp.play1[0], _resp.play1[1], 7, 200, 100);
				writeCard(_resp.play1[0], _resp.play1[1], _resp.str_play1, 200, 200, 200);
				board[board_x][board_y].is_up = 1;
				break;
			case 3:		//Acabou o jogo (todas as cartas tao up)
				done = 1;
			case 2:		//Jogar 2x e acertar na combinação
				paintCard(_resp.play1[0], _resp.play1[1], 107, 200, 100);
				writeCard(_resp.play1[0], _resp.play1[1], _resp.str_play1, 0, 0, 0);
				paintCard(_resp.play2[0], _resp.play2[1], 107, 200, 100);
				writeCard(_resp.play2[0], _resp.play2[1], _resp.str_play2, 0, 0, 0);
				board[board_x][board_y].is_up = 1;
				break;
			case -2:	//Jogar 2x e falhar na combinação
				paintCard(_resp.play1[0], _resp.play1[1], 107, 200, 100);
				writeCard(_resp.play1[0], _resp.play1[1], _resp.str_play1, 255, 0, 0);
				paintCard(_resp.play2[0], _resp.play2[1], 107, 200, 100);
				writeCard(_resp.play2[0], _resp.play2[1], _resp.str_play2, 255, 0, 0);
				board[board_x][board_y].is_up = 1;
				sleep(2);
				paintCard(_resp.play1[0], _resp.play1[1], 255, 255, 255);
				paintCard(_resp.play2[0], _resp.play2[1], 255, 255, 255);
				board[_resp.play1[0]][_resp.play1[1]].is_up = 0;
				board[board_x][board_y].is_up = 0;
				break;
			case -1:
				paintCard(_resp.play1[0], _resp.play1[1], 255, 255, 255);
				board[_resp.play1[0]][_resp.play1[1]].is_up = 0;
				break;
		}
	}
	printf("fim\n");

	//Free no board
	/*for(int i = 0; i < dim; i++){
		free(board[i]);
	}
	free(board);*/
//}