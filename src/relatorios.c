#include <stdio.h>
#include <sqlite3.h>

#include "relatorios.h"
#include "matriculas.h"

// ========================================
// FUNCOES AUXILIARES
// ========================================

// Limpa os caracteres restantes da entrada
static void limparBufferRelatorios(void) {
    int caractere;

    while ((caractere = getchar()) != '\n' &&
           caractere != EOF) {
    }
}

// Prepara uma consulta SQL
static int prepararConsulta(
    sqlite3 *db,
    const char *sql,
    sqlite3_stmt **stmt,
    const char *descricao
) {
    int resultado = sqlite3_prepare_v2(
        db,
        sql,
        -1,
        stmt,
        NULL
    );

    if (resultado != SQLITE_OK) {
        printf(
            "\nErro ao preparar %s: %s\n",
            descricao,
            sqlite3_errmsg(db)
        );

        return 0;
    }

    return 1;
}

// ========================================
// 1. RELATORIO DE ESTUDANTES
// ========================================

static void relatorioEstudantes(sqlite3 *db) {

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT "
        "COUNT(*), "
        "COALESCE(SUM(CASE WHEN ativo = 1 THEN 1 ELSE 0 END), 0), "
        "COALESCE(SUM(CASE WHEN ativo = 0 THEN 1 ELSE 0 END), 0) "
        "FROM estudantes;";

    if (!prepararConsulta(
        db,
        sql,
        &stmt,
        "relatorio de estudantes"
    )) {
        return;
    }

    int resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {

        sqlite3_int64 total = sqlite3_column_int64(stmt, 0);
        sqlite3_int64 ativos = sqlite3_column_int64(stmt, 1);
        sqlite3_int64 inativos = sqlite3_column_int64(stmt, 2);

        printf("\n========================================\n");
        printf("       RELATORIO DE ESTUDANTES\n");
        printf("========================================\n");

        printf("Total de estudantes: %lld\n", (long long)total);
        printf("Estudantes ativos:   %lld\n", (long long)ativos);
        printf("Estudantes inativos: %lld\n", (long long)inativos);

        printf("========================================\n");

    } else {
        printf(
            "\nErro ao consultar estudantes: %s\n",
            sqlite3_errmsg(db)
        );
    }

    sqlite3_finalize(stmt);
}

// ========================================
// 2. RELATORIO DE CURSOS
// ========================================

static void relatorioCursos(sqlite3 *db) {

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT "
        "COUNT(*), "
        "COALESCE(SUM(CASE WHEN situacao = 'ATIVO' THEN 1 ELSE 0 END), 0), "
        "COALESCE(SUM(CASE WHEN situacao = 'INATIVO' THEN 1 ELSE 0 END), 0) "
        "FROM cursos;";

    if (!prepararConsulta(
        db,
        sql,
        &stmt,
        "relatorio de cursos"
    )) {
        return;
    }

    int resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {

        sqlite3_int64 total = sqlite3_column_int64(stmt, 0);
        sqlite3_int64 ativos = sqlite3_column_int64(stmt, 1);
        sqlite3_int64 inativos = sqlite3_column_int64(stmt, 2);

        printf("\n========================================\n");
        printf("          RELATORIO DE CURSOS\n");
        printf("========================================\n");

        printf("Total de cursos: %lld\n", (long long)total);
        printf("Cursos ativos:   %lld\n", (long long)ativos);
        printf("Cursos inativos: %lld\n", (long long)inativos);

        printf("========================================\n");

    } else {
        printf(
            "\nErro ao consultar cursos: %s\n",
            sqlite3_errmsg(db)
        );
    }

    sqlite3_finalize(stmt);
}

// ========================================
// 3. RELATORIO DE MATRICULAS
// ========================================

static void relatorioMatriculas(sqlite3 *db) {

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT "
        "COUNT(*), "
        "COALESCE(SUM(CASE WHEN status = 'PENDENTE' THEN 1 ELSE 0 END), 0), "
        "COALESCE(SUM(CASE WHEN status = 'ATIVA' THEN 1 ELSE 0 END), 0), "
        "COALESCE(SUM(CASE WHEN status = 'CANCELADA' THEN 1 ELSE 0 END), 0), "
        "COALESCE(SUM(CASE WHEN status = 'CONCLUIDA' THEN 1 ELSE 0 END), 0), "
        "COALESCE(SUM(CASE WHEN status = 'EXPIRADA' THEN 1 ELSE 0 END), 0) "
        "FROM matriculas;";

    if (!prepararConsulta(
        db,
        sql,
        &stmt,
        "relatorio de matriculas"
    )) {
        return;
    }

    int resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {

        sqlite3_int64 total = sqlite3_column_int64(stmt, 0);
        sqlite3_int64 pendentes = sqlite3_column_int64(stmt, 1);
        sqlite3_int64 ativas = sqlite3_column_int64(stmt, 2);
        sqlite3_int64 canceladas = sqlite3_column_int64(stmt, 3);
        sqlite3_int64 concluidas = sqlite3_column_int64(stmt, 4);
        sqlite3_int64 expiradas = sqlite3_column_int64(stmt, 5);

        printf("\n========================================\n");
        printf("        RELATORIO DE MATRICULAS\n");
        printf("========================================\n");

        printf("Total de matriculas: %lld\n\n", (long long)total);

        printf("Pendentes:   %lld\n", (long long)pendentes);
        printf("Ativas:      %lld\n", (long long)ativas);
        printf("Canceladas:  %lld\n", (long long)canceladas);
        printf("Concluidas:  %lld\n", (long long)concluidas);
        printf("Expiradas:   %lld\n", (long long)expiradas);

        printf("========================================\n");

    } else {
        printf(
            "\nErro ao consultar matriculas: %s\n",
            sqlite3_errmsg(db)
        );
    }

    sqlite3_finalize(stmt);
}

