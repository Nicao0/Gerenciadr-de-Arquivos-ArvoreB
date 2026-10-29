// Nome: Gabriella Almeida            N° USP: 15528121
// Nome: Nicolas Amaral dos Santos	  N° USP: 16304033

#include <stdio.h>
#include "leitura_csv.h"
#include "recuperar_dados.h"
#include "buscar_registros.h"
#include "remover_registros.h"
#include "inserir_registros.h"
#include "atualizar_registros.h"

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
        atualizar_registros();
        break;
    default:
        break;
    }

    return 0;
}