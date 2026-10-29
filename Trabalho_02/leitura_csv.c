#include "leitura_csv.h"
#include "funcao-fornecida.h"
#include "structs.h"
#include "funcoes_auxiliares.h"

typedef struct cabecalho CABECALHO;
typedef struct registro REGISTRO;

void ler_csv_escrever_bin_cabecalho(void *ptr_dados, int bytes, int vezes, FILE *arq_entrada, FILE *arq_saida) {
    fread(ptr_dados, bytes, vezes, arq_entrada);
    fwrite(ptr_dados, bytes, vezes, arq_saida);
    
    fseek(arq_entrada, 1, SEEK_CUR); // Pula 1 byte da posição atual do ponteiro do csv (pula a vírgula) 
}

void escrever_char_bin_cabecalho(char *ptr_cabecalho, FILE *arq_entrada, FILE *arq_saida) { 
    fwrite(ptr_cabecalho, 1, 1, arq_saida);
}

void ler_csv_escrever_campos_fixos(void *ptr_dados, int bytes, int vezes, int type, FILE *arq_entrada, FILE *arq_saida) {
    int lidoInt;
    float lidoFloat;
    char c = fgetc(arq_entrada);

    if (c == ',') { // Se encontrar "," significa que o campo está vazio
        if(type == 0) {
            int nulo_int = -1; // -1 caso nulo
            fwrite(&nulo_int,sizeof(int), 1, arq_saida);
            return;

        } else { 
            float nulo_float = -1.0; // -1.0 caso nulo
            fwrite(&nulo_float, sizeof(float), 1, arq_saida);
            return;

        }
    } else {
        ungetc(c, arq_entrada); // Se não for "," volta o cursor 

        if (type == 0) { // 0 indica que vai ler int
            int *ptrInt = (int *) ptr_dados;
            fscanf(arq_entrada, "%d", &lidoInt);
            *ptrInt = lidoInt;
            fwrite(ptrInt, bytes, vezes, arq_saida);
            
        } else { // 1 indica que vai ler float
            float *ptrFloat = (float *) ptr_dados;
            fscanf(arq_entrada, "%f", &lidoFloat);
            *ptrFloat = lidoFloat;
            fwrite(ptrFloat, bytes, vezes, arq_saida);
        }
    }

    fseek(arq_entrada, 1, SEEK_CUR); // Pula 1 byte da posicao atual do ponteiro do arquivo csv (pula a virgula)
}

