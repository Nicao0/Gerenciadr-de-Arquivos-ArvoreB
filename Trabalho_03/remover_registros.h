#ifndef REMOVER_REGIS
    #define REMOVER_REGIS

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"

    typedef struct cabecalho CABECALHO;
    typedef struct registro REGISTRO;

    void remover(FILE *fp, REGISTRO *r, CABECALHO *c);
    void remover_registros();

#endif 