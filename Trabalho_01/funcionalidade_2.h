#ifndef FUNC2
    #define FUNC2

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"

    void recupera_dados();
    int ler_campos_variaveis(char **ptr_registro, char keyword, FILE *arq_bin);
    void print_fixos(char *descricao, void *conteudo, int tipo);
    void print_variaveis(char *descricao, char *conteudo, int tamanho);

#endif 