// ========================================
// 4. OCUPACAO DE VAGAS POR CURSO
// ========================================

static void relatorioOcupacao(sqlite3 *db) {

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT "
        "c.codigo, "
        "c.nome, "
        "c.situacao, "
        "c.capacidade_vagas, "
        "COUNT(m.id_matricula) "
        "FROM cursos AS c "
        "LEFT JOIN matriculas AS m "
        "ON c.id_curso = m.id_curso "
        "AND m.status IN ('PENDENTE', 'ATIVA') "
        "GROUP BY "
        "c.id_curso, "
        "c.codigo, "
        "c.nome, "
        "c.situacao, "
        "c.capacidade_vagas "
        "ORDER BY c.nome COLLATE NOCASE;";

    if (!prepararConsulta(
        db,
        sql,
        &stmt,
        "relatorio de ocupacao de vagas"
    )) {
        return;
    }

    printf("\n========================================\n");
    printf("       OCUPACAO DE VAGAS POR CURSO\n");
    printf("========================================\n");

    int resultado;
    int encontrou = 0;

    while ((resultado = sqlite3_step(stmt)) == SQLITE_ROW) {

        encontrou = 1;

        const unsigned char *codigo = sqlite3_column_text(stmt, 0);
        const unsigned char *nome = sqlite3_column_text(stmt, 1);
        const unsigned char *situacao = sqlite3_column_text(stmt, 2);

        int capacidade = sqlite3_column_int(stmt, 3);

        sqlite3_int64 ocupadas = sqlite3_column_int64(stmt, 4);

        sqlite3_int64 disponiveis = capacidade - ocupadas;

        double percentual = 0.0;

        if (capacidade > 0) {
            percentual = (double)ocupadas * 100.0 / capacidade;
        }

        printf("\nCodigo: %s\n",
            codigo ? (const char *)codigo : "-");

        printf("Curso: %s\n",
            nome ? (const char *)nome : "-");

        printf("Situacao: %s\n",
            situacao ? (const char *)situacao : "-");

        printf("Capacidade total: %d\n", capacidade);

        printf("Vagas ocupadas: %lld\n", (long long)ocupadas);

        if (disponiveis >= 0) {
            printf("Vagas disponiveis: %lld\n", (long long)disponiveis);
        } else {
            printf("Vagas disponiveis: 0\n");
            printf(
                "ATENCAO: ocupacao excede a capacidade em %lld vaga(s).\n",
                (long long)(-disponiveis)
            );
        }

        printf("Ocupacao: %.1f%%\n", percentual);

        printf("----------------------------------------\n");
    }

    if (!encontrou) {
        printf("\nNenhum curso cadastrado.\n");
    }

    if (resultado != SQLITE_DONE) {
        printf(
            "\nErro durante a consulta: %s\n",
            sqlite3_errmsg(db)
        );
    }

    sqlite3_finalize(stmt);
}

// ========================================
// MENU DE RELATORIOS
// ========================================

void exibirRelatorios(sqlite3 *db) {

    // Atualiza matriculas pendentes que ja expiraram
    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        printf("\nNao foi possivel atualizar as matriculas expiradas.\n");
        return;
    }

    int opcao = -1;

    do {

        printf("\n========================================\n");
        printf("             RELATORIOS\n");
        printf("========================================\n");

        printf("1 - Relatorio de estudantes\n");
        printf("2 - Relatorio de cursos\n");
        printf("3 - Relatorio de matriculas\n");
        printf("4 - Ocupacao de vagas por curso\n");
        printf("0 - Voltar ao menu principal\n");

        printf("========================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {

            if (feof(stdin)) {
                break;
            }

            printf("\nEntrada invalida! Digite um numero.\n");

            limparBufferRelatorios();

            opcao = -1;

            continue;
        }

        limparBufferRelatorios();

        switch (opcao) {

            case 1:
                relatorioEstudantes(db);
                break;

            case 2:
                relatorioCursos(db);
                break;

            case 3:
                relatorioMatriculas(db);
                break;

            case 4:
                relatorioOcupacao(db);
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
