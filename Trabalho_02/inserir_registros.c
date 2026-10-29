#include "remover_registros.h"
#include "funcoes_auxiliares.h"
#include "funcao-fornecida.h"

#define NULO_INT   -1
#define NULO_FLOAT -1.0

/* A função analisar_entrada recebe a entrada do usuário e preenche a struct registro, tratando
   os casos de campos com valores nulos com a atribuição de NULO_INT ou NULO_FLOAT (nos campos fixos)
   ou ponteiro NULL (nos campos variáveis).
   A função ler_campo está no arquivo de funções auxiliares.
*/
void analisar_entrada(char entrada[200], REGISTRO *r) {
    char *ptr = entrada;

    r->removido = '0';
    r->prox = -1;

    // idAttack (não pode ser nulo)
    char *campo = ler_campo(&ptr);
    r->idAttack = atoi(campo);
    free(campo);

    // year
    campo = ler_campo(&ptr);
    if (campo == NULL || strcmp(campo, "NULO") == 0) {
        r->year = NULO_INT;
    } else {
        r->year = atoi(campo);
    }
    free(campo);

    // financialLoss
    campo = ler_campo(&ptr);
    if (campo == NULL || strcmp(campo, "NULO") == 0) {
        r->financialLoss = NULO_FLOAT;
    } else {
        r->financialLoss = atof(campo);
    }

    free(campo); // Desaloca campo

    // campos variáveis
    r->country = ler_campo(&ptr);
    r->attackType = ler_campo(&ptr);
    r->targetIndustry = ler_campo(&ptr);
    r->defenseMechanism = ler_campo(&ptr);
}

/* A função tam_str é necessária para que a função escrever_campos_variaveis opere corretamente, não escrevendo nada
   se o campo for nulo, indicado pelo retorno do tamanho da inserção == -1, no caso nulo.
*/
int tam_str(char *str) {
    if (str != NULL) {
        return strlen(str);
    }

    return -1; // Caso campo nulo
}

// Insere os campos do registro a partir da struct REGISTRO
void inserir_registro(REGISTRO *r, FILE *bin) {
    fwrite(&(r->idAttack), 4, 1, bin);
    fwrite(&(r->year), 4, 1, bin);
    fwrite(&(r->financialLoss), 4, 1, bin);
    escrever_campos_variaveis(r->country, '1', tam_str(r->country), NULL, bin);
    escrever_campos_variaveis(r->attackType, '2',  tam_str(r->attackType), NULL, bin);
    escrever_campos_variaveis(r->targetIndustry, '3', tam_str(r->targetIndustry), NULL, bin);
    escrever_campos_variaveis(r->defenseMechanism, '4', tam_str(r->defenseMechanism), NULL, bin);

}

/* A função sobrescrever é utilizada no caso do novo registro a ser inserido, ter um tamanho menor ou
   igual ao tamanho de um registro removido. Assim, o registro antes removido, é sobrescrito pelo novo
   e, se sobrar espaço preenchido com "$".
   Por fim, verifica-se se o topo da lista deve ser atualizado ou se o anterior da lista deve receber
   o proximo da lista, para garantir que não se perca o "encadeamento".
*/
void sobrescrever(long int anterior_lista, long int atual_lista, long int proximo_lista, int subTam, int tam_antigo, REGISTRO *r, CABECALHO *c, FILE *bin) {
    char removido = '0';
    long int prox_padrao = -1;
    char cifrao = '$';

    inserir_registro(r, bin);

    for (int i = 0; i < subTam; i++) {
        fwrite(&cifrao, 1, 1, bin); // Escreve os cifrões, se não precisar subTam será 0, então não vai escrever nada
    }
    
    fseek(bin, -(tam_antigo + 5), SEEK_CUR);
    fwrite(&removido, 1, 1, bin); // Atualiza para não removido
    fseek(bin, 4, SEEK_CUR); // Pula o tamRegistro que deve permanecer igual
    fwrite(&prox_padrao, 8, 1, bin); 

    // Condições (lista)
    if (anterior_lista == -1) { // c->topo precisa ser atualizado
        c->topo = proximo_lista;
        fseek(bin, 1, SEEK_SET);
        fwrite(&(c->topo), 8, 1, bin);

    } else { // anterior_lista recebe proximo_lista
        fseek(bin, anterior_lista + 5, SEEK_SET);
        fwrite(&proximo_lista, 8, 1, bin);
    }

}

