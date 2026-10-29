#include "remover_arvB.h"
#include "funcoes_arvB.h"
#include "funcoes_auxiliares.h"
#include "funcao-fornecida.h"

// Tipo do nó
#define FOLHA -1
#define RAIZ 0
#define INTERMED 1

#define PTR_NULO -1 // Ponteiro (RRN) para outro nó
#define TAXA_OCUP 1 // Teto de m/2 => 3/2 = 1
#define CABEC 1 // fseek considerando cabeçalho

// Em relação ao nó pai onde está o nó anterior (anterior na recursão)
#define ESQUERDA 1
#define DIREITA 2

PROMOCAO* remover_chave(int chave, int RRN_atual, CABECALHO_ARVORE *c_arv, FILE *arvB) ;

void apagar_c1(NO *no) {
    no->C1 = no->C2; // Para não deixar espaço copia o c2 para c1, "apagando" c1
    no->PR1 = no->PR2; // Copia offset de c2 para offset de c1, "apagando" offset de c1
    no->P1 = no->P2; // Rearranja ponteiros
    no->P2 = no->P3;
    no->C2 = -1; // "Apaga" c2
    no->PR2 = PTR_NULO; // "Apaga" offset 
    (no->nroChaves)--;
}

void apagar_c2(NO *no) {
    no->C2 = -1; // "Apaga" c2
    no->PR2 = PTR_NULO; // "Apaga" offset
    no->P3 = PTR_NULO; // "Apaga" ponteiro
    (no->nroChaves)--;
}

void apagar_no(NO *no, CABECALHO_ARVORE *c_arv) {
    // no->tipoNo = FOLHA; 
    no->nroChaves = 0; 
    no->P1 = PTR_NULO;
    no->C1 = -1;
    no->PR1 = PTR_NULO;
    no->P2 = PTR_NULO;
    no->C2 = -1;
    no->PR2 = PTR_NULO;
    no->P3 = PTR_NULO;

    (c_arv->nroNos)--;
}

PROMOCAO* trocar(int chave_remoc, int RRN_atual, CABECALHO_ARVORE *c_arv, FILE *arvB) {
    // Busca pela menor chave de uma folha da direita
    int chave;
    long int offset;

    NO *no_atual = (NO *) malloc(sizeof(NO));
    if(no_atual == NULL) {
        return NULL;
    }

    while(RRN_atual != PTR_NULO) {
        // Acessa no arquivo da arvB para pegar o nó indicado pelo RRN atual
        int pos = (RRN_atual + CABEC) * 44; // RRN_atual + 1 porque o registro do cabeçalho está no primeiro RRN
        fseek(arvB, pos, SEEK_SET);
        ler_da_arvB(no_atual, arvB); // Preenche a struct do no_atual de acordo com o arqv arvB

        // Guarda a menor chave que encontrar
        chave = no_atual->C1; // c1 é sempre a menor chave do nó
        offset = no_atual->PR1;

        RRN_atual = no_atual->P1; // Se não folha, continua buscando menor chave
    }

    // Como a chave e offset estão guardados, chamo remover para essa chave guardada
    PROMOCAO *res = remover_chave(chave, c_arv->noRaiz, c_arv, arvB);

    // Busca pela chave que realmente quero remover e substitui pela guardada
    // int RRN_remov; // A função busca utilizará essa variável (passada por referência)
    // buscar(chave_remoc, c_arv->noRaiz, &RRN_remov, arvB); 

    // Acessa no arquivo da arvB para pegar o nó indicado pelo RRN_remov
    int pos = (RRN_atual + CABEC) * 44; // RRN_remov + 1 porque o registro do cabeçalho está no primeiro RRN
    fseek(arvB, pos, SEEK_SET);
    ler_da_arvB(no_atual, arvB); // Preenche a struct do no_atual de acordo com o arqv arvB

    // Substituição
    if(no_atual->C1 == chave_remoc) {
        no_atual->C1 = chave;
        no_atual->PR1 = offset;
        // Os ponteiros (P1, P2...) permanecem iguais
    } else {
        no_atual->C2 = chave;
        no_atual->PR2 = offset;
        // Os ponteiros (P1, P2...) permanecem iguais
    }
    
    fseek(arvB, pos, SEEK_SET);
    escrever_na_arvB(no_atual, arvB);

    free(no_atual); // Desaloca o nó 
    no_atual = NULL;

    return res;
}

