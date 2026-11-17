
#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

int conectarBanco(sqlite3 **db);
int criarTabelaEstudantes(sqlite3 *db);
int criarTabelaCursos(sqlite3 *db);
int criarTabelaMatriculas(sqlite3 *db);

#endif