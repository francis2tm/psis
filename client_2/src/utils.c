#include "utils.h"

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
