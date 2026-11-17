#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "matriculas.h"

// ========================================
// FUNCOES AUXILIARES
// ========================================

// Le uma linha de texto
static int lerLinha(
    const char *mensagem,
    char *destino,
    size_t tamanho
) {
    printf("%s", mensagem);

    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        return 0;
    }

    size_t fim = strcspn(destino, "\r\n");

    if (destino[fim] == '\0' && !feof(stdin)) {
        int caractere;

        while ((caractere = getchar()) != '\n' &&
               caractere != EOF) {
        }
    }

    destino[fim] = '\0';

    return 1;
}

// Le um campo obrigatorio ou opcional
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

// Le um numero inteiro dentro de um intervalo
static int lerInteiro(
    const char *mensagem,
    int minimo,
    int maximo,
    int *valor
) {
    char entrada[80];
    char *fim;
    long numero;

    while (1) {
        if (!lerLinha(mensagem, entrada, sizeof(entrada))) {
            return 0;
        }

        errno = 0;
        numero = strtol(entrada, &fim, 10);

        while (*fim == ' ' || *fim == '\t') {
            fim++;
        }

        if (
            errno == ERANGE ||
            fim == entrada ||
            *fim != '\0' ||
            numero < minimo ||
            numero > maximo
        ) {
            printf(
                "Digite um numero entre %d e %d.\n",
                minimo,
                maximo
            );

            continue;
        }

        *valor = (int)numero;
        return 1;
    }
}

// Inicia uma transacao no banco
static int iniciarTransacao(sqlite3 *db) {
    int resultado = sqlite3_exec(
        db,
        "BEGIN IMMEDIATE;",
        NULL,
        NULL,
        NULL
    );

    if (resultado != SQLITE_OK) {
        printf(
            "Erro ao iniciar transacao: %s\n",
            sqlite3_errmsg(db)
        );

        return 0;
    }

    return 1;
}

// Desfaz uma transacao
static void cancelarTransacao(sqlite3 *db) {
    sqlite3_exec(
        db,
        "ROLLBACK;",
        NULL,
        NULL,
        NULL
    );
}

// Confirma uma transacao
static int confirmarTransacao(sqlite3 *db) {
    int resultado = sqlite3_exec(
        db,
        "COMMIT;",
        NULL,
        NULL,
        NULL
    );

    if (resultado != SQLITE_OK) {
        printf(
            "Erro ao confirmar transacao: %s\n",
            sqlite3_errmsg(db)
        );

        cancelarTransacao(db);
        return 0;
    }

    return 1;
}

// ========================================
// CONTROLE DE EXPIRACAO
// ========================================

// Atualiza matriculas pendentes vencidas
int atualizarMatriculasExpiradas(sqlite3 *db) {
    const char *sql =
        "UPDATE matriculas "
        "SET status = 'EXPIRADA' "
        "WHERE status = 'PENDENTE' "
        "AND datetime(expira_em) <= datetime('now');";

    int resultado = sqlite3_exec(
        db,
        sql,
        NULL,
        NULL,
        NULL
    );

    if (resultado != SQLITE_OK) {
        fprintf(
            stderr,
            "Erro ao verificar expiracao: %s\n",
            sqlite3_errmsg(db)
        );
    }

    return resultado;
}

// ========================================
// CONSULTAS AUXILIARES
// ========================================

