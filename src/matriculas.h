
#ifndef MATRICULAS_H
#define MATRICULAS_H

#include <sqlite3.h>

void gerenciarMatriculas(sqlite3 *db, const char *perfil);
int atualizarMatriculasExpiradas(sqlite3 *db);

#endif
