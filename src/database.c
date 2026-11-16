
#include <stdio.h>
#include "database.h"

// Conecta ao banco de dados
int conectarBanco(sqlite3 **db) {
    int resultado = sqlite3_open("siga_plus.db", db);
    if (resultado != SQLITE_OK) {
        fprintf(stderr, "Erro ao conectar ao banco: %s\n",
                sqlite3_errmsg(*db));
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
