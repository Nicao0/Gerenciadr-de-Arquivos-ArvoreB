#include "atualizar_arvB.h"
#include "atualizar_registros.h"
#include "funcoes_auxiliares.h"
#include "funcoes_arvB.h"                              

/*  A função atualizar_offset_arvB obtém as informações do cabeçalho da árvore e, a partir da busca,
    faz a atualização do offset de uma chave que está na árvore.
*/
void atualizar_offset_arvB(int chave, long int offset, FILE *arvB) {
    CABECALHO_ARVORE *c_arv = (CABECALHO_ARVORE *) malloc(sizeof(CABECALHO_ARVORE));
    if(c_arv == NULL) {
        return;
    }

    fseek(arvB, 0, SEEK_SET); // Volta ao começo do arquivo da árvore b
    ler_cabecalho(0, c_arv, arvB); // Primeiro parâmetro = 0, pois queremos ler desde o status
    buscar(chave, c_arv->noRaiz, offset, arvB); // Busca já trata a atualização
}

/*  A função atualizar_arvore usa a função atualizar_registros (criada inicialmenta para a funcionalidade 6),
    a fim de reaproveitar o código, já que a atualização no arquivo de dados permanece a mesma.
*/
void atualizar_arvore() {
    atualizar_registros(1);
}