#include "funcionalidade_2.h"
#include "funcionalidade_3.h"
#include "funcao-fornecida.h"
#include "structs.h"
#include <string.h>
#include <stdbool.h>

void busca_bem_sucedida(CABECALHO *c, REGISTRO *r, int tam_country, int tam_attack_type, int tam_target_industry, int tam_defense_mechanism) {
    print_fixos(c->descreveIdent, &(r->idAttack), 0);
    print_fixos(c->descreveYear, &(r->year), 0);
    print_variaveis(c->descreveCountry, r->country, tam_country);
    print_variaveis(c->descreveTargetIndustry, r->targetIndustry, tam_target_industry);
    print_variaveis(c->descreveType, r->attackType, tam_attack_type);
    print_fixos(c->descreveFinancialLoss, &(r->financialLoss), 1);
    print_variaveis(c->descreveDefense, r->defenseMechanism, tam_defense_mechanism);
    printf("\n"); // Linha em branco
}

bool verifica_criterio(char *campo, char *buscado_str, REGISTRO *r) {
    int buscado_int;
    float buscado_float;

    if(strcmp(campo, "idAttack") == 0) {
        buscado_int = atoi(buscado_str); // Converte o valor na string para int
        if(r->idAttack == buscado_int) {
            return true;
        }

    } else if(strcmp(campo, "year") == 0) {
        buscado_int = atoi(buscado_str); // Converte o valor na string para int
        if(r->year == buscado_int) {
            return true;
        }

    } else if(strcmp(campo, "financialLoss") == 0) {
        char *end_ptr; 
        buscado_float = strtof(buscado_str, &end_ptr); // Converte o valor na string para float
        if(r->financialLoss == buscado_float) {
            return true;
        }

    // Verifica se o campo é NULL para evitar erros no strcmp
    } else if(strcmp(campo, "country") == 0){
        if(r->country != NULL && strcmp(r->country, buscado_str) == 0) {
            return true;
        }

    } else if(strcmp(campo, "attackType") == 0){
        if(r->attackType != NULL && strcmp(r->attackType, buscado_str) == 0) {
            return true;
        }
        
    } else if(strcmp(campo, "targetIndustry") == 0){
        if(r->targetIndustry != NULL && strcmp(r->targetIndustry, buscado_str) == 0) {
            return true;
        }

    } else if(strcmp(campo, "defenseMechanism") == 0){
        if(r->defenseMechanism != NULL && strcmp(r->defenseMechanism, buscado_str) == 0) {
            return true;
        }
    }

    return false; // Falha na busca retorna false
}

void percorre_registros(char **campos, char **buscados, int criterios, CABECALHO *c, FILE *fp) {
    REGISTRO *r = calloc(1, sizeof(REGISTRO)); // Zera toda a struct
    if (r == NULL) return;

    int encontrado = 0;

    while(fread(&(r->removido), sizeof(char), 1, fp) == 1) {
        r->country = NULL;
        r->attackType = NULL;
        r->targetIndustry = NULL;
        r->defenseMechanism = NULL;
        
        if(r->removido == '0') { // Se não removido
            fread(&(r->tamRegistro), 4, 1, fp);
            fseek(fp, 8, SEEK_CUR); // Pula campo próx

            fread(&(r->idAttack), 4, 1, fp);
            fread(&(r->year), 4, 1, fp);
            fread(&(r->financialLoss), 4, 1, fp);

            int tamanho_country = ler_campos_variaveis(&(r->country), '1', fp); 
            int tamanho_attack_type= ler_campos_variaveis(&(r->attackType), '2', fp); 
            int tamanho_target_industry = ler_campos_variaveis((&r->targetIndustry), '3', fp); 
            int tamanho_defense_mechanism = ler_campos_variaveis(&(r->defenseMechanism), '4',fp);
            

            int atende_criterios = 1;
            for(int i = 0; i < criterios; i++) { 
                if(!verifica_criterio(campos[i], buscados[i], r)) {
                    atende_criterios = 0;
                    break;
                }
            }

            if(atende_criterios == 1) {
                busca_bem_sucedida(c, r, tamanho_country, tamanho_attack_type, tamanho_target_industry, tamanho_defense_mechanism);
                encontrado++;
            }

        }
        // Desalocação de memória dos campos variáveis da struct registro
        free(r->country);
        free(r->attackType);
        free(r->targetIndustry);
        free(r->defenseMechanism);
    }

    if(encontrado == 0) { // Se a busca falhar
        printf("Registro inexistente.\n\n");
    }

    // Desalocação de memória da struct registro
    free(r);
}


void busca() {
    char arquivo[20];
    int quant_buscas;
    
    scanf("%s %d", arquivo,  &quant_buscas);

    FILE *bin = fopen(arquivo, "rb");
    if(bin == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    CABECALHO *c = (CABECALHO *) malloc(sizeof(CABECALHO));
    if(c == NULL) {
        return;
    }

    // Armazena na struct as descrições dos campos com o terminador \0 evitando invasão de memória
    fseek(bin, 25, SEEK_SET);

    fread(c->descreveIdent, 23, 1, bin);
    c->descreveIdent[23] = '\0';

    fread(c->descreveYear, 27, 1, bin);
    c->descreveYear[27] = '\0';

    fread(c->descreveFinancialLoss, 28, 1, bin);
    c->descreveFinancialLoss[28] = '\0';

    fseek(bin, 1, SEEK_CUR); // Ignora keyword

    fread(c->descreveCountry, 26, 1, bin);
    c->descreveCountry[26] = '\0';

    fseek(bin, 1, SEEK_CUR); // Ignora keyword
    
    fread(c->descreveType, 38, 1, bin);
    c->descreveType[38] = '\0';
    
    fseek(bin, 1, SEEK_CUR); // Ignora keyword
    
    fread(c->descreveTargetIndustry, 38, 1, bin);
    c->descreveTargetIndustry[38] = '\0';
    
    fseek(bin, 1, SEEK_CUR); // Ignora keyword
    
    fread(c->descreveDefense, 67, 1, bin);
    c->descreveDefense[67] = '\0'; 

    for (int i = 0; i < quant_buscas; i++) {
        int criterios;
        scanf("%d", &criterios);

        // Vetores de strings (ponteiros para string) com todos os critérios de busca (todos os campos e itens buscados)
        char **campos = (char **) malloc(criterios * sizeof(char *));
        char **buscados = (char **) malloc(criterios * sizeof(char *));

        for (int j = 0; j < criterios; j++) {
            campos[j] = (char *) malloc(30 * sizeof(char));
            buscados[j] = (char *) malloc(30 * sizeof(char));
            scanf("%s", campos[j]);
            if(strcmp(campos[j], "idAttack") == 0) {
                scanf("%s",buscados[j]); 
            } else {
                scan_quote_string(buscados[j]); 
            }
        }

        fseek(bin, 276, SEEK_SET); // Pula cabeçalho
        percorre_registros(campos, buscados, criterios, c, bin);
        printf("**********\n");

        // Desalocação do vetor de strings
        for (int j = 0; j < criterios; j++) {
            free(campos[j]);
            free(buscados[j]);
        }
        free(campos);
        free(buscados);
        
    }

    // Fecha arquivo
    fclose(bin);
    
    // Desalocação de memória da struct cabeçalho
    free(c);
}