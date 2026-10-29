#ifndef ALTERAR_REGIS
    #define ALTERAR_REGIS

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"

    void atualizar(REGISTRO *r, CABECALHO *c, FILE *bin, FILE *arvB);   
    void atualizar_registros(int arvore);
    void entrada_com_alteracoes(REGISTRO *r, char entrada[200], int quant_alteracoes);

#endif