void redistribuir(NO *pai, NO *irmao, int c_pai, int posic_anterior, PROMOCAO *promov) {
    int chaves[3] = { irmao->C1, irmao->C2, c_pai };
    int vazio = promov->RRN_anterior;

    NO *no_vazio = (NO *) malloc(sizeof(NO));
    if(no_vazio == NULL) {
        return;
    }

    for(int i = 0; i < 2; i++) {
        for(int j = i+1; j < 3; j++) {
            if(chaves[i] > chaves[j]) {
                int aux_chave = chaves[i];
                chaves[i] = chaves[j];
                chaves[j] = aux_chave;
            }
        }
    }

    if(posic_anterior == ESQUERDA) {
        no_vazio->C1 = chaves[0];
        //tratar offset
        pai->C1 = chaves[1];
        irmao->C1 = chaves[2];

    } else {
        no_vazio->C1 = chaves[2];
        //tratar offset
        pai->C2 = chaves[1];
        irmao->C1 = chaves[0];
    }
    // escrever

}

PROMOCAO* concatenar(NO *pai, NO *irmao, int RRN_pai, int RRN_irmao, int c_pai, int posic_anterior, PROMOCAO *promov, CABECALHO_ARVORE *c_arv, FILE *arvB) {
    NO *no_anterior = (NO *) malloc(sizeof(NO));
    if(no_anterior == NULL) {
        return NULL;
    }

    PROMOCAO *nova_promoc = (PROMOCAO *) malloc(sizeof(PROMOCAO));
    if(nova_promoc == NULL) {
        return NULL;
    }

    if(posic_anterior == ESQUERDA) { // Chaves devem ir para nó anterior 
        // Acessa no arquivo da arvB para pegar o nó indicado pelo RRN anterior
        int pos = (promov->RRN_anterior + CABEC) * 44; // RRN_anterior + 1 porque o registro do cabeçalho está no primeiro RRN
        fseek(arvB, pos, SEEK_SET);
        ler_da_arvB(no_anterior, arvB); // Preenche a struct do no_anterior de acordo com o arqv arvB

        if(c_pai == pai->C1) { // Pai c1
            no_anterior->C1 = pai->C1;
            no_anterior->PR1 = pai->PR1;
            apagar_c1(pai);  

        } else { // Pai c2
            no_anterior->C1 = pai->C2;
            no_anterior->PR1 = pai->PR2;
            apagar_c2(pai);
        }

        no_anterior->C2 = irmao->C1;
        no_anterior->PR2 = irmao->PR1; 
        no_anterior->nroChaves = 2;
        apagar_no(irmao, c_arv);

        if(pai->nroChaves < TAXA_OCUP) { // Underflow no pai
            if(pai->tipoNo == RAIZ) {
                no_anterior->tipoNo = RAIZ;
                c_arv->noRaiz = promov->RRN_anterior;
                apagar_no(pai, c_arv); // Destruir no
                
                int pos_pai = (RRN_irmao + CABEC) * 44; // RRN_irmao + 1 porque o registro do cabeçalho está no primeiro RRN
                fseek(arvB, pos_pai, SEEK_SET);
                escrever_na_arvB(pai, arvB);
                nova_promoc = NULL;

            } else {
                nova_promoc->RRN_anterior = RRN_pai;
            }

        } else {
            nova_promoc = NULL;
        }

        fseek(arvB, pos, SEEK_SET); 
        escrever_na_arvB(no_anterior, arvB);

        int pos_irmao = (RRN_irmao + CABEC) * 44; // RRN_irmao + 1 porque o registro do cabeçalho está no primeiro RRN
        fseek(arvB, pos_irmao, SEEK_SET);
        escrever_na_arvB(irmao, arvB);

        return nova_promoc;
        
    } else { // Chaves devem ir para nó irmão 
        if(c_pai == pai->C1) { // Pai c1
            irmao->C2 = pai->C1;
            irmao->PR2 = pai->PR1;
            apagar_c1(pai);  

        } else { // Pai c2
            irmao->C2 = pai->C2;
            irmao->PR2 = pai->PR2;
            apagar_c2(pai);
        }

        if(pai->nroChaves < TAXA_OCUP) { // Underflow no pai
            if(pai->tipoNo == RAIZ) {
                irmao->tipoNo = RAIZ;
                c_arv->noRaiz = RRN_irmao;
                apagar_no(pai, c_arv); // Destruir no
                
                int pos_pai = (RRN_irmao + CABEC) * 44; // RRN_irmao + 1 porque o registro do cabeçalho está no primeiro RRN
                fseek(arvB, pos_pai, SEEK_SET);
                escrever_na_arvB(pai, arvB);
                nova_promoc = NULL;

            } else {
                nova_promoc->RRN_anterior = RRN_pai;
            }

        } else {
            nova_promoc = NULL;
        }
        
        int pos = (RRN_irmao + CABEC) * 44; // RRN_anterior + 1 porque o registro do cabeçalho está no primeiro RRN
        fseek(arvB, pos, SEEK_SET);
        escrever_na_arvB(irmao, arvB); 
        
        int pos_anterior = (promov->RRN_anterior + CABEC) * 44; // RRN_irmao + 1 porque o registro do cabeçalho está no primeiro RRN
        fseek(arvB, pos_anterior, SEEK_SET);
        escrever_na_arvB(no_anterior, arvB);

        return nova_promoc;
    }
}

