#include <stdio.h>
#include "database.h"

// Conecta ao banco de dados
int conectarBanco(sqlite3 **db) {
    int resultado = sqlite3_open("siga_plus.db", db);

    if (resultado != SQLITE_OK) {
        fprintf(
            stderr,
            "Erro ao conectar ao banco: %s\n",
            sqlite3_errmsg(*db)
        );
        return resultado;
    }

    resultado = sqlite3_exec(
        *db,
        "PRAGMA foreign_keys = ON;",
        NULL,
        NULL,
        NULL
    );

    if (resultado != SQLITE_OK) {
        fprintf(
            stderr,
            "Erro ao ativar chaves estrangeiras: %s\n",
            sqlite3_errmsg(*db)
        );
        return resultado;
    }

    printf("\nBanco de dados conectado com sucesso!\n");
    return SQLITE_OK;
}

// Cria a tabela de estudantes
int criarTabelaEstudantes(sqlite3 *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS estudantes ("
        "id_estudante INTEGER PRIMARY KEY AUTOINCREMENT,"
        "codigo TEXT NOT NULL UNIQUE,"
        "nome TEXT NOT NULL,"
        "email TEXT,"
        "telefone TEXT,"
        "ativo INTEGER NOT NULL DEFAULT 1 CHECK (ativo IN (0, 1))"
        ");";

    char *mensagemErro = NULL;

    int resultado = sqlite3_exec(
        db,
        sql,
        NULL,
        NULL,
        &mensagemErro
    );

    if (resultado != SQLITE_OK) {
        fprintf(stderr, "Erro ao criar tabela: %s\n", mensagemErro);
        sqlite3_free(mensagemErro);
        return resultado;
    }

    printf("Tabela de estudantes verificada com sucesso!\n");

    return SQLITE_OK;
}

// Cria a tabela de cursos
int criarTabelaCursos(sqlite3 *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS cursos ("
        "id_curso INTEGER PRIMARY KEY AUTOINCREMENT,"
        "codigo TEXT NOT NULL UNIQUE,"
        "nome TEXT NOT NULL,"
        "carga_horaria INTEGER NOT NULL CHECK (carga_horaria > 0),"
        "capacidade_vagas INTEGER NOT NULL CHECK (capacidade_vagas > 0),"
        "situacao TEXT NOT NULL DEFAULT 'ATIVO' "
        "CHECK (situacao IN ('ATIVO', 'INATIVO'))"
        ");";

    char *mensagemErro = NULL;
    int resultado = sqlite3_exec(
        db,
        sql,
        NULL,
        NULL,
        &mensagemErro
    );
    if (resultado != SQLITE_OK) {
        fprintf(stderr, "Erro ao criar tabela de cursos: %s\n",
                mensagemErro);

        sqlite3_free(mensagemErro);
        return resultado;
    }
    printf("Tabela de cursos verificada com sucesso!\n");
    return SQLITE_OK;
}

int criarTabelaMatriculas(sqlite3 *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS matriculas ("
        "id_matricula INTEGER PRIMARY KEY AUTOINCREMENT,"
        "id_estudante INTEGER NOT NULL,"
        "id_curso INTEGER NOT NULL,"
        "status TEXT NOT NULL DEFAULT 'PENDENTE' "
        "CHECK (status IN ("
        "'PENDENTE', 'ATIVA', 'CANCELADA', "
        "'CONCLUIDA', 'EXPIRADA'"
        ")),"
        "criada_em TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "expira_em TEXT NOT NULL,"
        "confirmada_em TEXT,"
        "cancelada_em TEXT,"
        "concluida_em TEXT,"
        "FOREIGN KEY (id_estudante) "
        "REFERENCES estudantes(id_estudante),"
        "FOREIGN KEY (id_curso) "
        "REFERENCES cursos(id_curso)"
        ");"

        "CREATE UNIQUE INDEX IF NOT EXISTS "
        "idx_matriculas_unicas_ativas "
        "ON matriculas(id_estudante, id_curso) "
        "WHERE status IN ('PENDENTE', 'ATIVA');"

        "CREATE INDEX IF NOT EXISTS "
        "idx_matriculas_curso_status "
        "ON matriculas(id_curso, status);";

    char *mensagemErro = NULL;

    int resultado = sqlite3_exec(
        db,
        sql,
        NULL,
        NULL,
        &mensagemErro
    );

    if (resultado != SQLITE_OK) {
        fprintf(
            stderr,
            "Erro ao criar tabela de matriculas: %s\n",
            mensagemErro
        );

        sqlite3_free(mensagemErro);
        return resultado;
    }

    printf("Tabela de matriculas verificada com sucesso!\n");

    return SQLITE_OK;
}


int criarTabelaUsuarios(sqlite3 *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS usuarios ("
        "id_usuario INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "login TEXT NOT NULL UNIQUE,"
        "senha_hash BLOB NOT NULL CHECK(length(senha_hash) = 32),"
        "salt BLOB NOT NULL CHECK(length(salt) = 16),"
        "perfil TEXT NOT NULL "
        "CHECK(perfil IN ('ADMINISTRATIVO', 'PEDAGOGICO')),"
        "ativo INTEGER NOT NULL DEFAULT 1 "
        "CHECK(ativo IN (0, 1))"
        ");";

    char *mensagemErro = NULL;

    int resultado = sqlite3_exec(
        db,
        sql,
        NULL,
        NULL,
        &mensagemErro
    );

    if (resultado != SQLITE_OK) {
        fprintf(
            stderr,
            "Erro ao criar tabela de usuarios: %s\n",
            mensagemErro
        );

        sqlite3_free(mensagemErro);
        return resultado;
    }

    printf("Tabela de usuarios verificada com sucesso!\n");

    return SQLITE_OK;
}
