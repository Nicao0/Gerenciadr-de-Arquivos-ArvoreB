#ifndef AUX
    #define AUX

    #include <stdio.h>
    #include <stdlib.h>
    #include "structs.h"

    void print_erro();
    void descricoes_dos_campos(CABECALHO *c, FILE *bin) ;
    int ler_campos_variaveis(char **ptr_registro, char keyword, FILE *arq_bin);
    void print_fixos(char *descricao, void *conteudo, int tipo);
    void print_variaveis(char *descricao, char *conteudo, int tamanho);
    void exibir(CABECALHO *c, REGISTRO *r, int tam_country, int tam_attack_type, int tam_target_industry, int tam_defense_mechanism);
    int busca(FILE *bin, int quant_buscas, CABECALHO *c, int op);
    int escrever_campos_variaveis(char *ptr_registro, char keyword, int tam_insercao, FILE *csv, FILE *bin);
    void atualiza_cabecalho(int sobrescritos, int removidos, int inseridos, long int proxOffset, CABECALHO *c, FILE *bin);
    char* ler_campo(char **ptr);
    
#endif 