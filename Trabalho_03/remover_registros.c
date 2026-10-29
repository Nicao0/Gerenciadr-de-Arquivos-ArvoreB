#include "remover_registros.h"
#include "funcoes_auxiliares.h"
#include "funcao-fornecida.h"

/* A função remover, chamada a partir da busca, quando um registro atende aos critérios, considera que o ponteiro
   do arquivo está no fim do registro a ser removido (isso é garantido pela função busca), dessa forma, volta para
   o início do registro, marca-o como removido, atualiza o seu campo prox para receber o topo da lista e atualiza o novo 
   topo, que deve ser o offset desse registro removido.
   Por fim, o ponteiro do arquivo é recolocado no fim desse registro, para não interferir na pŕoxima busca.
   O topo da lista só é atualizado no arquivo de dados, ao fim de todas as buscas, evitando fseeks desnecessários,
   essa atualização do topo no arquivo é feita na função busca.
*/
void remover(FILE *bin, REGISTRO *r, CABECALHO *c) {
    fseek(bin, -((r->tamRegistro) + 5), SEEK_CUR); // Volta para o início do registro, 5 (tamanhoRegistro e removido)

    long int posicao = ftell(bin); // Obtém a posição do 1° byte do registro

    r->removido = '1';
    fwrite(&(r->removido), 1, 1, bin); // Marca como logicamente removido

    r->prox = c->topo; // r->prox recebe o topo da lista
    fseek(bin, 4, SEEK_CUR); // Pula tamRegistro que deve permanecer igual
    fwrite(&(r->prox), 8, 1, bin); // Escreve o anterior da lista
    
    c->topo = posicao; // Armazena novo topo (será escrito no arquivo quando todas as buscas cessarem (na função busca em funções auxiliares), a fim de evitar fseeks desnecessários)

    // Fseek para não interferir na próxima busca (while da função percorre_registros), -8 pois da posição atual porque queremos avançar para o fim e r->tamRegistro é o 
    // prox + fixos + variáveis, como o cursor já passou pelo prox, só avançamos os bytes dos campos fixos e variáveis
    fseek(bin, (r->tamRegistro - 8), SEEK_CUR); 
}

/* A função remover_registro é a responsável por toda a lógica de remoção. Primeiro ela lê as entradas do usuário, abre o arquivo e
   marca o status como '0', pois o arquivo está aberto para escrita. Depois disso, é chamada a função busca que obtém os critérios de busca do usuário
   e chama a função percorre_registros que percorre o arquivo, registro por registro, até encontrar um que tenha os critérios de busca.
   Após encontrar, é chamada a função remover, que marca o registro como removido e atualiza a lista para outras operações.
   Por fim, o cabeçalho é atualizado e o status alterado para '1', indicando que o arquivo está consistente.
*/
void remover_registros() {
    char arquivo[20];
    int quant_buscas;
    
    scanf("%s %d", arquivo,  &quant_buscas);

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
    fwrite(&(c->status), 1, 1, bin);

    fread(&(c->topo), 8, 1, bin);

    int removidos = busca(bin, quant_buscas, c, 1, NULL); // Implementação em funções auxiliares
    // A função busca já chama a função remover (acima)

    fseek(bin, 0, SEEK_END); // Necessário para atualizar corretamente o cabeçalho, com o último byte
    // atualiza_cabecalho já altera o status de volta a '1' (consistente)
    atualiza_cabecalho(0, removidos, 0, ftell(bin), c, bin);
    
    fclose(bin); // Fecha arquivo
    binarioNaTela(arquivo);

    free(c); // Desaloca struct

}