// Procura um estudante ativo pelo codigo
static int obterEstudanteAtivo(
    sqlite3 *db,
    const char *codigo,
    int *idEstudante
) {
    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT id_estudante "
        "FROM estudantes "
        "WHERE codigo = ? AND ativo = 1;";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao consultar estudante: %s\n",
               sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(
        stmt, 1, codigo, -1, SQLITE_TRANSIENT
    );

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {
        *idEstudante = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return 1;
    }

    if (resultado != SQLITE_DONE) {
        printf("Erro ao consultar estudante: %s\n",
               sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

// Procura um curso ativo pelo codigo
static int obterCursoAtivo(
    sqlite3 *db,
    const char *codigo,
    int *idCurso,
    int *capacidade
) {
    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT id_curso, capacidade_vagas "
        "FROM cursos "
        "WHERE codigo = ? AND situacao = 'ATIVO';";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao consultar curso: %s\n",
               sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(
        stmt, 1, codigo, -1, SQLITE_TRANSIENT
    );

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {
        *idCurso = sqlite3_column_int(stmt, 0);
        *capacidade = sqlite3_column_int(stmt, 1);

        sqlite3_finalize(stmt);
        return 1;
    }

    if (resultado != SQLITE_DONE) {
        printf("Erro ao consultar curso: %s\n",
               sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

// Verifica matriculas pendentes ou ativas duplicadas
static int contarDuplicadas(
    sqlite3 *db,
    int idEstudante,
    int idCurso
) {
    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT COUNT(*) "
        "FROM matriculas "
        "WHERE id_estudante = ? "
        "AND id_curso = ? "
        "AND status IN ('PENDENTE', 'ATIVA');";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao verificar duplicidade: %s\n",
               sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, idEstudante);
    sqlite3_bind_int(stmt, 2, idCurso);

    resultado = sqlite3_step(stmt);

    if (resultado != SQLITE_ROW) {
        printf("Erro ao verificar duplicidade: %s\n",
               sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int total = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return total;
}

// Conta matriculas que ocupam vagas
static int contarVagasOcupadas(
    sqlite3 *db,
    int idCurso
) {
    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT COUNT(*) "
        "FROM matriculas "
        "WHERE id_curso = ? "
        "AND status IN ('PENDENTE', 'ATIVA');";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao consultar vagas: %s\n",
               sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, idCurso);

    resultado = sqlite3_step(stmt);

    if (resultado != SQLITE_ROW) {
        printf("Erro ao consultar vagas: %s\n",
               sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int total = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return total;
}

// ========================================
// CADASTRAR MATRICULA
// ========================================
static void cadastrarMatricula(sqlite3 *db) {
    char codigoEstudante[31];
    char codigoCurso[31];

    int idEstudante;
    int idCurso;
    int capacidade;

    printf("\n========== NOVA MATRICULA ==========\n");

    if (!lerCampo(
        "Codigo do estudante: ",
        codigoEstudante,
        sizeof(codigoEstudante),
        1
    )) {
        return;
    }

    if (!lerCampo(
        "Codigo do curso: ",
        codigoCurso,
        sizeof(codigoCurso),
        1
    )) {
        return;
    }

    if (!iniciarTransacao(db)) {
        return;
    }

    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        cancelarTransacao(db);
        return;
    }

    int resultado = obterEstudanteAtivo(
        db,
        codigoEstudante,
        &idEstudante
    );

    if (resultado != 1) {
        if (resultado == 0) {
            printf("\nEstudante nao encontrado ou inativo.\n");
        }

        cancelarTransacao(db);
        return;
    }

    resultado = obterCursoAtivo(
        db,
        codigoCurso,
        &idCurso,
        &capacidade
    );

    if (resultado != 1) {
        if (resultado == 0) {
            printf("\nCurso nao encontrado ou inativo.\n");
        }

        cancelarTransacao(db);
        return;
    }

    int duplicadas = contarDuplicadas(
        db,
        idEstudante,
        idCurso
    );

    if (duplicadas < 0) {
        cancelarTransacao(db);
        return;
    }

    if (duplicadas > 0) {
        printf(
            "\nEste estudante ja possui uma matricula "
            "pendente ou ativa neste curso.\n"
        );

        cancelarTransacao(db);
        return;
    }

    int ocupadas = contarVagasOcupadas(db, idCurso);

    if (ocupadas < 0) {
        cancelarTransacao(db);
        return;
    }

    if (ocupadas >= capacidade) {
        printf("\nNao ha vagas disponiveis neste curso.\n");

        cancelarTransacao(db);
        return;
    }

    const char *sql =
        "INSERT INTO matriculas "
        "(id_estudante, id_curso, status, criada_em, expira_em) "
        "VALUES (?, ?, 'PENDENTE', CURRENT_TIMESTAMP, "
        "datetime('now', '+48 hours'));";

    sqlite3_stmt *stmt = NULL;

    resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar matricula: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    sqlite3_bind_int(stmt, 1, idEstudante);
    sqlite3_bind_int(stmt, 2, idCurso);

    resultado = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (resultado != SQLITE_DONE) {
        printf("Erro ao cadastrar matricula: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    int novoId = (int)sqlite3_last_insert_rowid(db);

    if (!confirmarTransacao(db)) {
        return;
    }

    printf("\nMatricula criada com sucesso!\n");
    printf("Numero da matricula: %d\n", novoId);
    printf("Status: PENDENTE\n");
    printf("Prazo para confirmacao: 48 horas.\n");
    printf("Vagas ocupadas: %d de %d\n",
           ocupadas + 1, capacidade);
}

// ========================================
// EXIBIR MATRICULAS
// ========================================
static void imprimirMatricula(sqlite3_stmt *stmt) {
    printf("\nMatricula: %d\n",
           sqlite3_column_int(stmt, 0));

    printf("Codigo do estudante: %s\n",
           (const char *)sqlite3_column_text(stmt, 1));

    printf("Estudante: %s\n",
           (const char *)sqlite3_column_text(stmt, 2));

    printf("Codigo do curso: %s\n",
           (const char *)sqlite3_column_text(stmt, 3));

    printf("Curso: %s\n",
           (const char *)sqlite3_column_text(stmt, 4));

    printf("Status: %s\n",
           (const char *)sqlite3_column_text(stmt, 5));

    printf("Criada em: %s\n",
           (const char *)sqlite3_column_text(stmt, 6));

    printf("Expira em: %s\n",
           (const char *)sqlite3_column_text(stmt, 7));
}

// ========================================
// LISTAR MATRICULAS
// ========================================
static void listarMatriculas(sqlite3 *db) {
    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        return;
    }

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT "
        "m.id_matricula, "
        "e.codigo, "
        "e.nome, "
        "c.codigo, "
        "c.nome, "
        "m.status, "
        "datetime(m.criada_em, 'localtime'), "
        "CASE WHEN m.status = 'PENDENTE' "
        "THEN datetime(m.expira_em, 'localtime') "
        "ELSE '-' END "
        "FROM matriculas m "
        "INNER JOIN estudantes e "
        "ON e.id_estudante = m.id_estudante "
        "INNER JOIN cursos c "
        "ON c.id_curso = m.id_curso "
        "ORDER BY m.id_matricula DESC;";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao listar matriculas: %s\n",
               sqlite3_errmsg(db));
        return;
    }

    printf("\n========== MATRICULAS CADASTRADAS ==========\n");

    int encontrou = 0;

    while ((resultado = sqlite3_step(stmt)) == SQLITE_ROW) {
        encontrou = 1;
        imprimirMatricula(stmt);
        printf("--------------------------------------------\n");
    }

    if (!encontrou) {
        printf("Nenhuma matricula cadastrada.\n");
    }

    if (resultado != SQLITE_DONE) {
        printf("Erro ao percorrer matriculas: %s\n",
               sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// ========================================
// BUSCAR MATRICULA
// ========================================
static void buscarMatricula(sqlite3 *db) {
    int idMatricula;

    if (!lerInteiro(
        "Numero da matricula: ",
        1,
        INT_MAX,
        &idMatricula
    )) {
        return;
    }

    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        return;
    }

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT "
        "m.id_matricula, "
        "e.codigo, "
        "e.nome, "
        "c.codigo, "
        "c.nome, "
        "m.status, "
        "datetime(m.criada_em, 'localtime'), "
        "CASE WHEN m.status = 'PENDENTE' "
        "THEN datetime(m.expira_em, 'localtime') "
        "ELSE '-' END "
        "FROM matriculas m "
        "INNER JOIN estudantes e "
        "ON e.id_estudante = m.id_estudante "
        "INNER JOIN cursos c "
        "ON c.id_curso = m.id_curso "
        "WHERE m.id_matricula = ?;";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar busca: %s\n",
               sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, idMatricula);

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {
        printf("\n========== MATRICULA ENCONTRADA ==========\n");
        imprimirMatricula(stmt);
    } else if (resultado == SQLITE_DONE) {
        printf("\nMatricula nao encontrada.\n");
    } else {
        printf("Erro ao buscar matricula: %s\n",
               sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// ========================================
// CONFIRMAR MATRICULA
// ========================================
static void confirmarMatricula(sqlite3 *db) {
    int idMatricula;

    printf("\n========== CONFIRMAR MATRICULA ==========\n");

    if (!lerInteiro(
        "Numero da matricula: ",
        1,
        INT_MAX,
        &idMatricula
    )) {
        return;
    }

    if (!iniciarTransacao(db)) {
        return;
    }

    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        cancelarTransacao(db);
        return;
    }

    const char *sql =
        "UPDATE matriculas "
        "SET status = 'ATIVA', "
        "confirmada_em = CURRENT_TIMESTAMP "
        "WHERE id_matricula = ? "
        "AND status = 'PENDENTE' "
        "AND datetime(expira_em) > datetime('now');";

    sqlite3_stmt *stmt = NULL;

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar confirmacao: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    sqlite3_bind_int(stmt, 1, idMatricula);

    resultado = sqlite3_step(stmt);

    int alteradas = sqlite3_changes(db);

    sqlite3_finalize(stmt);

    if (resultado != SQLITE_DONE) {
        printf("Erro ao confirmar matricula: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    if (alteradas == 0) {
        printf(
            "\nMatricula nao encontrada, ja confirmada "
            "ou com prazo expirado.\n"
        );

        cancelarTransacao(db);
        return;
    }

    if (confirmarTransacao(db)) {
        printf("\nMatricula confirmada com sucesso!\n");
        printf("Status atualizado para ATIVA.\n");
    }
}

// ========================================
// CANCELAR MATRICULA
// ========================================
static void cancelarMatricula(sqlite3 *db) {
    int idMatricula;

    printf("\n========== CANCELAR MATRICULA ==========\n");

    if (!lerInteiro(
        "Numero da matricula: ",
        1,
        INT_MAX,
        &idMatricula
    )) {
        return;
    }

    if (!iniciarTransacao(db)) {
        return;
    }

    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        cancelarTransacao(db);
        return;
    }

    const char *sql =
        "UPDATE matriculas "
        "SET status = 'CANCELADA', "
        "cancelada_em = CURRENT_TIMESTAMP "
        "WHERE id_matricula = ? "
        "AND status IN ('PENDENTE', 'ATIVA');";

    sqlite3_stmt *stmt = NULL;

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar cancelamento: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    sqlite3_bind_int(stmt, 1, idMatricula);

    resultado = sqlite3_step(stmt);

    int alteradas = sqlite3_changes(db);

    sqlite3_finalize(stmt);

    if (resultado != SQLITE_DONE) {
        printf("Erro ao cancelar matricula: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    if (alteradas == 0) {
        printf(
            "\nMatricula nao encontrada ou nao pode ser cancelada.\n"
        );

        cancelarTransacao(db);
        return;
    }

    if (confirmarTransacao(db)) {
        printf("\nMatricula cancelada com sucesso!\n");
        printf("A vaga foi liberada para novas matriculas.\n");
    }
}

// ========================================
// CONCLUIR MATRICULA
// ========================================
static void concluirMatricula(sqlite3 *db) {
    int idMatricula;

    printf("\n========== CONCLUIR MATRICULA ==========\n");

    if (!lerInteiro(
        "Numero da matricula: ",
        1,
        INT_MAX,
        &idMatricula
    )) {
        return;
    }

    if (!iniciarTransacao(db)) {
        return;
    }

    if (atualizarMatriculasExpiradas(db) != SQLITE_OK) {
        cancelarTransacao(db);
        return;
    }

    const char *sql =
        "UPDATE matriculas "
        "SET status = 'CONCLUIDA', "
        "concluida_em = CURRENT_TIMESTAMP "
        "WHERE id_matricula = ? "
        "AND status = 'ATIVA';";

    sqlite3_stmt *stmt = NULL;

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar conclusao: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    sqlite3_bind_int(stmt, 1, idMatricula);

    resultado = sqlite3_step(stmt);

    int alteradas = sqlite3_changes(db);

    sqlite3_finalize(stmt);

    if (resultado != SQLITE_DONE) {
        printf("Erro ao concluir matricula: %s\n",
               sqlite3_errmsg(db));
        cancelarTransacao(db);
        return;
    }

    if (alteradas == 0) {
        printf(
            "\nMatricula nao encontrada ou nao esta ativa.\n"
        );

        cancelarTransacao(db);
        return;
    }

    if (confirmarTransacao(db)) {
        printf("\nMatricula concluida com sucesso!\n");
        printf("A vaga foi liberada para novas matriculas.\n");
    }
}

// ========================================
// MENU DE MATRICULAS
// ========================================
void gerenciarMatriculas(sqlite3 *db, const char *perfil) {
    int opcao;

    int administrativo =
        strcmp(perfil, "ADMINISTRATIVO") == 0;

    do {
        printf("\n====================================\n");
        printf("      GERENCIAMENTO DE MATRICULAS\n");
        printf("====================================\n");

        if (administrativo) {
            printf("1 - Realizar matricula\n");
        }

        printf("2 - Listar matriculas\n");
        printf("3 - Buscar matricula\n");

        if (administrativo) {
            printf("4 - Confirmar matricula\n");
            printf("5 - Cancelar matricula\n");
        } else {
            printf("6 - Concluir matricula\n");
        }

        printf("0 - Voltar ao menu principal\n");
        printf("====================================\n");

        if (!lerInteiro(
            "Escolha uma opcao: ",
            0,
            6,
            &opcao
        )) {
            printf("\nEntrada encerrada.\n");
            return;
        }

        switch (opcao) {
            case 1:
                if (administrativo) {
                    cadastrarMatricula(db);
                } else {
                    printf("\nAcesso negado.\n");
                }
                break;

            case 2:
                listarMatriculas(db);
                break;

            case 3:
                buscarMatricula(db);
                break;

            case 4:
                if (administrativo) {
                    confirmarMatricula(db);
                } else {
                    printf("\nAcesso negado.\n");
                }
                break;

            case 5:
                if (administrativo) {
                    cancelarMatricula(db);
                } else {
                    printf("\nAcesso negado.\n");
                }
                break;

            case 6:
                if (!administrativo) {
                    concluirMatricula(db);
                } else {
                    printf("\nAcesso negado.\n");
                }
                break;

            case 0:
                printf("\nVoltando ao menu principal...\n");
                break;
        }

    } while (opcao != 0);
}