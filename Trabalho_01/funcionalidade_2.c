#include "funcionalidade_2.h"
#include "structs.h"

int ler_campos_variaveis(char **ptr_registro, char keyword, FILE *arq_bin) {
    char c;
    int tamanho = 0;

    *ptr_registro = NULL; // Começa com ponteiro nulo

    if (fread(&c, 1, 1, arq_bin) != 1) {
        return 0; // Erro ao ler o arquivo
    }

    if(c != keyword) { // Se a keyword desejada não for encontrada significa que o campo está vazio (não existe)
        ungetc(c, arq_bin); // Antes de retornar volta o caracter lido (que não é a keyword)
        return tamanho; // Tamanho = 0 (campo vazio)
    }

    while ((c = fgetc(arq_bin)) != '|' && c != '\n' && c != EOF) { // Se a keyword corresponder, lê o campo caracter a caracter
        char *temp = realloc(*ptr_registro, tamanho + 2);
        if (temp == NULL) {
            free(*ptr_registro); // Evita vazamento de memória
            *ptr_registro = NULL;
            return 0;
        }

        *ptr_registro = temp; // Ponteiro original recebe o temporário
        (*ptr_registro)[tamanho] = c;
        tamanho++;
    }  

    if (*ptr_registro == NULL) {
        *ptr_registro = calloc(1, 1); // Aloca e já inicializa com \0
    } else {
        // Garante terminação mesmo se o campo não terminou com |
        (*ptr_registro)[tamanho] = '\0';
    }

    return tamanho;
}

void print_fixos(char *descricao, void *conteudo, int tipo) {
    if(tipo == 0) { // int = 0
        int *ptr_int = (int *) conteudo;

        if(*ptr_int != -1) // Se não for nulo
            printf("%s: %d\n", descricao, *ptr_int);

        else    
            printf("%s: NADA CONSTA\n", descricao);

    } else { // float = 1
        float *ptr_float = (float *) conteudo;
        
        if(*ptr_float != -1.0) // Se não for nulo
            printf("%s: %.2f\n", descricao, *ptr_float);

        else    
           printf("%s: NADA CONSTA\n", descricao);
    }
}

void print_variaveis(char *descricao, char *conteudo, int tamanho) {
    if(tamanho != 0 && conteudo != NULL) { // Se não for nulo
        printf("%s: %s\n", descricao, conteudo);

    } else {
        printf("%s: NADA CONSTA\n", descricao);
    }

}

void recupera_dados() {
    char arquivo[20];
    scanf("%s", arquivo);

    FILE *p_bin = fopen(arquivo, "rb");
    if(p_bin == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    CABECALHO *c = (CABECALHO *) malloc(sizeof(CABECALHO));
    if(c == NULL) {
        return;
    }

    REGISTRO *r = (REGISTRO *) malloc(sizeof(REGISTRO));
    if(r == NULL) {
        return;
    }

    // Armazena na struct as descrições dos campos
    fseek(p_bin, 25, SEEK_SET);

    fread(c->descreveIdent, 23, 1, p_bin);
    fread(c->descreveYear, 27, 1, p_bin);
    fread(c->descreveFinancialLoss, 28, 1, p_bin);
    fseek(p_bin, 1, SEEK_CUR); // Ignora keyword
    fread(c->descreveCountry, 26, 1, p_bin);
    fseek(p_bin, 1, SEEK_CUR); // Ignora keyword
    fread(c->descreveType, 38, 1, p_bin);
    fseek(p_bin, 1, SEEK_CUR);// Ignora keyword
    fread(c->descreveTargetIndustry, 38, 1, p_bin);
    fseek(p_bin, 1, SEEK_CUR); // Ignora keyword
    fread(c->descreveDefense, 67, 1, p_bin);
    
    fseek(p_bin, 276, SEEK_SET); // Pula cabeçalho

    while(fread(&(r->removido), sizeof(char), 1, p_bin) == 1){ // Obtém os registros no arquivo binário
        if(r->removido == '0') { // Se não removido
            // Campos fixos
            fread(&(r->tamRegistro), sizeof(int), 1, p_bin);
            fseek(p_bin, 8, SEEK_CUR); // Pula o campo prox
            fread(&(r->idAttack), sizeof(int), 1, p_bin); 
            fread(&(r->year), sizeof(int), 1, p_bin);
            fread(&(r->financialLoss), sizeof(float), 1, p_bin);
        
            // Campos variáveis
            int tamanho_country = ler_campos_variaveis(&(r->country), '1', p_bin); 
            int tamanho_attack_type= ler_campos_variaveis(&(r->attackType), '2', p_bin); 
            int tamanho_target_industry = ler_campos_variaveis((&r->targetIndustry), '3', p_bin); 
            int tamanho_defense_mechanism = ler_campos_variaveis(&(r->defenseMechanism), '4',p_bin);

            print_fixos(c->descreveIdent, &(r->idAttack), 0);
            print_fixos(c->descreveYear, &(r->year), 0);
            print_variaveis(c->descreveCountry, r->country, tamanho_country);
            print_variaveis(c->descreveTargetIndustry, r->targetIndustry, tamanho_target_industry);
            print_variaveis(c->descreveType, r->attackType, tamanho_attack_type);
            print_fixos(c->descreveFinancialLoss, &(r->financialLoss), 1);
            print_variaveis(c->descreveDefense, r->defenseMechanism, tamanho_defense_mechanism);
            printf("\n"); // Linha em branco
        }
    }

    // Fecha arquivo
    fclose(p_bin);

    // Desalocação de memória dos campos variáveis e das structs
    free(r->country);
    free(r->attackType);
    free(r->targetIndustry);
    free(r->defenseMechanism);
    free(c);
    free(r);
}