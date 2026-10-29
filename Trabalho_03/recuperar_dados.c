#include "recuperar_dados.h"
#include "funcoes_auxiliares.h"
#include "structs.h"

void recuperar_dados() {
    char arquivo[20];
    scanf("%s", arquivo);

    FILE *bin = fopen(arquivo, "rb");
    if(bin == NULL) {
        print_erro();
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

    descricoes_dos_campos(c, bin);
    
    while(fread(&(r->removido), sizeof(char), 1, bin) == 1 ){ // Obtém os registros no arquivo binário
        if(r->removido == '0') { // Se não removido
            // Campos fixos
            fread(&(r->tamRegistro), sizeof(int), 1, bin);
            fseek(bin, 8, SEEK_CUR); // Pula o campo prox
            fread(&(r->idAttack), sizeof(int), 1, bin); 
            fread(&(r->year), sizeof(int), 1, bin);
            fread(&(r->financialLoss), sizeof(float), 1, bin);
        
            // Campos variáveis
            int tamanho_country = ler_campos_variaveis(&(r->country), '1', bin); 
            int tamanho_attack_type= ler_campos_variaveis(&(r->attackType), '2', bin); 
            int tamanho_target_industry = ler_campos_variaveis((&r->targetIndustry), '3', bin); 
            int tamanho_defense_mechanism = ler_campos_variaveis(&(r->defenseMechanism), '4',bin);

            exibir(c, r, tamanho_country, tamanho_attack_type, tamanho_target_industry, tamanho_defense_mechanism); // Implementação em funções auxiliares
        }
    }

    // Fecha arquivo
    fclose(bin);

    // Desalocação de memória dos campos variáveis e das structs
    free(r->country);
    free(r->attackType);
    free(r->targetIndustry);
    free(r->defenseMechanism);
    free(c);
    free(r);
}