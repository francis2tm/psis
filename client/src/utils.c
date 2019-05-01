#include "utlis.h"

void verifyErr(void *p, char type){
  if(p == NULL){
    if(type == 'm'){
      fprintf(stderr, "Erro a alocar memoria");
    }else{
      fprintf(stderr, "Erro ao abrir o ficheiro");
    }
      exit(0);
  }
}

