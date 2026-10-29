#ifndef FUNCOES_ARVB
    #define FUNCOES_ARVB

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include "structs.h"

    void ler_cabecalho(int status, CABECALHO_ARVORE *c_arv, FILE *arvB);
    void atualizar_cabecalho_arv(CABECALHO_ARVORE *c_arv, FILE *arvB);
    bool chaves_iguais(int chave, int chave_no_atual);
    void ler_da_arvB(NO *no, FILE *arvB);
    void visualizar_arvore(CABECALHO_ARVORE *c_arv, FILE *arvB);
    void escrever_na_arvB(NO *no, FILE * arvB);
    NO* criar_no(int chave, long int offset, CABECALHO_ARVORE *c_arv);
    long int buscar(int chave, int RRN_atual, long int novo_offset, FILE *arvB);
    
    void teste();
    
#endif 