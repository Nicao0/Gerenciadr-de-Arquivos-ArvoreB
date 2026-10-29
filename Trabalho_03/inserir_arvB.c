#include "inserir_arvB.h"
#include "funcao-fornecida.h"
#include "inserir_registros.h"
#include "funcoes_auxiliares.h"
#include "funcoes_arvB.h"

// Tipo do nó
#define FOLHA -1
#define RAIZ 0
#define INTERMED 1

#define PTR_NULO -1 // Ponteiro (RRN) para outro nó
#define CAPACIDADE 2 // 2 chaves por nó
#define CABEC 1 // fseek considerando cabeçalho

/*  A função insere_ordenado insere a chave (com seu offset) no nó da árvore, verificando se deve inserir
    como C1 (então o conteúdo antigo da chave C1, deve ser armazenado em C2) ou em C2.
    A função também tem o parâmetro filho_dir, que é necessário no caso de promoção e, incrementa o número
    de chaves no nó.
*/
void insere_ordenado(int chave, long int offset, int filho_dir, NO *no) {
    if(no->C1 > chave) { // Troca
        int aux = no->C1; 
        no->C1 = chave;
        no->C2 = aux;
        no->PR2 = no->PR1; // Mantém offset
        no->PR1 = offset;
        no->P3 = no->P2; // Como C1 trocou para C2 o P3 precisa apontar para quem o P2 apontava
        no->P2 = filho_dir; // Novo nó criado por split (caso de promoção)

    } else { // Só insere em C2
        no->C2 = chave;
        no->PR2 = offset;
        no->P3 = filho_dir;
    }

    (no->nroChaves)++;
}

/*  A função split distribui uniformemente as chaves contidas no nó e a nova chave, promovendo a que melhor as divide. 
    Cria um novo nó (direito), que conterá a maior chave, sendo que a menor, permanecerá no nó que antes estava cheio.
*/
PROMOCAO* split(int nova_chave, long int offset, int RRN_promov_dir, int RRN_no_cheio, NO *no_cheio, CABECALHO_ARVORE *c_arv, FILE *arvB) {
    PROMOCAO *promov = (PROMOCAO *) malloc(sizeof(PROMOCAO));
    if(promov == NULL) {
        return NULL;
    }
    
    // Distribui uniformemente e promove 

    int chaves[3] = { no_cheio->C1, no_cheio->C2, nova_chave };
    long int offsets[3] = { no_cheio->PR1, no_cheio->PR2, offset };
    int filhos[4] = { no_cheio->P1, no_cheio->P2, no_cheio->P3, RRN_promov_dir };

    // Ordena as chaves, mantendo offsets com as chaves corretas e ajustando filhos (ponteiros)
    for(int i = 0; i < 2; i++) {
        for(int j = i+1; j < 3; j++) {
            if(chaves[i] > chaves[j]) {
                int aux_chave = chaves[i];
                chaves[i] = chaves[j];
                chaves[j] = aux_chave;

                long int aux_offset = offsets[i];
                offsets[i] = offsets[j];
                offsets[j] = aux_offset;

                int aux_filho = filhos[i+1];
                filhos[i+1] = filhos[j+1];
                filhos[j+1] = aux_filho;
            }
        }
    }
    
    // Cria novo nó direito para a chave mais a direita (maior chave)
    NO *no_novo = criar_no(chaves[2], offsets[2], c_arv); // Já faz c_arv->proxRRN++, então o RRN desse novo nó é c_arv->proxRRN -1
    no_novo->nroChaves = 1;
    no_novo->C1 = chaves[2];
    no_novo->PR1 = offsets[2];
    no_novo->P1 = filhos[2];
    no_novo->P2 = filhos[3];
    no_novo->P3 = PTR_NULO;

    // Caso split em nó intermediário ou raiz
    if(no_cheio->tipoNo == INTERMED || no_cheio->tipoNo == RAIZ) {
        if(c_arv->proxRRN != 2) { // Somente no caso em que o nó raiz não é folha (primeiro split que o novo nó deve continuar sendo folha)
            no_novo->tipoNo = INTERMED;
        }   
    } 
    
    // Atualiza o nó cheio para conter só a primeira chave (menor)
    no_cheio->nroChaves = 1; // Apenas 1 chave
    no_cheio->C1 = chaves[0];
    no_cheio->PR1 = offsets[0];
    no_cheio->C2 = -1; // "Apaga" 2° chave
    no_cheio->PR2 = PTR_NULO; // "Apaga" 2° offset
    no_cheio->P1 = filhos[0];
    no_cheio->P2 = filhos[1];
    no_cheio->P3 = PTR_NULO; // P3 fica nulo pois não temos mais a chave C2 nesse nó (então não aponta para ninguém)

    // Promove a chave do meio
    promov->C = chaves[1];
    promov->PR = offsets[1];
    promov->noEsq = RRN_no_cheio; // Usado somente quando há promoção até a raiz
    promov->noDir = (c_arv->proxRRN)-1; // RRN do novo nó criado
    
    // Escreve os dois nós (atual e o criado) no arq da arvB    
    fseek(arvB, ((RRN_no_cheio + CABEC) * 44), SEEK_SET); 
    escrever_na_arvB(no_cheio, arvB);
    fseek(arvB, ((((c_arv->proxRRN)-1) + CABEC) * 44), SEEK_SET); // -1 porque proxRRN já foi incrementado na criação do nó
    escrever_na_arvB(no_novo, arvB);

    // Desalocação
    free(no_novo);
    no_novo = NULL;

    return promov;

}

