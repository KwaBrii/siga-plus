
#include <stdio.h>
#include "estudantes.h"
#include "cursos.h"
#include "matriculas.h"
#include "database.h"

// Prototipos das funcoes
void exibirMenu(void);
void limparBuffer(void);
void exibirRelatorios(void);

int main(void) {
    sqlite3 *db = NULL;

    if (conectarBanco(&db) != SQLITE_OK) {
        if (db != NULL) {
            sqlite3_close(db);
        }

        return 1;
    }

    if (criarTabelaEstudantes(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    if (criarTabelaCursos(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    if (criarTabelaMatriculas(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    int opcao = -1;

    do {
        exibirMenu();

        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            if (feof(stdin)) {
                break;
            }

            printf("\nEntrada invalida! Digite um numero.\n");
            limparBuffer();
            opcao = -1;
            continue;
        }

        limparBuffer();

        switch (opcao) {
            case 1:
                gerenciarEstudantes(db);
                break;

            case 2:
                gerenciarCursos(db);
                break;

            case 3:
                gerenciarMatriculas(db);
                break;

            case 4:
                exibirRelatorios();
                break;

            case 0:
                printf("\nEncerrando o SIGA+...\n");
                break;

            default:
                printf("\nOpcao inexistente! Tente novamente.\n");
                break;
        }

    } while (opcao != 0);

    sqlite3_close(db);

    return 0;
}

// Exibe o menu principal
void exibirMenu(void) {
    printf("\n");
    printf("============================================\n");
    printf("                    SIGA+\n");
    printf("    Sistema Integrado de Gestao Academica\n");
    printf("============================================\n");
    printf("1 - Gerenciar estudantes\n");
    printf("2 - Gerenciar cursos\n");
    printf("3 - Gerenciar matriculas\n");
    printf("4 - Relatorios\n");
    printf("0 - Sair\n");
    printf("============================================\n");
}

// Limpa os caracteres restantes da entrada
void limparBuffer(void) {
    int caractere;

    while ((caractere = getchar()) != '\n' &&
           caractere != EOF) {
    }
}

// Modulo de relatorios
void exibirRelatorios(void) {
    printf("\nModulo de relatorios em desenvolvimento.\n");
}