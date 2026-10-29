#ifndef ALTERAR_REGIS
    #define ALTERAR_REGIS

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"

    typedef struct cabecalho CABECALHO;
    typedef struct registro REGISTRO;

    void atualizar(REGISTRO *r, CABECALHO *c, FILE *bin);
    void atualizar_registros();
    void entrada_com_alteracoes(REGISTRO *r, char entrada[200], int quant_alteracoes);


#endif