/*  A função inserir_arvore insere uma nova chave na árvore B, não inserindo chaves duplicadas. Ela trata
    os diferentes casos de inserção:
    - Inserção no nó raiz quando este é o único nó.
    - Inserção que necessita de split, ou que somente insere a chave de forma ordenada no nó.
    Na volta recursiva dessa função, lidamos com as promoções que podem resultar em, somente, inserir de forma ordenada
    no nó de nível superior, que necessitam de splits, ou que geram novas promoções.
*/
PROMOCAO* inserir_arvore(int chave, long int offset, int RRN_atual, CABECALHO_ARVORE *c_arv, FILE *arvB) {
    if(RRN_atual == -1) { // Caso base recursão
        return NULL; 
    }
 
    PROMOCAO *promov_retornado = NULL;

    NO *no_atual = (NO *) malloc(sizeof(NO));
    if(no_atual == NULL) {
        return NULL;
    }
   
    // Acessa no arquivo da arvB para pegar o nó indicado pelo RRN atual
    int pos = (RRN_atual + CABEC) * 44; // RRN_atual + 1 porque o registro do cabeçalho está no primeiro RRN
    fseek(arvB, pos, SEEK_SET);
    ler_da_arvB(no_atual, arvB); // Preenche a struct do no_atual de acordo com o arqv arvB

    if (chaves_iguais(chave, no_atual->C1) || (no_atual->nroChaves > 1 && chaves_iguais(chave, no_atual->C2))) {
        free(no_atual);
        no_atual = NULL;
        return NULL; // Não insere chaves duplicadas
    }

    if(no_atual->tipoNo == FOLHA || (no_atual->tipoNo == RAIZ && c_arv->nroNos == 1)) { // Encontrou o nó folha para inserção, ou o caso em que a raiz é folha
        if(no_atual->nroChaves < CAPACIDADE) { // Tem espaço no nó
            insere_ordenado(chave, offset, PTR_NULO, no_atual); // Como é folha filho_dir == -1 (PTR_NULO)
            fseek(arvB, pos, SEEK_SET);
            escrever_na_arvB(no_atual, arvB); // Escreve no arquivo arvB o nó atual
            return NULL; // Não houve promoção

        } else { // Não tem espaço no nó folha
            promov_retornado = split(chave, offset, PTR_NULO, RRN_atual, no_atual, c_arv, arvB);
            // Split já escreve os nós no arquivo arvB
            return promov_retornado; // Houve promoção
                    
        }

    } else { // Não é nó folha, então continua buscando
        int ptr_filho;
        
        if(chave < no_atual->C1) {
            ptr_filho = no_atual->P1;
        
        } else if(chave < no_atual->C2 || no_atual->C2 == -1) {
            ptr_filho = no_atual->P2;

        } else {
            ptr_filho = no_atual->P3;

        }

        promov_retornado =  inserir_arvore(chave, offset, ptr_filho, c_arv, arvB); // Chama recursão para o ptr_filho
    }

    // Volta da recursão

    if(promov_retornado == NULL) {  // Não houve promoção
        return NULL;
    }

    // Se houve promoção precisamos propagar para os níveis superiores
    if(no_atual->nroChaves < CAPACIDADE) { // Tem espaço no nó superior
        insere_ordenado(promov_retornado->C, promov_retornado->PR, promov_retornado->noDir, no_atual);
        fseek(arvB, (RRN_atual + CABEC) * 44, SEEK_SET);
        escrever_na_arvB(no_atual, arvB); // Escreve no arquivo arvB o nó atual
        return NULL; // Não houve outra promoção

    } else { // Não tem espaço no nó superior
        PROMOCAO *nova_promocao = split(promov_retornado->C, promov_retornado->PR, promov_retornado->noDir, RRN_atual, no_atual, c_arv, arvB); 
        // Split já escreve os nós no arquivo arvB

        free(promov_retornado); // Libera a promoção antiga
        promov_retornado = NULL;
        free(no_atual); // Libera o nó
        no_atual = NULL;

        return nova_promocao; // Houve outra promoção
    }
}

