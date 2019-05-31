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