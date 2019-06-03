/******************************************************************************
* 						2018/2019 - Programação de Sistemas
*
* Elementos do Grupo: Francisco Melo Nº 86998
*					  Inês Moreira Nº 88050
*
* SECÇÃO: CLIENTE
* FICHEIRO: utils.c
*
* Funcionalidades: 
*	- Funções auxiliares: verificação de erros de alocação de memória e 
* inicialização de elementos de SDL;
*****************************************************************************/

#include "utils.h"

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
		exit(0);
	}
	
}

/******************************************************************************
* void initSDL()
* Argumentos: null
*
* Retorno:  void
*
* Descrição: Função que inicializa elementos do SDL e verifica se estes foram
* inicializados com sucesso
*****************************************************************************/
void initSDL(){
	if(SDL_Init( SDL_INIT_VIDEO) < 0){
		 printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		 exit(-1);
	}
	if(TTF_Init() == -1){
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}
}