PROMOCAO* remover_chave(int chave, int RRN_atual, CABECALHO_ARVORE *c_arv, FILE *arvB) {
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

    PROMOCAO *promov = (PROMOCAO *) malloc(sizeof(PROMOCAO));
    if(promov == NULL) {
        return NULL;
    }

    int posic_escrita = -1;
    
    if (chaves_iguais(chave, no_atual->C1) ) { // Achou a chave a ser removida
        if(no_atual->tipoNo == RAIZ && c_arv->nroNos == 1) { // Caso raiz=folha
            apagar_c1(no_atual); // c2 assume o lugar de c1, para não deixar "espaço vazio"
            posic_escrita = pos;
            if(no_atual->nroChaves < TAXA_OCUP) {
                apagar_no(no_atual, c_arv);
            }

        } else if(no_atual->tipoNo != FOLHA) { 
            promov = trocar(chave, no_atual->P2, c_arv, arvB); // Troca com a menor chave da direita (sucessora imediata)
            // no_atual->P2 (para procurar a menor chave a direita)

        } else { // Remover c1
            apagar_c1(no_atual); // c2 assume o lugar de c1, para não deixar "espaço vazio"

            if(no_atual->nroChaves < TAXA_OCUP) { // Underflow
                promov->RRN_anterior = RRN_atual;
                apagar_no(no_atual, c_arv);
                posic_escrita = pos;

            } else {
                promov = NULL;
                posic_escrita = pos;
            }
        }
        
        if(posic_escrita != -1) {
            fseek(arvB, pos, SEEK_SET);
            escrever_na_arvB(no_atual, arvB);    
        }

        return promov;

    } else if (no_atual->nroChaves > 1 && chaves_iguais(chave, no_atual->C2)) { // Achou a chave a ser removida
        if(no_atual->tipoNo == RAIZ && c_arv->nroNos == 1) { // Caso raiz=folha
            apagar_c2(no_atual); // Se tenho c2, tenho c1 então não tenho underflow
            posic_escrita = pos;
            promov = NULL;

        } else if(no_atual->tipoNo != FOLHA) {
            promov = trocar(chave, no_atual->P3, c_arv, arvB); // Troca com a menor chave da direita (sucessora imediata)
            // no_atual->P3 (para procurar a menor chave a direita)

        } else { // Remover c2
            apagar_c2(no_atual);
            promov = NULL; // Se eu tenho c2 obrigatoriamente tenho c1 (não dá underflow)
            posic_escrita = pos;
        }
        
        if(posic_escrita != -1) {
            fseek(arvB, pos, SEEK_SET);
            escrever_na_arvB(no_atual, arvB);    
        }

        return promov;

    } else { // Não encontrou a chave no nó, então continua buscando
        int ptr_filho;
        
        if(chave < no_atual->C1) {
            ptr_filho = no_atual->P1;
        
        } else if(chave < no_atual->C2 || no_atual->C2 == -1) {
            ptr_filho = no_atual->P2;

        } else {
            ptr_filho = no_atual->P3;

        }

        promov_retornado = remover_chave(chave, ptr_filho, c_arv, arvB); // Chama recursão para o ptr_filho
    }

    // Volta de recursão
    if(promov_retornado == NULL) {
        // Estamos no pai (pela volta da recursão)
        int RRN_pag_irma;
        int c_pai; // Chave pai
        int posic_anterior;

        if(promov_retornado->RRN_anterior == no_atual->P1) {
            posic_anterior = ESQUERDA; // Em relação ao pai
            c_pai = no_atual->C1;
            RRN_pag_irma = no_atual->P2; // Direita

        } else if (promov_retornado->RRN_anterior == no_atual->P2){
            if(no_atual->P3 != PTR_NULO) { // Posso usar irmão da direita
                posic_anterior = ESQUERDA;  // Em relação ao pai
                c_pai = no_atual->C2;
                RRN_pag_irma = no_atual->P3; // Direita

            } else { // Usar irmão da esquerda
                posic_anterior = DIREITA;  // Em relação ao pai
                c_pai = no_atual->C1;
                RRN_pag_irma = no_atual->P1; // Esquerda
            }
            

        } else { // promov_retornado->RRN_anterior == no_atual->P3, usar irmão da esquerda
            posic_anterior = DIREITA; // Em relação ao pai 
            c_pai = no_atual->C2;
            RRN_pag_irma = no_atual->P2; // Esquerda
        }

        NO *no_irmao = (NO *) malloc(sizeof(NO));
        if(no_irmao == NULL) {
            return NULL;
        }
        // Acessa no arquivo da arvB para pegar o nó indicado pelo RRN da pág irmã
        int posic = (RRN_pag_irma + CABEC) * 44; // RRN_atual + 1 porque o registro do cabeçalho está no primeiro RRN
        fseek(arvB, posic, SEEK_SET);
        ler_da_arvB(no_irmao, arvB); // Preenche a struct do no_irmao de acordo com o arqv arvB

        if(no_irmao->nroChaves > TAXA_OCUP) {
            //redistribuir(no_atual, no_irmao, c_pai, posic_anterior);

        } else { // no_irmao->nroChaves == TAXA_OCUP
            concatenar(no_atual, no_irmao, RRN_atual, RRN_pag_irma, c_pai, posic_anterior, promov_retornado, c_arv, arvB);
        }
    }
}

