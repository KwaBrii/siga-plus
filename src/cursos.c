#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "cursos.h"

// ========================================
// FUNCOES AUXILIARES
// ========================================

// Le uma linha de texto
static int lerLinha(const char *mensagem, char *destino, size_t tamanho) {
    printf("%s", mensagem);

    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        return 0;
    }

    size_t tamanhoLido = strcspn(destino, "\r\n");

    if (destino[tamanhoLido] == '\n' ||
        destino[tamanhoLido] == '\r') {
        destino[tamanhoLido] = '\0';
    } else {
        int caractere;

        while ((caractere = getchar()) != '\n' &&
               caractere != EOF) {
        }
    }

    return 1;
}

// Le um campo de texto
static int lerCampo(
    const char *mensagem,
    char *campo,
    size_t tamanho,
    int obrigatorio
) {
    while (1) {
        if (!lerLinha(mensagem, campo, tamanho)) {
            return 0;
        }

        if (obrigatorio && campo[0] == '\0') {
            printf("Este campo e obrigatorio!\n");
            continue;
        }

        return 1;
    }
}

// Le um numero inteiro positivo
static int lerInteiroPositivo(
    const char *mensagem,
    int *valor,
    int obrigatorio,
    int *informado
) {
    char entrada[100];
    char *fim;
    long numero;

    while (1) {
        if (!lerLinha(mensagem, entrada, sizeof(entrada))) {
            return 0;
        }

        if (entrada[0] == '\0' && !obrigatorio) {
            *informado = 0;
            *valor = 0;
            return 1;
        }

        errno = 0;
        numero = strtol(entrada, &fim, 10);

        while (*fim == ' ' || *fim == '\t') {
            fim++;
        }

        if (errno == ERANGE ||
            fim == entrada ||
            *fim != '\0' ||
            numero <= 0 ||
            numero > INT_MAX) {

            printf("Digite um numero inteiro positivo valido.\n");
            continue;
        }

        *valor = (int)numero;
        *informado = 1;

        return 1;
    }
}

// Le uma opcao do menu
static int lerOpcao(int *opcao) {
    char entrada[50];
    char *fim;
    long numero;

    while (1) {
        if (!lerLinha("Escolha uma opcao: ", entrada, sizeof(entrada))) {
            return 0;
        }

        errno = 0;
        numero = strtol(entrada, &fim, 10);

        while (*fim == ' ' || *fim == '\t') {
            fim++;
        }

        if (errno == ERANGE ||
            fim == entrada ||
            *fim != '\0' ||
            numero < 0 ||
            numero > INT_MAX) {

            printf("Opcao invalida! Digite um numero.\n");
            continue;
        }

        *opcao = (int)numero;
        return 1;
    }
}

// Solicita confirmacao de uma operacao
static int lerConfirmacao(int *confirmacao) {
    char entrada[32];
    char *fim;
    long numero;

    while (1) {
        if (!lerLinha(
            "Confirmar? (1 - Sim / 0 - Nao): ",
            entrada,
            sizeof(entrada)
        )) {
            return 0;
        }

        errno = 0;
        numero = strtol(entrada, &fim, 10);

        while (*fim == ' ' || *fim == '\t') {
            fim++;
        }

        if (errno == 0 &&
            fim != entrada &&
            *fim == '\0' &&
            (numero == 0 || numero == 1)) {

            *confirmacao = (int)numero;
            return 1;
        }

        printf("Digite 1 para confirmar ou 0 para cancelar.\n");
    }
}

// ========================================
// CADASTRAR CURSO
// ========================================