/* A função verificar_criterios_insercao calcula o novo tamanho do registro a ser inserido e, a partir da 
   lista de removidos, verifica se é possível sobrescrever algum registro logicamente removido, reusando espaço,
   se o tamanho antigo for maior ou igual o tamanho novo, ou se a inserção deve ocorrer no fim do arquivo.
   A lógica da lista se dá acessando o topo e se diferente de -1, acessando o prox removido a partir do registro
   que estava indicado pelo topo, sempre atualizando o atual e anterior da lista também, até que o topo seja igual
   a -1, portanto, fim da lista.
*/
int verificar_criterios_insercao(REGISTRO *r, CABECALHO *c, FILE *bin) {
    int sobrescreveu = 0; // Variável que será retornada, necessária para atualizar corretamente o cabeçalho

    int tam_country = r->country ? strlen(r->country) + 2 : 0; // + 2 por causa da keyword e '|'
    int tam_attackType = r->attackType ? strlen(r->attackType) + 2 : 0;
    int tam_targetIndustry = r->targetIndustry ? strlen(r->targetIndustry) + 2 : 0;
    int tam_defenseMechanism = r->defenseMechanism ? strlen(r->defenseMechanism) + 2 : 0;
    
    int tam_novo = 20 + tam_country + tam_attackType + tam_targetIndustry + tam_defenseMechanism; // 20 (campos fixos sem tamregistro e removido)
    
    long int prox_lista = c->topo;
    long int atual_lista = -1;
    long int anterior_lista = -1;

    if (prox_lista != -1) {
        int tam_antigo;

        do { 
            fseek(bin, (prox_lista + 1), SEEK_SET);  // Acessa registro removido
            fread(&tam_antigo, 4, 1, bin); // Tamanho antigo

            anterior_lista = atual_lista; // Anterior recebe atual
            atual_lista = prox_lista; // Atual recebe prox
            fread(&prox_lista, 8, 1, bin); // prox recebe o prox da lista

            int subTam = tam_antigo - tam_novo;

            if (subTam >= 0) { // Tem espaço
                sobrescrever(anterior_lista, atual_lista, prox_lista, subTam, tam_antigo, r, c, bin);
                sobrescreveu++;
                break; 
            }

        } while (prox_lista != -1);
        
    } 

    if (sobrescreveu == 0) { // Insere no fim
        fseek(bin, 0, SEEK_END); // Fim do arquivo
        
        // Atualiza remov, tamRegis e prox
        r->removido = '0';
        r->prox = -1; // Valor padrão
        fwrite(&(r->removido), 1, 1, bin);
        fwrite(&tam_novo, 4, 1, bin);
        fwrite(&(r->prox), 8, 1, bin);

        inserir_registro(r, bin);
    }  

    return sobrescreveu; 
}

/*  A função inserir_registros é a responsável por toda a lógica de inserção. Primeiro ela lê o nome do arquivo e a quantidade
    de registros a serem inseridos, depois abre o arquivo binário e marca o status como '0', pois o arquivo está sendo utilizado. 
    Lê o campo topo, do cabeçalho, para manter o controle do topo da lista de removidos. 
    Para cada linha de entrada, a função analisar_entrada é chamada para preencher os dados do novo registro a ser inserido na struct REGISTRO. 
    Depois, a função verificar_criterios_insercao verifica o caso de inserção: Se existe um registro removido com espaço suficiente para 
    armazenar o novo registro e, caso exista, a função sobrescrever é usada para reutilizar esse espaço. Caso contrário, 
    o novo registro será escrito ao final do arquivo pela função inserir_registro. 
    Após todas as inserções, a função atualiza_cabecalho altera o cabeçalho com os valores corretos de registros inseridos, removidos e o 
    próximo byte offset, além de marcar o status como '1', indicando que o arquivo está consistente.
*/
void inserir_registros() {
    char arquivo[20];
    int quant_insercoes;
    
    scanf("%s %d", arquivo,  &quant_insercoes);
    getchar();

    FILE *bin = fopen(arquivo, "rb+"); // Abre para escrita e leitura
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

    c->status = '0'; // Inconsistente
    fwrite(&(c->status), 1, 1, bin); 

    fread(&(c->topo), 8, 1, bin); // Obtém topo da lista

    int sobrescritos = 0;
    char entrada[200];

    for (int i = 0; i < quant_insercoes; i++) {
        scanf(" %199[^\n\r]", entrada); // Captura a linha exceto \n e \r
        getchar(); // Remove o terminador (\n ou \r) residual
        
        analisar_entrada(entrada, r); // Preenche a struct com os novos valores
        sobrescritos += verificar_criterios_insercao(r, c, bin); // Se não sobrescrever somará com 0
    }

    fseek(bin, 0, SEEK_END);  // Para atualizar corretamente o proxByteOffset do cabeçalho (que é sempre o fim do registro)
    // atualiza_cabecalho já altera o status de volta a '1' e atualiza proxByteOffset
    atualiza_cabecalho(sobrescritos, 0, quant_insercoes, ftell(bin), c, bin);

    fclose(bin);
    binarioNaTela(arquivo);

    // Desalocação das structs
    free(c);
    free(r);
}