void remover_arvB() {
    int quant_buscas;
    char arquivo_dados[20];
    char arquivo_arvB[20];
    
    scanf("%s %s %d", arquivo_dados,  arquivo_arvB, &quant_buscas);

    FILE *bin = fopen(arquivo_dados, "rb+"); // Leitura e escrita
    if(bin == NULL) {
        print_erro();
        return;
    }

    FILE *arvB = fopen(arquivo_arvB, "rb+"); // Leitura e escrita
    if(arvB == NULL) {
        print_erro();
        return;
    }
    
    CABECALHO *c = (CABECALHO *) malloc(sizeof(CABECALHO));
    if(c == NULL) {
        return;
    }

    descricoes_dos_campos(c, bin);

    int removidos = busca(bin, quant_buscas, c, 0, arvB);

    // Atualizar cabeçalho do arquivo de dados
    fseek(bin, 0, SEEK_END);  // Para atualizar corretamente o proxByteOffset do cabeçalho (que é sempre o fim do registro)
    // atualiza_cabecalho já altera o status de volta a '1' e atualiza proxByteOffset
    atualiza_cabecalho(0, removidos, 0, ftell(bin), c, bin);

    fclose(bin);
    fclose(arvB);

    binarioNaTela(arquivo_dados);
    binarioNaTela(arquivo_arvB);

    free(c); // Desalocação
    c = NULL;
}