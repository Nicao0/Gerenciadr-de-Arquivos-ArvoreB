#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

    #include <stdbool.h>

    typedef struct cabecalho CABECALHO;
    typedef struct registro REGISTRO;
    typedef struct no NO;
    typedef struct cabecalhoArv CABECALHO_ARVORE;
    typedef struct promocao PROMOCAO;
    
    struct cabecalho {
        char status; // 0 (inconsistente quando abre para escrita) ou 1 (consistente quando finaliza o uso)
        long int topo; // Armazena byte offset de um registro lógicamente removido
        long int proxOffset; // Armazena o próx byteoffset disponível
        int nroRegArq; // Número de registros não removidos (inicia com 0)
        int nroRegRem; // Número de registros removidos (inicia com 0)

        /* Os campos abaixo estão com um byte a mais na struct,
        isso se deve a necessidade do \0, porém na hora de escrever 
        no arquivo binário foi MANTIDO o número de bytes especificado*/
        
        char descreveIdent[24]; // 23 + \0
        char descreveYear[28]; // 27 + \0
        char descreveFinancialLoss[29]; // 28 + \0
        char codCountry; // Código da keyword 
        char descreveCountry[27]; // 26 + \0
        char codType; // Código da keyword 
        char descreveType[39]; // 38 + \0
        char codTargetIndustry; // Código da keyword
        char descreveTargetIndustry[39]; // 38 + \0
        char codDefense; // Código da keyword 
        char descreveDefense[68]; // 67 + \0
    };

    struct registro {
        // Fixos -> quando nulos: string = $, int/float = -1
        char removido; // 1 = removido, 0 = não removido 
        int tamRegistro; // Número de bytes do registro 
        long int prox; // Inicia com -1, armazena o byteOffset do próx registro lógicamente removido
        int idAttack;
        int year;
        float financialLoss;
        // Variáveis -> delimitador '|'
        char *country;
        char *attackType;
        char *targetIndustry;
        char *defenseMechanism;
    };

    struct cabecalhoArv {
        char status; // 0 (inconsistente quando abre para escrita) ou 1 (consistente quando finaliza o uso)
        int noRaiz; // RRN do nó (página), se vazia == -1 
        int proxRRN; // Incialmente 0, quando nó folha = nó raiz 1 e depois sempre o prox RRN que poderá ser criado 
        int nroNos; // Nro de páginas na árvore, inicialmente 0
    };

    struct no {
        int tipoNo; // -1 folha, 0 raiz, 1 intermediário
        int nroChaves; // Quantas chaves há no nó
        int P1; // "Ponteiro" para outro nó, -1 se nulo
        int C1; // Chave de busca, -1 se nula
        long int PR1; // "Ponteiro" para acesso direto (offset) no arquivo de dados, -1 nulo
        int P2; // "Ponteiro" para outro nó, -1 se nulo
        int C2; // Chave de busca, -1 se nula
        long int PR2; // "Ponteiro" para acesso direto (offset) no arquivo de dados, -1 nulo
        int P3; // "Ponteiro" para outro nó, -1 se nulo
    };

    struct promocao {
        int C; // Chave promovida
        long int PR; // Offset promovido
        int noDir; // Novo nó direito criado por split
        int noEsq; // Ponteiro usado no caso de promoção até a raiz
        int RRN_anterior;
    };

#endif