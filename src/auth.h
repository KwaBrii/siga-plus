#ifndef AUTH_H
#define AUTH_H

#include <sqlite3.h>

typedef struct {
    int id;
    char nome[121];
    char perfil[20];
} SessaoUsuario;

int iniciarSessao(sqlite3 *db, SessaoUsuario *sessao);

void gerenciarUsuarios(
    sqlite3 *db,
    const SessaoUsuario *sessao
);

#endif
