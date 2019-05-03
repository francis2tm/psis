#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include <stdio.h>

/*****************************************************************************************************
 * verifyErr ()
 *  Arguments: p: ponteiro genérico
 *            type: modo de verificação: 'm' -> alocação memória
 *  Returns: void
 *  Description: Verifica se um ponteiro aponta para null ou não. Função utilizada para verificar o sucesso
 * de mallocs ou de abertura de ficheiros
 ****************************************************************************************************/
void verifyErr(void *p, char type);

#endif