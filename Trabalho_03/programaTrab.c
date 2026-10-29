// Nome: Gabriella Almeida            N° USP: 15528121
// Nome: Nicolas Amaral dos Santos	  N° USP: 16304033

#include <stdio.h>
#include "leitura_csv.h"
#include "recuperar_dados.h"
#include "buscar_registros.h"
#include "remover_registros.h"
#include "inserir_registros.h"
#include "atualizar_registros.h"
#include "criar_arvB.h"
#include "buscar_arvB.h"
#include "inserir_arvB.h"
#include "funcoes_arvB.h"
#include "remover_arvB.h"
#include "atualizar_arvB.h"

int main() {
    int funcionalidade;
    scanf("%d", &funcionalidade);

    switch (funcionalidade) {
    case 1:
        leitura_csv();
        break;
    case 2:
        recuperar_dados();
        break;
    case 3:
        buscar_registros();
        break;
    case 4:
        remover_registros();
        break;
    case 5:
        inserir_registros();
        break;
    case 6:
        atualizar_registros(0);
        break;
    case 7:
        criar_indice();
        break;
    case 8:
        buscar_arvore();
        break;
    // case 9:
    //     remover_arvB();
    //     break;
    case 10:
        inserir_novo_arv();
        break;
    case 11:
        atualizar_arvore();
        break;
    // case 0: // Teste (exibe a árvore a partir de um arquivo de entrada)
    //     teste();
    //     break;
    default:
        break;
    }

    return 0;
}