#include "funcoes_arvB.h"

// Tipo do nó
#define FOLHA -1
#define RAIZ 0
#define INTERMED 1

#define PTR_NULO -1 // Ponteiro (RRN) para outro nó
#define CAPACIDADE 2 // 2 chaves por nó
#define CABECALHO_ARV 1 // fseek considerando cabeçalho

/*  A função ler_cabecalho obtém as informações do cabeçalho da árvore, armazenando-os
    na struct cabecalho_arvore.
    O parâmetro (int status) auxilia para que a função possa ser usada mesmo se o status
    acabou de ser atualizado (quando abre para escrita e já altera para inconsistente), sendo
    que 1 indica que a leitura do status não precisa ser feita.
*/
void ler_cabecalho(int status, CABECALHO_ARVORE *c_arv, FILE *arvB) {
    if(status != 1) { // Não precisa ler status 
        fread(&(c_arv->status), 1, 1, arvB); 
    }
    fread(&(c_arv->noRaiz), 4, 1, arvB);
    fread(&(c_arv->proxRRN), 4, 1, arvB);
    fread(&(c_arv->nroNos), 4, 1, arvB);
}

/*  A função atualizar_cabecalho_arv recebe os dados atualizados do cabeçalho e
    os escreve na árvore B. O fseek é dado antes da chamada da função.
*/
void atualizar_cabecalho_arv(CABECALHO_ARVORE *c_arv, FILE *arvB) {
    fwrite(&(c_arv->status), 1, 1, arvB); 
    fwrite(&(c_arv->noRaiz), 4, 1, arvB);
    fwrite(&(c_arv->proxRRN), 4, 1, arvB);
    fwrite(&(c_arv->nroNos), 4, 1, arvB);
}

/*  A função chaves_iguais compara a chave de busca desejada com a do nó sendo verificado. */
bool chaves_iguais(int chave, int chave_no_atual) {
    if(chave == chave_no_atual) {
        return true;
    }
    return false;
}

/*  A função ler_da_arvB obtém as informações de um nó da árvore B, o fseek é
    dado antes da função ser chamada.
*/
void ler_da_arvB(NO *no, FILE *arvB) {
    fread(&(no->tipoNo), 4, 1, arvB);
    fread(&(no->nroChaves), 4, 1, arvB);
    fread(&(no->P1), 4, 1, arvB);
    fread(&(no->C1), 4, 1, arvB);
    fread(&(no->PR1), 8, 1, arvB);
    fread(&(no->P2), 4, 1, arvB);
    fread(&(no->C2), 4, 1, arvB);
    fread(&(no->PR2), 8, 1, arvB);
    fread(&(no->P3), 4, 1, arvB);
}

/*  A função escrever_na_arvB escreve as informações de um nó na árvore B, o fseek é
    dado antes da função ser chamada.
*/
void escrever_na_arvB(NO *no, FILE * arvB) {
    fwrite(&(no->tipoNo), 4, 1, arvB);
    fwrite(&(no->nroChaves), 4, 1, arvB);
    fwrite(&(no->P1), 4, 1, arvB);
    fwrite(&(no->C1), 4, 1, arvB);
    fwrite(&(no->PR1), 8, 1, arvB);
    fwrite(&(no->P2), 4, 1, arvB);
    fwrite(&(no->C2), 4, 1, arvB);
    fwrite(&(no->PR2), 8, 1, arvB);
    fwrite(&(no->P3), 4, 1, arvB);
}

/*  A função visualizar_arvore é uma função usada nos testes, ela permite vizualizar a
    árvore B a partir de um arquivo.
*/
void visualizar_arvore(CABECALHO_ARVORE *c_arv, FILE *arvB) {
    char cifroes[32];

    NO *no_arv = (NO *) malloc(sizeof(NO));
        if(no_arv == NULL) {
            return;
        }

    fseek(arvB, 0, SEEK_SET);

    fread(&(c_arv->status), 1, 1, arvB);
    fread(&(c_arv->noRaiz), 4, 1, arvB);
    fread(&(c_arv->proxRRN), 4, 1, arvB);
    fread(&(c_arv->nroNos), 4, 1, arvB);
    fread(cifroes, 31, 1, arvB);
    cifroes[31] = '\0'; 

    printf("=== Cabeçalho da Árvore B ===\n");
    printf("Status: %c\n", c_arv->status);
    printf("RRN da raiz: %d\n", c_arv->noRaiz);
    printf("Próximo RRN: %d\n", c_arv->proxRRN);
    printf("Número de nós: %d\n", c_arv->nroNos);
    printf("Cifroes: %s\n\n", cifroes);

    printf("=== Nós da Árvore B ===\n");
    int i = 0;
    while(fread(&(no_arv->tipoNo), sizeof(int), 1, arvB) == 1) {
        fseek(arvB, -4, SEEK_CUR); // Volta tipo do nó, pq  a função abaixo faz isso
        ler_da_arvB(no_arv, arvB);
        printf("RRN nó %d\n", i);
        printf("Tipo nó %d\n", no_arv->tipoNo);
        printf("Nros chaves %d\n", no_arv->nroChaves);
        printf("P1 %d\n", no_arv->P1);
        printf("C1 %d\n", no_arv->C1);
        printf("PR1 %ld\n", no_arv->PR1);
        printf("P2 %d\n", no_arv->P2);
        printf("C2 %d\n", no_arv->C2);
        printf("PR2 %ld\n", no_arv->PR2);
        printf("P3 %d\n\n", no_arv->P3);
        i++;
    }

    free(no_arv);
    no_arv = NULL;
}