static void cadastrarCurso(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;

    char codigo[31];
    char nome[121];

    int cargaHoraria;
    int capacidadeVagas;
    int informado;

    printf("\n========== CADASTRAR CURSO ==========\n");

    if (!lerCampo("Codigo do curso: ", codigo, sizeof(codigo), 1)) {
        return;
    }

    if (!lerCampo("Nome do curso: ", nome, sizeof(nome), 1)) {
        return;
    }

    if (!lerInteiroPositivo(
        "Carga horaria (horas): ",
        &cargaHoraria,
        1,
        &informado
    )) {
        return;
    }

    if (!lerInteiroPositivo(
        "Capacidade de vagas: ",
        &capacidadeVagas,
        1,
        &informado
    )) {
        return;
    }

    const char *sql =
        "INSERT INTO cursos "
        "(codigo, nome, carga_horaria, capacidade_vagas) "
        "VALUES (?, ?, ?, ?);";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar cadastro: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, nome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, cargaHoraria);
    sqlite3_bind_int(stmt, 4, capacidadeVagas);

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_DONE) {
        printf("\nCurso cadastrado com sucesso!\n");
    } else {
        printf("\nErro ao cadastrar curso: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// ========================================
// LISTAR CURSOS
// ========================================

static void listarCursos(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT id_curso, codigo, nome, carga_horaria, "
        "capacidade_vagas, situacao "
        "FROM cursos ORDER BY nome;";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao consultar cursos: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n============================== CURSOS ==============================\n");
    printf("%-5s | %-12s | %-28s | %-10s | %-10s | %-10s\n",
           "ID", "CODIGO", "NOME", "CARGA H.", "VAGAS", "SITUACAO");

    printf("---------------------------------------------------------------------\n");

    int encontrou = 0;

    while ((resultado = sqlite3_step(stmt)) == SQLITE_ROW) {
        encontrou = 1;

        printf(
            "%-5d | %-12s | %-28s | %-10d | %-10d | %-10s\n",
            sqlite3_column_int(stmt, 0),
            (const char *)sqlite3_column_text(stmt, 1),
            (const char *)sqlite3_column_text(stmt, 2),
            sqlite3_column_int(stmt, 3),
            sqlite3_column_int(stmt, 4),
            (const char *)sqlite3_column_text(stmt, 5)
        );
    }

    if (!encontrou) {
        printf("Nenhum curso cadastrado.\n");
    }

    printf("=====================================================================\n");

    if (resultado != SQLITE_DONE) {
        printf("Erro ao listar cursos: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// ========================================
// BUSCAR CURSO
// ========================================

static void buscarCurso(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;
    char codigo[31];

    printf("\n========== BUSCAR CURSO ==========\n");

    if (!lerCampo("Codigo do curso: ", codigo, sizeof(codigo), 1)) {
        return;
    }

    const char *sql =
        "SELECT id_curso, codigo, nome, carga_horaria, "
        "capacidade_vagas, situacao "
        "FROM cursos WHERE codigo = ?;";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro na busca: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {
        printf("\nCurso encontrado!\n");
        printf("ID: %d\n", sqlite3_column_int(stmt, 0));
        printf("Codigo: %s\n", sqlite3_column_text(stmt, 1));
        printf("Nome: %s\n", sqlite3_column_text(stmt, 2));
        printf("Carga horaria: %d horas\n", sqlite3_column_int(stmt, 3));
        printf("Capacidade: %d vagas\n", sqlite3_column_int(stmt, 4));
        printf("Situacao: %s\n", sqlite3_column_text(stmt, 5));
    } else if (resultado == SQLITE_DONE) {
        printf("\nCurso nao encontrado.\n");
    } else {
        printf("\nErro ao buscar curso: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// ========================================
// ATUALIZAR CURSO
// ========================================

static void atualizarCurso(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;

    char codigo[31];
    char nome[121];

    int cargaHoraria;
    int capacidadeVagas;

    int alterarCarga;
    int alterarCapacidade;

    printf("\n========== ATUALIZAR CURSO ==========\n");

    if (!lerCampo("Codigo do curso: ", codigo, sizeof(codigo), 1)) {
        return;
    }

    printf("\nDeixe os campos vazios para manter os valores atuais.\n\n");

    if (!lerCampo("Novo nome: ", nome, sizeof(nome), 0)) {
        return;
    }

    if (!lerInteiroPositivo(
        "Nova carga horaria: ",
        &cargaHoraria,
        0,
        &alterarCarga
    )) {
        return;
    }

    if (!lerInteiroPositivo(
        "Nova capacidade de vagas: ",
        &capacidadeVagas,
        0,
        &alterarCapacidade
    )) {
        return;
    }

    if (nome[0] == '\0' && !alterarCarga && !alterarCapacidade) {
        printf("\nNenhuma alteracao foi informada.\n");
        return;
    }

    const char *sql =
        "UPDATE cursos SET "
        "nome = COALESCE(NULLIF(?, ''), nome), "
        "carga_horaria = CASE WHEN ? = 1 THEN ? ELSE carga_horaria END, "
        "capacidade_vagas = CASE WHEN ? = 1 THEN ? ELSE capacidade_vagas END "
        "WHERE codigo = ? AND situacao = 'ATIVO';";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar atualizacao: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, alterarCarga);
    sqlite3_bind_int(stmt, 3, cargaHoraria);
    sqlite3_bind_int(stmt, 4, alterarCapacidade);
    sqlite3_bind_int(stmt, 5, capacidadeVagas);
    sqlite3_bind_text(stmt, 6, codigo, -1, SQLITE_TRANSIENT);

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_DONE) {
        if (sqlite3_changes(db) > 0) {
            printf("\nCurso atualizado com sucesso!\n");
        } else {
            printf("\nNenhum curso ativo encontrado com esse codigo.\n");
        }
    } else {
        printf("\nErro ao atualizar curso: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// ========================================
// INATIVAR CURSO
// ========================================

static void inativarCurso(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;

    char codigo[31];
    int confirmacao;

    printf("\n========== INATIVAR CURSO ==========\n");

    if (!lerCampo("Codigo do curso: ", codigo, sizeof(codigo), 1)) {
        return;
    }

    if (!lerConfirmacao(&confirmacao)) {
        return;
    }

    if (confirmacao == 0) {
        printf("\nOperacao cancelada.\n");
        return;
    }

    const char *sql =
        "UPDATE cursos SET situacao = 'INATIVO' "
        "WHERE codigo = ? AND situacao = 'ATIVO';";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar inativacao: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_DONE) {
        if (sqlite3_changes(db) > 0) {
            printf("\nCurso inativado com sucesso!\n");
            printf("O registro foi preservado no banco de dados.\n");
        } else {
            printf("\nNenhum curso ativo encontrado com esse codigo.\n");
        }
    } else {
        printf("\nErro ao inativar curso: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// ========================================
// MENU DE GERENCIAMENTO DE CURSOS
// ========================================

void gerenciarCursos(sqlite3 *db) {
    int opcao;

    do {
        printf("\n====================================\n");
        printf("       GERENCIAMENTO DE CURSOS\n");
        printf("====================================\n");
        printf("1 - Cadastrar curso\n");
        printf("2 - Listar cursos\n");
        printf("3 - Buscar curso\n");
        printf("4 - Atualizar curso\n");
        printf("5 - Inativar curso\n");
        printf("0 - Voltar ao menu principal\n");
        printf("====================================\n");

        if (!lerOpcao(&opcao)) {
            printf("\nEntrada encerrada.\n");
            return;
        }

        switch (opcao) {
            case 1:
                cadastrarCurso(db);
                break;

            case 2:
                listarCursos(db);
                break;

            case 3:
                buscarCurso(db);
                break;

            case 4:
                atualizarCurso(db);
                break;

            case 5:
                inativarCurso(db);
                break;

            case 0:
                printf("\nVoltando ao menu principal...\n");
                break;

            default:
                printf("\nOpcao inexistente! Tente novamente.\n");
                break;
        }

    } while (opcao != 0);
}
