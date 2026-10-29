#include "buscar_arvB.h"
#include "funcoes_arvB.h"
#include "funcoes_auxiliares.h"
#include "atualizar_registros.h"
#include "funcao-fornecida.h"

/*  A função buscar_registro_arvB é chamada pela função busca (do arquivo de dados), quando existe árvore B e, que ao menos, um critério de busca
    seja o idAttack. Assim a função buscar (da árvore B), retornará o offset em que está aquela chave no arquivo de dados e, se
    existirem mais critérios de busca, esses também serão verificados.
    A função também é usada na atualização (que segue a mesma lógica de busca), o parâmetro op (= 2 para atualização), auxilia nisso.
*/
void buscar_registro_arvB(int chave, char **campos, char **buscados, int criterios, int op, CABECALHO *c, FILE *bin, FILE *arvB) {
    int atende_criterios = 1;

    // Variáveis necessárias na atualização (funcionalidade 11)
    int quant_alteracoes = 0;
    char entrada[200];
    
    CABECALHO_ARVORE *c_arv = (CABECALHO_ARVORE *) malloc(sizeof(CABECALHO_ARVORE));
    if(c_arv == NULL) {
        return;
    }

    REGISTRO *r = (REGISTRO *) malloc(sizeof(REGISTRO));
    if(r == NULL) {
        return;
    }

    fseek(arvB, 0, SEEK_SET);
    ler_cabecalho(0, c_arv, arvB); 

    long int offset = buscar(chave, c_arv->noRaiz, -1, arvB); // Começa a busca pela raiz

    if(offset != -1) { // Encontrou
        fseek(bin, offset, SEEK_SET); // Acesso direto ao registro

        fread(&(r->removido), 1, 1, bin);
        fread(&(r->tamRegistro), 4, 1, bin);
        fseek(bin, 8, SEEK_CUR); // Pula campo próx
        fread(&(r->idAttack), 4, 1, bin);
        fread(&(r->year), 4, 1, bin);
        fread(&(r->financialLoss), 4, 1, bin);

        int tamanho_country = ler_campos_variaveis(&(r->country), '1', bin); 
        int tamanho_attack_type= ler_campos_variaveis(&(r->attackType), '2', bin); 
        int tamanho_target_industry = ler_campos_variaveis((&r->targetIndustry), '3', bin); 
        int tamanho_defense_mechanism = ler_campos_variaveis(&(r->defenseMechanism), '4',bin);

        // Verificar se os outros critérios de busca são satisfeitos
        
        for(int i = 0; i < criterios; i++) {  
            if(!verifica_criterio(campos[i], buscados[i], r)) {
                atende_criterios = 0;
                break;
            }
        }

        if(atende_criterios == 1 && op == 0) { // Indica que está na funcionalidade da busca
            exibir(c, r, tamanho_country, tamanho_attack_type, tamanho_target_industry, tamanho_defense_mechanism);
    
        } else if(atende_criterios == 1 && op == 2) { // Indica que está na funcionalidade da atualização
            scanf("%d", &quant_alteracoes);
            scanf(" %199[^\n\r]", entrada); // Captura tudo exceto \n e \r
            getchar(); // Remove o terminador (\n ou \r) residual
        
            entrada_com_alteracoes(r, entrada, quant_alteracoes);
            atualizar(r, c, bin, arvB);

        } else {
            if(op == 0) {
                printf("Registro inexistente.\n\n");
            }
        }

        if(atende_criterios == 0 && op == 2) { // Se a busca falhar na atualização, a linha com os novos dados deve ser consumida
            scanf(" %199[^\n\r]", entrada); // Captura tudo exceto \n e \r
            getchar(); // Remove o terminador (\n ou \r) residual
        }

    } else { // Não encontrou
        if(op == 0) {
            printf("Registro inexistente.\n\n");

        } else if(op == 2) { // Se a busca falhar na atualização, a linha com os novos dados deve ser consumida
            scanf(" %199[^\n\r]", entrada); // Captura tudo exceto \n e \r
            getchar(); // Remove o terminador (\n ou \r) residual
        }
    }

    free(c_arv); // Desalocações
    c_arv = NULL;
    free(r);
    r = NULL;
}

/*  A função buscar_arvore abre os arquivos somente para leitura, usa a função descricoes_dos_campos e realiza a busca,
    a função busca foi alterada para diferenciar os casos que a busca é feita na árvore e quando é feita percorrendo sequencialmente
    os registros do arquivo de dados.
*/
void buscar_arvore() {
    int quant_buscas;
    char arquivo_dados[20];
    char arquivo_arvB[20];
    
    scanf("%s %s %d", arquivo_dados,  arquivo_arvB, &quant_buscas);

    FILE *bin = fopen(arquivo_dados, "rb"); // Leitura
    if(bin == NULL) {
        print_erro();
        return;
    }

    FILE *arvB = fopen(arquivo_arvB, "rb"); // Leitura
    if(arvB == NULL) {
        print_erro();
        return;
    }
    
    CABECALHO *c = (CABECALHO *) malloc(sizeof(CABECALHO));
    if(c == NULL) {
        return;
    }

    descricoes_dos_campos(c, bin);

    busca(bin, quant_buscas, c, 0, arvB);

    // Fecha arquivos
    fclose(bin);
    fclose(arvB);

    free(c); // Desalocação
    c = NULL;
}