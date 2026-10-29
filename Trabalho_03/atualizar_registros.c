#include "atualizar_registros.h"
#include "funcoes_auxiliares.h"
#include "inserir_registros.h"
#include "remover_registros.h"
#include "funcao-fornecida.h"
#include <string.h>

int sobrescrito = 0; // Sobrescrito em relação a arquivos removidos (inserir no lugar de um removido)
int removido = 0;
int inserido = 0;

#define NULO_INT   -1
#define NULO_FLOAT -1.0

/* A função entrada_com_alteracoes recebe a struct já preenchida pela função busca, com os dados do registro
   que estão no arquivo binário, a partir da entrada do usuário (obtida apenas uma vez na função busca, quando encontrado == 1)
   temos os campos a serem alterados e os armazenamos na struct, assim teremos a struct com todos os valores antigos, exceto por aqueles 
   que o usuário quer alterar.
*/
void entrada_com_alteracoes(REGISTRO *r, char entrada[200], int quant_alteracoes) {
    char *ptr = entrada;
    
    for(int i= 0; i < quant_alteracoes; i++) {
        char *campo = ler_campo(&ptr);
        char *valor = ler_campo(&ptr);

        if (strcmp(campo, "idAttack") == 0) { // idAttack nunca nulo
            r->idAttack = atoi(valor);
            
        } else if (strcmp(campo, "year") == 0) {
            if(valor == NULL) {
                r->year = NULO_INT;
            } else {
                r->year = atoi(valor);
            }
            
        } else if (strcmp(campo, "financialLoss") == 0) {
            if(valor == NULL) {
                r->financialLoss = NULO_FLOAT;
            } else {
                r->financialLoss = atof(valor);
            }

        } else if (strcmp(campo, "country") == 0) {
            r->country = valor; // Se nulo o ponteiro será NULL e inserir_registro já trata

        } else if (strcmp(campo, "attackType") == 0) {
            r->attackType = valor;

        } else if (strcmp(campo, "targetIndustry") == 0) {
            r->targetIndustry = valor;

        } else if (strcmp(campo, "defenseMechanism") == 0) {
            r->defenseMechanism = valor;

        } 
        
    }

}

/* A cada busca bem sucedida, a função atualizar é chamada (pela função busca), os valores: int sobrescrito, int removido e
   int inserido são inicialmente iguais a 0, visto que serão incrementados de acordo com o caso de atualização.
   A função entrada_com_alteracoes (chamada antes na funçao busca) altera a struct para conter o registro atualizado, o novo tamanho é calculado e se verifica
   o caso de atualização, em que pode haver espaço suficiente (então o registro só é escrito) ou que não há espaço e deve-se
   remove-lo (removido += 1) e inseri-lo novamente, o que resulta em duas outras possibilidades: inserir no fim (inserido += 1) ou reusar
   o espaço de algum registro removido (sobrescrito += 1).
*/
void atualizar(REGISTRO *r, CABECALHO *c, FILE *bin, FILE *arvB) {
    int tam_country = r->country ? strlen(r->country) + 2 : 0; // +2 por causa da keyword e '|'
    int tam_attackType = r->attackType ? strlen(r->attackType) + 2 : 0;
    int tam_targetIndustry = r->targetIndustry ? strlen(r->targetIndustry) + 2 : 0;
    int tam_defenseMechanism = r->defenseMechanism ? strlen(r->defenseMechanism) + 2 : 0;

    int tam_novo = 20 + tam_country + tam_attackType + tam_targetIndustry + tam_defenseMechanism; // 20 (campos fixos sem tamregistro e removido)
    
    int sub_tam = r->tamRegistro - tam_novo;

    if(sub_tam >= 0) { // Tem espaço
    // Aqui não altera a árvore, o offset permanece o mesmo
        char cifrao = '$';

        fseek(bin, -((r->tamRegistro - 8)), SEEK_CUR); // Volta para escrever os campos a partir do idAttack (-8 campo prox)   
        inserir_registro(r, bin);

        for (int i = 0; i < sub_tam; i++) {
            fwrite(&cifrao, 1, 1, bin); // Escreve os cifrões, se não precisar sub_tam será 0, então não vai escrever nada
        } 
        
    } else {
    // Aqui precisamos atualizar o novo offset na árvore, em caso de atualização com árvore b, a função verificar_criterios_insercao tratará isso
        long int pos = ftell(bin); // Fim do registro
 
        // Remove e insere novamente
        removido += 1;
        inserido += 1;

        remover(bin, r, c); // Já volta para o início do registro
        
        fseek(bin, 1, SEEK_SET); // Atualiza topo (a função remover não atualiza, o topo é atualizado no fim de todas as remoções, quando está na funcionalidade de remoção), por isso atualizamos aqui
        fwrite(&(c->topo), 8, 1, bin); 
        
        sobrescrito += verificar_criterios_insercao(r, c, NULL, bin, arvB);

        fseek(bin, pos, SEEK_SET); // Para não prejudicar a próxima busca
    }

}