/*  A função criar_no cria um novo nó da árvore B, tratando todos os nós como folhas, exceto
    no caso em que o nó raiz é uma folha, nesse caso, altera o tipo do nó para raiz.
    A função também atualiza a struct do cabeçalho, incrementando o número de nós e o proxRRN.
*/
NO* criar_no(int chave, long int offset, CABECALHO_ARVORE *c_arv) { 
    NO *no_arv = (NO *) malloc(sizeof(NO));
    if(no_arv == NULL) {
        return NULL;
    }

    // Cria todos como folha
    no_arv->tipoNo = FOLHA; 
    no_arv->nroChaves = 1; // Somente 1 chave inicialmente
    no_arv->P1 = PTR_NULO;
    no_arv->C1 = chave;
    no_arv->PR1 = offset;
    no_arv->P2 = PTR_NULO;
    no_arv->C2 = -1;
    no_arv->PR2 = -1;
    no_arv->P3 = PTR_NULO;

    if(c_arv->noRaiz == -1) { // Primeiro nó criado (noRaiz = nó folha = -1)
        c_arv->noRaiz = c_arv->proxRRN; // Raiz recebe o proxRRN, no caso 0
        no_arv->tipoNo = RAIZ;
    }

    (c_arv->nroNos)++;
    (c_arv->proxRRN)++;

    return no_arv;
}

/*  A função buscar é a função recursiva da busca na árvore B, ela acessa os nós na árvore
    de acordo com o critério de busca em árvore B (menor->esquerda, maior->direita) e compara
    se alguma das chaves contidas no nó, é a desejada.
    A função é usada também na atualização com árvore B, desse modo, em caso de atualização 
    o parâmetro (long int novo_offset) é usado para atualizar corretamente o offset de uma determinada
    chave (caso durante a atualização o registro mude de lugar no arquivo de dados), se não for o caso de 
    atualização, esse parâmetro terá o valor -1.
    O retorno é o offset que o registro, que contém aquela chave, está no arquivo de dados, sendo
    retornado para que a função buscar_registro_arvB acesse diretamente o registro no arquivo de dados.
*/
long int buscar(int chave, int RRN_atual, long int novo_offset, FILE *arvB) {
    if(RRN_atual == -1) { // Caso base recursão
        return -1; // Não encontrado
    }

    NO *no_atual = (NO *) malloc(sizeof(NO));
    if(no_atual == NULL) {
        return -1;
    }
   
    // Acessa no arquivo da arvB para pegar o nó indicado pelo RRN atual
    int pos = (RRN_atual + CABECALHO_ARV) * 44; // RRN_atual + 1 porque o registro do cabeçalho está no primeiro RRN
    fseek(arvB, pos, SEEK_SET);
    ler_da_arvB(no_atual, arvB); // Preenche a struct do no_atual de acordo com o arqv arvB

    if (chaves_iguais(chave, no_atual->C1)) {
        if(novo_offset != -1) { // Necessário para a funcionalidade de atualização com arvB
            no_atual->PR1 = novo_offset; // Em caso de atualização offset precisa ser atualizado
            fseek(arvB, pos, SEEK_SET); // Volta para escrever na arvB
            escrever_na_arvB(no_atual, arvB); 
        }
        return no_atual->PR1; // Offset do arquivo de dados, necessário na funcionalidade da busca com arvB

    } else if (no_atual->nroChaves > 1 && chaves_iguais(chave, no_atual->C2)) {
        if(novo_offset != -1) { // Necessário para a funcionalidade de atualização com arvB
            no_atual->PR2 = novo_offset; // Em caso de atualização offset precisa ser atualizado
            fseek(arvB, pos, SEEK_SET); // Volta para escrever na arvB
            escrever_na_arvB(no_atual, arvB); 
        }
        return no_atual->PR2; // Offset do arquivo de dados, necessário na funcionalidade da busca com arvB
    }

    // Se não encontrou continua buscando
    int ptr_filho;
    if(chave < no_atual->C1) {
        ptr_filho = no_atual->P1;
    
    } else if(chave < no_atual->C2 || no_atual->C2 == -1) {
        ptr_filho = no_atual->P2;

    } else {
        ptr_filho = no_atual->P3;

    }

    buscar(chave, ptr_filho, novo_offset, arvB); // Chamada recursiva
}

/*  Função teste para exibir uma árvore a partir de um arquivo */
void teste() {
    CABECALHO_ARVORE *c_arv = malloc(sizeof(CABECALHO_ARVORE));

    char arquivo_arvB[20];
    
    scanf("%s", arquivo_arvB);

    FILE *arvB = fopen(arquivo_arvB, "rb"); 

    visualizar_arvore(c_arv, arvB);

    fclose(arvB);
    free(c_arv);
    c_arv = NULL;
}