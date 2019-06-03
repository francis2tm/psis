/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: SERVIDOR
* FICHEIRO: board_library.c
*
* Funcionalidades: 
*	- Preencher o board inicialmente com strings;
*   - Verificar se uma dada coordenada recebida é válida;
*   - Processamento de jogadas, atualizando o resp que vai ser enviado ao 
* cliente;
*   - Guardar informação do "dono" de uma dada carta virada.
*****************************************************************************/

#include "board_library.h"
#include "utils.h"
#include "player_manage.h"
#include "server.h"

extern int dim;
extern Board_Place** board;
extern pthread_rwlock_t rwlock_score;
extern pthread_rwlock_t rwlock_stack;
extern Score_List score;
extern int n_players;

/******************************************************************************
* void initBoard(char flag)
*
* Argumentos: char flag: flag que verifica se é preciso alocar ou não de novo 
* o board, isto porque quando o jogo for reiniciado não será necessário alocar
* o tabuleiro pois este fora alocado no jogo que acabou de terminar;
* 
* Retorno: void;
*
* Descrição: Função que aloca memória para o board e inicializa o tabuleiro
* com strings.
*****************************************************************************/
void initBoard(char flag){
    int count  = 0;
    int i, j;
    char* str_place;
  
    if(!flag){                              //Se for o servidor tiver começado agora, se não, não é preciso alocar 
        //Alocar a matriz
        board = (Board_Place**)malloc(sizeof(Board_Place*)*dim);
        verifyErr(board);
        for(i = 0; i < dim; i++){
            board[i] = NULL;
            board[i] = (Board_Place*)malloc(sizeof(Board_Place)*dim);
            if(board == NULL){
                verifyErr(board[i]);
            }
        }
    }

	//Inicializar a matriz
	for(i = 0; i < (dim); i++){
		for(j = 0; j < dim; j++){
			board[i][j].str[0] = '\0';
			board[i][j].is_up = 0;
	 	}
	}

    for (char c1 = 'a' ; c1 < ('a'+dim); c1++){
        for (char c2 = 'a' ; c2 < ('a'+dim); c2++){
            do{
                i = rand() % dim;
                j = rand() % dim;
                str_place = board[i][j].str;
            }while(str_place[0] != '\0');
            str_place[0] = c1;
            str_place[1] = c2;
            str_place[2] = '\0';
            do{
                i = rand() % dim;
                j = rand() % dim;
                str_place = board[i][j].str;
            }while(str_place[0] != '\0');
            str_place[0] = c1;
            str_place[1] = c2;
            str_place[2] = '\0';
            count += 2;
            if (count == dim*dim){
                return;
            }
        }
    }
}

/******************************************************************************
* void boardPlay(Play_Response* resp, char* n_play, int* n_corrects, int x, int y)
*
* Argumentos: Play_Response* resp: resp do cliente que selecionou (x,y)
*             char* n_play: se for a primeira jogada (1) se for a segunda (0);
*             int* n_corrects: respostas corretas de um jogador;
*             int x: coordenada x do board;
*             int y: coordenada y do board;

* Retorno: void;
*
* Descrição: Função que recebendo uma dada coordenada a processa atualizando
* a informação de modo a enviá-la posteriormente ao cliente;
*****************************************************************************/
//Processa as jogada de um determinado jogador que enviou ao servidor as coordenadas (x,y)
void boardPlay(Play_Response* resp, char* n_play, int* n_corrects, int x, int y){
    mutex(LOCK, &(board[x][y].mutex_board));
    if(board[x][y].is_up){
        if(resp->code == 1){    //Se for a segunda jogada e for para virar a primeira carta para baixo
            resp->code = -1;
            *n_play = 0;
            fillCard(*resp, 0, resp->play1[0], resp->play1[1]);
        }else{
            printf("FILLED\n");
            resp->code = 0;
        }
    }else{
        if(!*n_play){            //Se for a primeira jogada
            printf("FIRST\n");
            resp->code = 1;
            *n_play = 1;
            resp->play1[0] = x;
            resp->play1[1] = y;
            strcpy(resp->str_play1, board[x][y].str);
            fillCard(*resp, 1, x, y);
        }else{                  //Se for a segunda jogada
            char* first_str = board[resp->play1[0]][resp->play1[1]].str;
            char* secnd_str = board[x][y].str;

            strcpy(resp->str_play1, first_str);
            resp->play2[0] = x;
            resp->play2[1] = y;
            strcpy(resp->str_play2, secnd_str);
            
            if (!strcmp(first_str, secnd_str)){
                printf("CORRECT!!!\n\n");
                *n_corrects += 2;
                rwLock(W_LOCK, &rwlock_score);
                score.top_score += 2;
                if(score.top_score == dim*dim){
                    rwLock(UNLOCK, &rwlock_score);
                    resp->code = 3;
                    printf("ACABOU\n\n");
                }else{
                    rwLock(UNLOCK, &rwlock_score);
                    resp->code = 2;
                }
                fillCard(*resp, 1, x, y);
                fillCard(*resp, 1, resp->play1[0], resp->play1[1]);
            }else{
                printf("INCORRECT\n\n");
                resp->code = -2;
                fillCard(*resp, 1, x, y);
                fillCard(*resp, 1, resp->play1[0], resp->play1[1]);
            }
            *n_play = 0;
        }
    }
    mutex(UNLOCK, &(board[x][y].mutex_board));
}

/******************************************************************************
* void fillCard(Play_Response resp, char value, int x, int y)
*
* Argumentos: Play_Response resp: informação do dono da carta;
*             char value: valor para o is_up (0 ou 1) dependendo se a carta está
* virada ou não;
*             int x: coordenada x do tabuleiro da peça escolhida;
*             int y: coordenada y do tabuleiro da peça escolhida;
*
* Retorno: void;
*
* Descrição: Função que preenche uma carta do tabuleiro com o respetivo dono (isto
* é, cliente que a virou)
*****************************************************************************/
void fillCard(Play_Response resp, char value, int x, int y){
    board[x][y].is_up = value;
    board[x][y].code = resp.code;
    if(value){                      //Só meter preencher cor se for para virar a carta para cima e se for a primeira jogada
        cpy3CharVec(resp.color, board[x][y].owner_color);		//Copiar vetor resp.color para vetor board[x][y].owner_color
    }
}

/******************************************************************************
* int checkPlay(int x, int y)
*
* Argumentos: int x: coordenada x do tabuleiro que foi selecionada
*             int y: coordenada y do tabuleiro que foi selecionada
*
* Retorno: int 0: jogada válida;
*          int 1: jogada inválida;   
*
* Descrição: Função que analisa o número de jogadores e se este for maior que
* 1 e se as coordenadas recebidas estiverem no tabuleiro irá retornar 0 
* (sucesso, que equivale a uma jogada válida).
*****************************************************************************/
int checkPlay(int x, int y){
    //Verificar se é o único jogador
    rwLock(R_LOCK, &rwlock_stack);
    if(n_players == 1){
        rwLock(UNLOCK, &rwlock_stack);
        return 1;
    }
    rwLock(UNLOCK, &rwlock_stack);

    //Verificar se as coordenadas recebidas estão dentro do board
    if(x >= dim || y >= dim || x < 0 || y < 0){                       
        return 1;
    }
    return 0;
}

/******************************************************************************
* void deleteBoard()
*
* Argumentos: null;
*
* Retorno: void;
*
* Descrição: Função que liberta a memória anteriormente alocada para o board
*****************************************************************************/
void deleteBoard(){
    int i;

    for(i = 0; i < dim; i++){
		free(board[i]);
	}
    free(board);
}