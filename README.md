# Gerenciador de Arquivos e Engine de Armazenamento (C)

Este repositório contém a implementação em C de uma engine de armazenamento de baixo nível, desenvolvida para simular o funcionamento interno de um sistema de gerenciamento de dados relacional (SGBD). O projeto aplica algoritmos e estruturas de dados avançadas para a manipulação direta e eficiente de arquivos binários em disco.

---

## ⚙️ Arquitetura e Funcionalidades

O sistema implementa do zero a manipulação de registros binários, focando na otimização do espaço em disco e na velocidade de acesso:

*   **Manipulação de Registros (CRUD):** Leitura, inserção, busca parametrizada, atualização e remoção lógica e física de registros diretamente em arquivos `.bin`.
*   **Reaproveitamento de Espaço:** Algoritmo **First Fit** para a gestão de espaço livre em disco, garantindo o reuso otimizado de lacunas deixadas por registros deletados.
*   **Indexação por Árvore-B:** Estruturação e manutenção de um índice em Árvore-B (B-tree) desacoplado do arquivo de dados principal, permitindo inserções, remoções e buscas com complexidade logarítmica (O(log n)) em grandes massas de dados.

---

## 🚀 Estrutura do Projeto e Execução

O código foi estruturado de forma modularizada em C, separando o controle da Árvore-B (`criar_arvB.c`, `remover_arvB.c`) dos módulos de recuperação e atualização dos registros no arquivo de dados.

Para compilar o projeto, navegue até a pasta do código-fonte e execute:

```bash
make
```
Para rodar a aplicação:
```bash
./programaTrab
