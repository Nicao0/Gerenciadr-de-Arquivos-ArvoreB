#include "funcoes_auxiliares.h"
#include "funcao-fornecida.h"
#include "atualizar_registros.h"
#include "remover_registros.h"
#include <stdbool.h>

// Função para exibir uma mensagem padrão de erro no processamento de arquivos.
void print_erro() {
    printf("Falha no processamento do arquivo.");
}

// Armazena na struct as descrições dos campos com o terminador \0 evitando invasão de memória
void descricoes_dos_campos(CABECALHO *c, FILE *bin) {
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
}

/* A função ler_campos_variaveis compara a keyword (passada como parâmetro) e a keyword lida (c), para saber
   se o campo variável existe, se existir, começa a ler o campo, byte a byte, até encontrar o pipe (|), lixo, EOF, ou o \n,
   a memória do campo é realocada dinamicamente e garante-se que a string termine com \0 na struct.
*/
int ler_campos_variaveis(char **ptr_registro, char keyword, FILE *bin) {
    char c;
    int tamanho = 0;

    *ptr_registro = NULL; // Começa com ponteiro nulo

    if (fread(&c, 1, 1, bin) != 1) {
        return 0; // Erro ao ler o arquivo
    }

    if(c != keyword) { // Se a keyword desejada não for encontrada significa que o campo está vazio (não existe)
        ungetc(c, bin); // Antes de retornar volta o caracter lido (que não é a keyword)
        return tamanho; // Tamanho = 0 (campo vazio)
    }

    while ((c = fgetc(bin)) != '|' && '$' && c != '\n' && c != EOF) { // Se a keyword corresponder, lê o campo caracter a caracter
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

/* A função atualiza_cabecalho altera a status do arquivo para consistente, não altera o campo de topo da lista,
   tendo em vista que esse campo é atualizado sempre que necessário (na remoção/inserção/atualização), recebe por parâmetro o fim
   do arquivo (para atualizar/escrever o proxOffset) e calcula a quantidade de registros existentes e logicamente removidos.
*/
void atualiza_cabecalho(int sobrescritos, int removidos, int inseridos, long int proxOffset, CABECALHO *c, FILE *bin) {
    fseek(bin, 0, SEEK_SET);

    c->status = '1'; // Finalizou o uso então está consistente (1)
    fwrite((&c->status), 1, 1, bin);
    fseek(bin, 8, SEEK_CUR); // Pula topo, pq ele já é atualizado quando necessário (remoção/inserção)
    fwrite(&proxOffset, 8, 1, bin); // Sempre o fim do arquivo

    fread(&(c->nroRegArq), 4, 1, bin);
    fread(&(c->nroRegRem), 4, 1, bin);

    fseek(bin, -8, SEEK_CUR); // Volta para escrever nroRegArq e nroRegRem

    int soma_reg = c->nroRegArq + inseridos - removidos;
    fwrite(&soma_reg, 4, 1, bin);

    int soma_reg_remov = c->nroRegRem - sobrescritos + removidos;  
    fwrite(&soma_reg_remov, 4, 1, bin);
}

/* A função ler_campo recebe um ponteiro para ponteiro de char (char **ptr),
   que aponta para uma string contendo campos separados por espaços.
   Ela retorna uma string alocada dinamicamente contendo o próximo campo lido,
   respeitando aspas e tratando o valor "NULO" como um campo nulo (retornando NULL)
*/
char* ler_campo(char **ptr) {
    char buffer[200];
    int i = 0;

    // Pula espaços
    while (**ptr == ' ') {
        (*ptr)++;
    } 

    // Se estiver entre aspas
    if (**ptr == '"') {
        (*ptr)++; // Pula aspas iniciais
        while (**ptr != '"' && **ptr != '\0' && **ptr != '\r') {
            buffer[i++] = *(*ptr);
            (*ptr)++;
        }
        if (**ptr == '"') (*ptr)++; // Pula aspas finais

    } else {
        // Sem aspas: lê até espaço
        while (**ptr != ' ' && **ptr != '\0' && **ptr != '\n' && **ptr != '\r'){
            buffer[i++] = *(*ptr);
            (*ptr)++;
        }
    }

    buffer[i] = '\0';

    if (strcmp(buffer, "NULO") == 0) {
        return NULL; 
    }
    
    char *campo = malloc(strlen(buffer) + 1);
    strcpy(campo, buffer);
    return campo;
}

/* A função escrever_campos_variaveis pode ser usada para escrever um campo variável a partir do csv,
   ou a partir do campo armazenado na struct (char *ptr_registro). 
   A partir do csv: O ponteiro csv é diferente de NULL; A função lê caractere por caractere até encontrar uma vírgula, 
   quebra de linha ou EOF; O campo só será escrito se o tamanho final for diferente de 0 (tamanho final é sempre incrementado,
   se não for encontrado duas vírgulas (,,) -> campo nulo).
   A partir da struct (usado na função inserir_registro): O ponteiro csv é igual a NULL; o parâmetro tam_insercao é diferente 
   de -1 (indica que o campo não está vazio); a função tam_str(), no arquivo inserir_registros, garante que o tam_insercao será -1 caso campo nulo.
   Retorna o tamanho do campo escrito.
*/
int escrever_campos_variaveis(char *ptr_registro, char keyword, int tam_insercao, FILE *csv, FILE *bin) {
    char c;
    char delimitador = '|';
    int tamanho = 0;

    if (csv != NULL) {
        while ((c = fgetc(csv)) != EOF && c != ',' && c != '\n') {
            if(c == ',') { // Encontrou uma vírgula, preciso verificar se o próximo é uma vírgula também 

                int verificador = fgetc(csv); // Pega o caracter do próximo

                if (verificador == ',') {
                    break;  // Como o cursor está na segunda vírgula, mantém assim e encerra o loop (se o campo está vazio no csv, não escreve nada no binário)

                } else {
                    ungetc(verificador, csv); // Volta o cursor para o início e continua o loop
                    break; // Volta normalmente para ler o campo
                }
            }

            ptr_registro = realloc(ptr_registro, tamanho + 2); // Alocou um espaço
            ptr_registro[tamanho] = c; // O espaço alocado recebe o caracter
            tamanho++;
        }
    }
    
    if (tamanho != 0 || ptr_registro != NULL) { // Se o campo não estiver vazio a escrita no binário será realizada
        if(tam_insercao != -1) { // tam_insercao será diferente de -1, quando na inserção o campo não for nulo
            fwrite(&keyword, 1, 1, bin);
            fwrite(ptr_registro, 1, tam_insercao, bin);
            fwrite(&delimitador, 1, 1, bin);

        } else if (ptr_registro != NULL){
            fwrite(&keyword, 1, 1, bin);
            fwrite(ptr_registro, 1, tamanho, bin); // Esse tamanho é obtido do cálculo do csv
            fwrite(&delimitador, 1, 1, bin);
        }

        tamanho += 2; // Keyword e delimitador
    }
    
    return tamanho;
}

// Printa o conteúdo armazenado na struct, fazendo os devidos castings, juntamente com as descrições.
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

// Printa o conteúdo armazenado na struct, juntamente com as descrições.
void print_variaveis(char *descricao, char *conteudo, int tamanho) {
    if(tamanho != 0 && conteudo != NULL) { // Se não for nulo
        printf("%s: %s\n", descricao, conteudo);

    } else {
        printf("%s: NADA CONSTA\n", descricao);
    }

}

// Exibe o registro com auxílio das funções anteriores.
void exibir(CABECALHO *c, REGISTRO *r, int tam_country, int tam_attack_type, int tam_target_industry, int tam_defense_mechanism) {
    print_fixos(c->descreveIdent, &(r->idAttack), 0);
    print_fixos(c->descreveYear, &(r->year), 0);
    print_variaveis(c->descreveCountry, r->country, tam_country);
    print_variaveis(c->descreveTargetIndustry, r->targetIndustry, tam_target_industry);
    print_variaveis(c->descreveType, r->attackType, tam_attack_type);
    print_fixos(c->descreveFinancialLoss, &(r->financialLoss), 1);
    print_variaveis(c->descreveDefense, r->defenseMechanism, tam_defense_mechanism);
    printf("\n"); // Linha em branco
}

// Funções referentes à busca

/*  A função verifica_criterio recebe cada string dos vetores de string, campos e buscados, verificando
    se o valor (buscado_str) do campo buscado (campo) corresponde com o armazenado na struct, que antes foi lido
    a partir do arquivo de dados. Se corresponder retorna true, caso contrário false.
*/
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

/* A função percorre_registros recebe os vetores de string, campos e buscados, para poder passar por parâmetro para a função
   verifica_criterio, um ponteiro para removidos também é passado (passagem por referência), para ser possível
   incrementar corretamente a quantidade de removidos que será retornada pela função busca.
   A variável op indica a operação que se quer realizar: 0 -> exibir, 1 -> remover e 2 -> atualizar.
   O arquivo é percorrido registro a registro (sendo que os campos fixos e variáveis de cada registro, são armazenados na struct),
   se houver algum lixo eles serão consumidos, a fim de garantir que a próxima busca, ou próximas ações, sejam feitas no lugar correto
   dentro do arquivo. Se todos os critérios de busca forem atendidos a operação desejada (op) pode ser realizada.
   Antes da próxima busca, a memória alocada para os campos variáveis é liberada, para que a o próximo registro buscado seja preenchido
   corretamente na struct. No fim de todas as buscas, há verificações para tratar exceções em caso de falha na busca de acordo com cada operação.
*/
void percorre_registros(char **campos, char **buscados, int criterios, int *removidos, int op, CABECALHO *c,  FILE *bin) {
    REGISTRO *r = calloc(1, sizeof(REGISTRO)); // Zera toda a struct 
    if (r == NULL) return;

    int encontrado = 0;

    // Variáveis necessárias para a atualização
    int quant_alteracoes = 0;
    char entrada[200];

    while(fread(&(r->removido), sizeof(char), 1, bin) == 1) {
        r->country = NULL;
        r->attackType = NULL;
        r->targetIndustry = NULL;
        r->defenseMechanism = NULL;
        
        if(r->removido == '0') { // Se não removido
            fread(&(r->tamRegistro), 4, 1, bin);
            fseek(bin, 8, SEEK_CUR); // Pula campo próx

            fread(&(r->idAttack), 4, 1, bin);
            fread(&(r->year), 4, 1, bin);
            fread(&(r->financialLoss), 4, 1, bin);

            int tamanho_country = ler_campos_variaveis(&(r->country), '1', bin); 
            int tamanho_attack_type= ler_campos_variaveis(&(r->attackType), '2', bin); 
            int tamanho_target_industry = ler_campos_variaveis((&r->targetIndustry), '3', bin); 
            int tamanho_defense_mechanism = ler_campos_variaveis(&(r->defenseMechanism), '4',bin);

            // Se houver lixo precisamos garantir que eles sejam consumidos 
            char cifrao = '$';
            int byte_lido = 0;
            while ((byte_lido = fread(&cifrao, 1, 1, bin)) == 1 && cifrao == '$') { 
                // Apenas consome o '$'
            }
            if (byte_lido != 0) { // Volta 1 byte se fread conseguiu ler algo
                fseek(bin, -1, SEEK_CUR); // Volta 1 byte que o while (cifrao == '$') lerá a mais
            }

            int atende_criterios = 1;
            for(int i = 0; i < criterios; i++) {  // Verifica se todos os critérios foram atendidos
                if(!verifica_criterio(campos[i], buscados[i], r)) {
                    atende_criterios = 0;
                    break;
                }
            }

            if(atende_criterios == 1) {
                encontrado++;

                if (op == 0) { // Indica que quero exibir o registro encontrado
                    exibir(c, r, tamanho_country, tamanho_attack_type, tamanho_target_industry, tamanho_defense_mechanism);

                } else if (op == 1) { // Indica que quero remover logicamente o registro encontrado
                    remover(bin, r, c);
                    (*removidos) += 1;

                } else if(op == 2) { // Indica que quero atualizar o registro encontrado
                    if(encontrado == 1) { // Se a busca encontrou o primeiro registro que atende aos critérios, devemos obter a entrada do usuário
                        scanf("%d", &quant_alteracoes);
                        scanf(" %199[^\n\r]", entrada); // Captura tudo exceto \n e \r
                        getchar(); // Remove o terminador (\n ou \r) residual
                    }
                    entrada_com_alteracoes(r, entrada, quant_alteracoes);
                    atualizar(r, c, bin);
                } 
            }

        }

        // Desalocação de memória dos campos variáveis da struct registro
        free(r->country);
        free(r->attackType);
        free(r->targetIndustry);
        free(r->defenseMechanism);
    }

    if (encontrado == 0 && op == 0) { // Se a busca falhar na operação de exibir os registros encontrados
        printf("Registro inexistente.\n\n");

    } else if (*removidos != 0) { // Necessário para atualizar o topo da lista em caso de remoção
        fseek(bin, 1, SEEK_SET);
        fwrite(&(c->topo), 8, 1, bin); // Atualiza topo da lista no arquivo apenas 1 vez, independentemente da quantidade de buscas bem sucedidas (registros removidos)
    
    } else if (encontrado == 0 && op == 2) { // Se a busca falhar na atualização, a linha com os novos dados deve ser consumida
        scanf(" %199[^\n\r]", entrada); // Captura tudo exceto \n e \r
        getchar(); // Remove o terminador (\n ou \r) residual
    }

    // Desalocação de memória da struct registro
    free(r);
}

/* A função busca captura os critérios de busca que o usuário digitar, armazenando em um vetor de strings,
   campos e buscados, chama a função que percorre todos os registros do arquivo e retorna a quantidade de 
   remoções que ocorreram, esse retorno é necessário para que o cabeçalho seja atualizado corretamente na remoção.
*/
int busca(FILE *bin, int quant_buscas, CABECALHO *c, int op) {
    int removidos = 0; // Variável que será retornada, é necessária para atualizar corretamente o cabeçalho em caso de remoção

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
        
        percorre_registros(campos, buscados, criterios, &removidos, op, c, bin);
        
        if (op == 0) { // Somente necessário em caso de busca para exibir registros encontrados
            printf("**********\n");
        }

        // Desalocação do vetor de strings
        for (int j = 0; j < criterios; j++) {
            free(campos[j]);
            free(buscados[j]);
        }
        free(campos);
        free(buscados);
    }

    return removidos;
}