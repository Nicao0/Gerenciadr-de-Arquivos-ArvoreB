#include "criar_arvB.h"
#include "funcoes_arvB.h"
#include "funcoes_auxiliares.h"
#include "funcao-fornecida.h"
#include "inserir_arvB.h"

// Tipo do nó
#define FOLHA -1
#define RAIZ 0
#define INTERMED 1

#define PTR_NULO -1 // Ponteiro (RRN) para outro nó
#define CAPACIDADE 2 // 2 chaves por nó
#define CABECALHO 1 // fseek considerando cabeçalho

/*  A função ler_registros_bin percorre o arquivo de dados, registro a registro, obtendo a chave (idAttack)
    e o offset do registro, para inseri-los na árvore. Essa função, trata o caso em que o primeiro nó deve ser
    criado (fora do algoritmo da inserção), bem como, o caso em que um promoção é propagada até a raiz da árvore e,
    portanto, a raiz deve mudar.
    A função escreve na árvore nesses dois casos acima, nos demais, a função inserir_arvore se encarrega.
*/
void ler_registros_bin(REGISTRO *r, CABECALHO_ARVORE *c_arv, FILE *bin, FILE *arvB) {
    long int offset; // Armazena o offset
    int primeiro = 0;

    while(fread(&(r->removido), sizeof(char), 1, bin) == 1) {
        offset = ftell(bin) - 1; // Desconsidera o removido já lido, para ter o offset correto (1° byte do registro)
        fread(&(r->tamRegistro), 4, 1, bin);

        if(r->removido == '0') { // Se não removido
            primeiro++;
            fseek(bin, 8, SEEK_CUR); // Pula campo próx
            fread(&(r->idAttack), 4, 1, bin); 

            if(primeiro == 1) { // Caso raiz (criada fora do algoritmo de inserção)
                NO *raiz = criar_no(r->idAttack, offset, c_arv); // Cria o 1° nó (raiz)
                // fseek desnecessário pois como já escrevemos o cabeçalho no arq da arvB, já estamos no lugar certo
                escrever_na_arvB(raiz, arvB);

            } else { // Se não for o primeiro inserido na árvore
                PROMOCAO *promov = inserir_arvore(r->idAttack, offset, c_arv->noRaiz, c_arv, arvB); 
                // A função inserir_arvore já se encarrega de escrever na árvore B, no caso que não há promoção até a raiz

                // Se a promoção foi propagada até esse nível significa que essa promoção deve se tornar raiz e, devemos escrever essa raiz na arvB
                if(promov != NULL) {
                    NO *nova_raiz = criar_no(promov->C, promov->PR, c_arv);
                    int RRN_nova_raiz = (c_arv->proxRRN) -1; // -1 porque foi incrementado na criação do nó
                    nova_raiz->P1 = promov->noEsq; 
                    nova_raiz->P2 = promov->noDir; // Novo nó criado por split
                    nova_raiz->tipoNo = RAIZ;
                    
                    // Acessa raiz antiga
                    int RRN_raiz_antiga = c_arv->noRaiz; // Ainda não atualizado, portanto = raiz antiga
                    fseek(arvB, (RRN_raiz_antiga + CABECALHO) * 44, SEEK_SET);
                    
                    NO *raiz_antiga = (NO *) malloc(sizeof(NO)); // Para armazenar raiz antiga
                    if(raiz_antiga == NULL) {
                        return;
                    }

                    ler_da_arvB(raiz_antiga, arvB); // Lê raiz antiga
            
                    if(raiz_antiga->P1 == PTR_NULO) { // É folha
                        raiz_antiga->tipoNo = FOLHA; // Deixa de ser raiz e vira folha

                    } else { // Não é folha
                        raiz_antiga->tipoNo = INTERMED; // Deixa de ser raiz e vira intermediário
                    }

                    fseek(arvB, (RRN_raiz_antiga + CABECALHO) * 44, SEEK_SET); 
                    escrever_na_arvB(raiz_antiga, arvB); // Escreve novamente mas agora atualizado corretamente

                    fseek(arvB, (RRN_nova_raiz + CABECALHO) * 44, SEEK_SET); 
                    escrever_na_arvB(nova_raiz, arvB); // Escreve nova raiz na arvB

                    // Atualiza raiz no cabeçalho (o cabeçalho é escrito somente no fim do algoritmo, para evitar fseeks desnecessários)
                    c_arv->noRaiz = RRN_nova_raiz;
                    
                    // Desalocações
                    free(raiz_antiga);  
                    raiz_antiga = NULL;
                    
                    free(nova_raiz); 
                    nova_raiz = NULL;

                    free(promov); 
                    promov = NULL;
                }
            
            }

            fseek(bin, ((r->tamRegistro)-8 -4), SEEK_CUR); // -8 do prox - 4 do idAttack
        
        } else { // Se removido (não insere)
            fseek(bin, ((r->tamRegistro)), SEEK_CUR); // Avança para o fim do registro removido
            
        }
    }  
}

/*  A função criar_indice abre o arquivo de dados, somente para leitura e, a partir dele, cria 
    o índice árvore B (no modo criação que permite leitura e escrita também). Preenche o cabeçalho  
    do arquivo da árvore e chama a função ler_registros_bin que percorrerá o arquivo de dados para 
    inserir as chaves (idAttack) na árvore B.
    Por fim, altera o status do cabeçalho e fecha o arquivo.
*/
void criar_indice() {
    char arquivo_dados[20];
    char arquivo_arvB[20];
    
    scanf("%s %s", arquivo_dados,  arquivo_arvB);

    FILE *bin = fopen(arquivo_dados, "rb"); // Leitura
    if(bin == NULL) {
        print_erro();
        return;
    }

    FILE *arvB = fopen(arquivo_arvB, "wb+"); // Cria para leitura e escrita
    if(arvB == NULL) {
        print_erro();
        return;
    }

    REGISTRO *r = (REGISTRO *) malloc(sizeof(REGISTRO));
    if(r == NULL) {
        return;
    }

    CABECALHO_ARVORE *c_arv = (CABECALHO_ARVORE *) malloc(sizeof(CABECALHO_ARVORE));
    if(c_arv == NULL) {
        return;
    }

    c_arv->status = '0'; // Inconsistente
    c_arv->noRaiz = -1; // Árvore vazia
    c_arv->proxRRN = 0; // Inicialmente 0
    c_arv->nroNos = 0; // Nenhum nó
    atualizar_cabecalho_arv(c_arv, arvB); // Escreve a atualização do cabeçalho
    
    char cifrao = '$';
    for (int i = 0; i < 31; i++) { // Escreve os cifrões
        fwrite(&cifrao, 1, 1, arvB); 
    }
    
    fseek(bin, 276, SEEK_SET); // Pula cabeçalho no arquivo de dados
    ler_registros_bin(r, c_arv, bin, arvB);

    // Atualizar cabeçalho
    c_arv->status = '1';
    fseek(arvB, 0, SEEK_SET); // Volta ao início do arquivo da arvB
    atualizar_cabecalho_arv(c_arv, arvB);

    // visualizar_arvore(c_arv, arvB);

    fclose(bin);
    fclose(arvB);
    
    binarioNaTela(arquivo_arvB);

    // Desalocações
    free(c_arv);
    c_arv = NULL;
    free(r);
    r = NULL;
}