/*  A função inserir_novo_arv abre os arquivos e altera o status do cabeçalho de ambos. Recebe a entrada do usuário e, com auxílio
    da função analisar_entrada, armazena os novos dados na struct registro. A inserção no arquivo de dados segue a mesma lógica da funcionalidade
    5, então usamos a função verificar_criterios_insercao, porém modificada para que no parâmetro offset, tenhamos o offset correto de onde 
    ocorreu a inserção. Após isso, a chave é inserida na árvore.
    Por fim, o cabeçalho de ambos os arquivos é atualizado somente no fim das inserções, evitando fseeks desnecessários.
*/
void inserir_novo_arv() {
    char arquivo_bin[20];
    char arquivo_arvB[20];
    int quant_insercoes;

    scanf("%s %s %d", arquivo_bin, arquivo_arvB,  &quant_insercoes);
    getchar();

    FILE *bin = fopen(arquivo_bin, "rb+"); // Abre para escrita e leitura
    if(bin == NULL) {
        print_erro();
        return;
    }

    FILE *arvB = fopen(arquivo_arvB, "rb+"); // Abre para escrita e leitura
    if(arvB == NULL) {
        print_erro();
        return;
    }
    
    REGISTRO *r = (REGISTRO *) malloc(sizeof(REGISTRO));
    if(r == NULL) {
        return;
    }

    CABECALHO *c = (CABECALHO *) malloc(sizeof(CABECALHO));
    if(c == NULL) {
        return;
    }

    CABECALHO_ARVORE *c_arv = (CABECALHO_ARVORE *) malloc(sizeof(CABECALHO_ARVORE));
    if(c_arv == NULL) {
        return;
    }

    c->status = '0'; // Inconsistente arquivo de dados
    fwrite(&(c->status), 1, 1, bin); 
    fread(&(c->topo), 8, 1, bin); // Obtém topo da lista de removidos

    c_arv->status = '0'; // Inconsistente árvore
    fwrite(&(c_arv->status), 1, 1, arvB); 
    ler_cabecalho(1, c_arv, arvB); // Obtém as informações do cabeçalho (raiz, proxRRN...)

    int sobrescritos = 0;
    char entrada[200];
    long int offset; // Será passado por referência para que, a partir da busca no arq de dados, tenhamos o offset

    for (int i = 0; i < quant_insercoes; i++) {
        scanf(" %199[^\n\r]", entrada); // Captura a linha exceto \n e \r
        getchar(); 
        
        // Insere primeiro no arquivo de dados
        analisar_entrada(entrada, r); // Preenche a struct com os novos valores
        sobrescritos += verificar_criterios_insercao(r, c, &offset, bin, NULL); 
        
        // Insere na arvB com a chave e o offset corretos
        if(c_arv->proxRRN == 0) { // Caso raiz (criada fora do algoritmo de inserção)
            fseek(arvB, (c_arv->proxRRN + CABEC) * 44, SEEK_SET); // Pois como li o cabeçalho, preciso pular crifões
            NO *raiz = criar_no(r->idAttack, offset, c_arv); // Cria o 1° nó (raiz)
            escrever_na_arvB(raiz, arvB);

        } else {
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
                fseek(arvB, (RRN_raiz_antiga + CABEC) * 44, SEEK_SET);
                
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

                fseek(arvB, (RRN_raiz_antiga + CABEC) * 44, SEEK_SET); 
                escrever_na_arvB(raiz_antiga, arvB); // Escreve novamente mas agora atualizado corretamente

                fseek(arvB, (RRN_nova_raiz + CABEC) * 44, SEEK_SET); 
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

        
    }
    
    // Atualizar cabeçalho do arquivo de dados
    fseek(bin, 0, SEEK_END);  // Para atualizar corretamente o proxByteOffset do cabeçalho (que é sempre o fim do registro)
    // atualiza_cabecalho já altera o status de volta a '1' e atualiza proxByteOffset
    atualiza_cabecalho(sobrescritos, 0, quant_insercoes, ftell(bin), c, bin);

    // Atualizar cabecalho da árvore
    c_arv->status = '1';
    fseek(arvB, 0, SEEK_SET); // Volta ao começo do arquivo da arvB
    atualizar_cabecalho_arv(c_arv, arvB);

    // visualizar_arvore(c_arv, arvB);

    fclose(bin);
    fclose(arvB);

    binarioNaTela(arquivo_bin);
    binarioNaTela(arquivo_arvB);

    // Desalocações
    free(r);
    r = NULL;
    free(c);
    c = NULL;
    free(c_arv);
    c_arv = NULL;
}