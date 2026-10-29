#include "buscar_registros.h"
#include "funcoes_auxiliares.h"
#include "funcao-fornecida.h"
#include <string.h>

void buscar_registros() {
    char arquivo[20];
    int quant_buscas;
    
    scanf("%s %d", arquivo,  &quant_buscas);

    FILE *bin = fopen(arquivo, "rb");
    if(bin == NULL) {
        print_erro();
        return;
    }

    CABECALHO *c = (CABECALHO *) malloc(sizeof(CABECALHO));
    if(c == NULL) {
        return;
    }

    descricoes_dos_campos(c, bin);

    busca(bin, quant_buscas, c, 0, NULL); // Implementação em funções auxiliares
    // A função de busca já exibe os registros que atenderem aos critérios

    // Fecha arquivo
    fclose(bin);
    
    // Desalocação de memória da struct cabeçalho
    free(c);
}

