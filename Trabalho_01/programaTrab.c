// Nome: Gabriella Almeida            N° USP: 15528121
// Nome: Nicolas Amaral dos Santos	  N° USP: 16304033

#include <stdio.h>
#include "funcionalidade_1.h"
#include "funcionalidade_2.h"
#include "funcionalidade_3.h"

int main() {
    int funcionalidade;
    scanf("%d", &funcionalidade);

    switch (funcionalidade) {
    case 1:
        leitura_csv();
        break;
    case 2:
        recupera_dados();
        break;
    case 3:
        busca();
        break;
    default:
        break;
    }

    return 0;
}