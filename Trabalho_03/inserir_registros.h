#ifndef INSERIR_REGIS
    #define INSERIR_REGIS

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"

    typedef struct cabecalho CABECALHO;
    typedef struct registro REGISTRO;

    void inserir_registros();
    void analisar_entrada(char entrada[200], REGISTRO *r);
    void inserir_registro(REGISTRO *r, FILE *bin);
    int verificar_criterios_insercao(REGISTRO *r, CABECALHO *c, long int *offset, FILE *bin, FILE *arvB);

#endif 