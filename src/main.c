
#include <stdio.h>
#include <string.h>

#include "estudantes.h"
#include "cursos.h"
#include "matriculas.h"
#include "database.h"
#include "auth.h"

// Prototipos das funcoes
void exibirMenu(const char *perfil);
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

    if (criarTabelaEstudantes(db) != SQLITE_OK ||
        criarTabelaCursos(db) != SQLITE_OK ||
        criarTabelaMatriculas(db) != SQLITE_OK ||
        criarTabelaUsuarios(db) != SQLITE_OK) {

        sqlite3_close(db);
        return 1;
    }

    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    SessaoUsuario sessao;

    if (!iniciarSessao(db, &sessao)) {
        sqlite3_close(db);
        return 1;
    }

    int opcao = -1;

    do {
        exibirMenu(sessao.perfil);

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

        int administrativo =
            strcmp(sessao.perfil, "ADMINISTRATIVO") == 0;

        switch (opcao) {
            case 1:
                if (administrativo) {
                    gerenciarEstudantes(db);
                } else {
                    printf("\nAcesso negado.\n");
                }
                break;

            case 2:
                if (administrativo) {
                    gerenciarCursos(db);
                } else {
                    printf("\nAcesso negado.\n");
                }
                break;

            case 3:
                gerenciarMatriculas(db, sessao.perfil);
                break;

            case 4:
                if (administrativo) {
                    gerenciarUsuarios(db, &sessao);
                } else {
                    printf("\nAcesso negado.\n");
                }
                break;

            case 5:
                if (administrativo) {
                    exibirRelatorios();
                } else {
                    printf("\nAcesso negado.\n");
                }
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

// ========================================
// MENU PRINCIPAL
// ========================================

void exibirMenu(const char *perfil) {
    int administrativo =
        strcmp(perfil, "ADMINISTRATIVO") == 0;

    printf("\n");
    printf("============================================\n");
    printf("                    SIGA+\n");
    printf("    Sistema Integrado de Gestao Academica\n");
    printf("============================================\n");

    printf("Usuario: %s\n", perfil);
    printf("--------------------------------------------\n");

    if (administrativo) {
        printf("1 - Gerenciar estudantes\n");
        printf("2 - Gerenciar cursos\n");
        printf("3 - Gerenciar matriculas\n");
        printf("4 - Gerenciar usuarios\n");
        printf("5 - Relatorios\n");
    } else {
        printf("3 - Consultar e concluir matriculas\n");
    }

    printf("0 - Sair\n");
    printf("============================================\n");
}

// ========================================
// LIMPAR BUFFER
// ========================================

void limparBuffer(void) {
    int caractere;

    while ((caractere = getchar()) != '\n' &&
           caractere != EOF) {
    }
}

// ========================================
// RELATORIOS
// ========================================

void exibirRelatorios(void) {
    printf("\nModulo de relatorios em desenvolvimento.\n");
}