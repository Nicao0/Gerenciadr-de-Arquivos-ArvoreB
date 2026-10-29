#ifndef INSERIR_NOVO
    #define INSERIR_NOVO

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"

    PROMOCAO* inserir_arvore(int chave, long int offset, int RRN_atual, CABECALHO_ARVORE *c_arv, FILE *arvB);
    void inserir_novo_arv();

#endif 