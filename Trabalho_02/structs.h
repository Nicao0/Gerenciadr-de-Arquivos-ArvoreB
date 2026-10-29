#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

    typedef struct cabecalho CABECALHO;
    typedef struct registro REGISTRO;
    
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

#endif