/*  atualizar_registros altera o status do arquivo para inconsistente e, a partir de uma busca, atualiza os registros.
    Para a função busca é passado um indicador de que se quer atualizar um registro (op = 2) e então, na função de busca,
    quando um registro atender aos critérios de busca, a função atualizar (acima) será chamada.
    A função de busca está no arquivo de funções auxiliares.
    Por fim, atualizamos o cabeçalho com as quantidades de inserções, remoções e sobrescritas de registros antes removidos (reúso de espaço).

    A função foi modificada para tratar também a atualização na árvore B, o parâmetro arvore (= 1 se existe árvore) auxilia nisso.
*/
void atualizar_registros(int arvore) {
    char arquivo[20];
    char arquivo_arv[20];
    int quant_buscas;
    char arv_status;
    FILE *arvB;
    
    if(arvore == 1) {
        scanf("%s %s %d", arquivo, arquivo_arv,  &quant_buscas);

        arvB = fopen(arquivo_arv, "rb+"); // Abre para escrita e leitura
        if(arvB == NULL) {
            print_erro();
            return;
        }

        arv_status = '0'; 
        fwrite(&(arv_status), 1, 1, arvB); // Altera status da árvore para inconsistente

    } else {
        scanf("%s %d", arquivo,  &quant_buscas);
    }
    
    FILE *bin = fopen(arquivo, "rb+"); // Abre para escrita e leitura
    if(bin == NULL) {
        print_erro();
        return;
    }

    CABECALHO *c = (CABECALHO *) malloc(sizeof(CABECALHO));
    if(c == NULL) {
        return;
    }

    c->status = '0'; // Inconsistente
    fwrite(&(c->status), 1, 1, bin); // Altera status no arquivo de dados
    fread(&(c->topo), 8, 1, bin); // Obtém topo da lista, caso precise (no caso de remover e inserir de novo)

    if(arvore == 1) {
        busca(bin, quant_buscas, c, 2, arvB); // A função busca chama a função atualizar (acima)
    } else {
        busca(bin, quant_buscas, c, 2, NULL); // A função busca chama a função atualizar (acima)
    }
    
    fseek(bin, 0, SEEK_END); // Para atualizar corretamente o proxByteOffset do cabeçalho (que é sempre o fim do arquivo)
    atualiza_cabecalho(sobrescrito, removido, inserido, ftell(bin), c, bin); // Já volta para consistente (1)

    fclose(bin); // Fecha arquivo de dados
    binarioNaTela(arquivo);

    if(arvore == 1) {
        arv_status = '1'; // Consistente
        fseek(arvB, 0, SEEK_SET); // Volta ao início do arquivo da arvB
        fwrite(&(arv_status), 1, 1, arvB); // Altera status no arquivo da arvB

        fclose(arvB); // Fecha arquivo da arvB
        binarioNaTela(arquivo_arv);
    }

    free(c); // Desaloca struct
    c = NULL;
}