void leitura_csv() {
    char entrada[20];
    char saida[20];
    
    scanf("%s %s", entrada, saida);

    FILE *p_entrada = fopen(entrada, "r"); 
    if(p_entrada == NULL) {
        print_erro();
        return;
    }

    FILE *p_saida = fopen(saida, "wb");
    if(p_saida == NULL) {
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

    c->status = '0'; // Abriu para escrita logo é 0
    c->topo = -1; // Sem registros removidos, então topo = -1
    c->proxOffset = 0; // Inicialmente = 0
    c->nroRegArq = 0; // Inicialmente = 0
    c->nroRegRem = 0; // Inicialmente = 0
    c->codCountry = '1'; // Keyword
    c->codType = '2'; // Keyword
    c->codTargetIndustry = '3'; // Keyword
    c->codDefense = '4'; // Keyword
    
    fwrite(&(c->status), sizeof(char), 1, p_saida);
    fwrite(&(c->topo), 8, 1, p_saida); // Sizeof long int
    fwrite(&(c->proxOffset), 8, 1, p_saida); // Sizeof long int
    fwrite(&(c->nroRegArq), sizeof(int), 1, p_saida);
    fwrite(&(c->nroRegRem), sizeof(int), 1, p_saida);

    ler_csv_escrever_bin_cabecalho(c->descreveIdent, 23, 1, p_entrada, p_saida);
    ler_csv_escrever_bin_cabecalho(c->descreveYear, 27, 1, p_entrada, p_saida);
    ler_csv_escrever_bin_cabecalho(c->descreveFinancialLoss, 28, 1, p_entrada, p_saida);
    escrever_char_bin_cabecalho(&(c->codCountry), p_entrada, p_saida);
    ler_csv_escrever_bin_cabecalho(c->descreveCountry, 26, 1, p_entrada, p_saida);
    escrever_char_bin_cabecalho(&(c->codType), p_entrada, p_saida);
    ler_csv_escrever_bin_cabecalho(c->descreveType, 38, 1, p_entrada, p_saida);
    escrever_char_bin_cabecalho(&(c->codTargetIndustry), p_entrada, p_saida);
    ler_csv_escrever_bin_cabecalho(c->descreveTargetIndustry, 38, 1, p_entrada, p_saida);
    escrever_char_bin_cabecalho(&(c->codDefense), p_entrada, p_saida);
    ler_csv_escrever_bin_cabecalho(c->descreveDefense, 67, 1, p_entrada, p_saida);

    int regNaoRemovidos = 0;

    while (fscanf(p_entrada, "%d,", &(r->idAttack)) == 1) {

        r->removido = '0';
        r->tamRegistro = 0; 
        r->prox = -1;

        fwrite(&(r->removido), sizeof(char), 1, p_saida);
        fwrite(&(r->tamRegistro), sizeof(int), 1, p_saida);
        fwrite(&(r->prox), 8, 1, p_saida); // Sizeof long int

        fwrite(&(r->idAttack), sizeof(int), 1, p_saida); // Já lido pelo fscanf do while
        ler_csv_escrever_campos_fixos(&(r->year), sizeof(int), 1, 0, p_entrada, p_saida);
        ler_csv_escrever_campos_fixos(&(r->financialLoss), sizeof(float), 1, 1, p_entrada, p_saida);

        int tamTotal = 20; // Tamanho do campos fixos do registro (sem o removido e o tamRegistro)
        tamTotal += escrever_campos_variaveis(r->country, '1', -1, p_entrada, p_saida);
        tamTotal += escrever_campos_variaveis(r->attackType, '2', -1,  p_entrada, p_saida);
        tamTotal += escrever_campos_variaveis(r->targetIndustry, '3', -1,  p_entrada, p_saida);
        tamTotal += escrever_campos_variaveis(r->defenseMechanism, '4', -1,  p_entrada, p_saida);
        r->tamRegistro = tamTotal;

        fseek(p_saida, -(tamTotal + 4), SEEK_CUR); // Volta no campo do tamRegistro para atualizá-lo
        fwrite(&(r->tamRegistro), sizeof(int), 1, p_saida); // Atualiza o tamanho do registro
        fseek(p_saida, 0, SEEK_END); // Volta pro fim do registro
        
        regNaoRemovidos++; // Conta quantos registros foram inseridos
        c->nroRegArq = regNaoRemovidos; 
    }

    long int ultimoByteArq = ftell(p_saida);  // Retorna a posição atual (que será o total de bytes, já que o ponteiro do arquivo está no fim)
    c->proxOffset = ultimoByteArq;
    fseek(p_saida, 9, SEEK_SET); // Volta pro começo e anda +9 bytes (início do campo proxByteOffset)
    fwrite(&(c->proxOffset), 8, 1, p_saida); // Escreve o proxOffset e o cursor já estará no campo nroRegArq
    fwrite(&(c->nroRegArq), sizeof(int), 1, p_saida); // Escreve o nroRegArq 

    c->status = '1'; // Finalizou o uso logo é 1
    fseek(p_saida, 0, SEEK_SET); // Volta pro início do arquivo
    fwrite(&(c->status), sizeof(char), 1, p_saida); // Atualiza status
    
    // Fecha arquivos
    fclose(p_entrada);
    fclose(p_saida);

    // Desalocação de memória dos campos variáveis e das structs
    free(r->country);
    free(r->attackType);
    free(r->targetIndustry);
    free(r->defenseMechanism);
    free(c);
    free(r);
    
    binarioNaTela(saida);
}