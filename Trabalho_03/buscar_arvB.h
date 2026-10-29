#ifndef BUSCAR_ARV
    #define BUSCAR_ARV

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"
    #include <stdbool.h>

    void buscar_arvore();
    void buscar_registro_arvB(int chave, char **campos, char **buscados, int criterios, int op, CABECALHO *c, FILE *bin, FILE *arvB);
   
#endif 