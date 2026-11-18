#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <conio.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include "auth.h"

#define TAM_SALT 16
#define TAM_HASH 32
#define ITERACOES_PBKDF2 600000

// ========================================
// FUNCOES AUXILIARES
// ========================================

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

static int lerCampo(
    const char *mensagem,
    char *campo,
    size_t tamanho
) {
    while (1) {
        if (!lerLinha(mensagem, campo, tamanho)) {
            return 0;
        }

        if (campo[0] == '\0') {
            printf("Este campo e obrigatorio!\n");
            continue;
        }

        return 1;
    }
}

static int lerInteiro(
    const char *mensagem,
    int minimo,
    int maximo,
    int *valor
) {
    char entrada[50];
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

// Le a senha sem exibi-la no terminal
static int lerSenha(
    const char *mensagem,
    char *senha,
    size_t tamanho
) {
    size_t posicao = 0;
    int caractere;

    printf("%s", mensagem);
    fflush(stdout);

    while (1) {
        caractere = _getch();

        if (caractere == '\r' || caractere == '\n') {
            break;
        }

        if (caractere == 8) {
            if (posicao > 0) {
                posicao--;
                printf("\b \b");
            }

            continue;
        }

        // Ignora teclas especiais
        if (caractere == 0 || caractere == 224) {
            _getch();
            continue;
        }

        if (
            caractere >= 32 &&
            caractere != 127 &&
            posicao < tamanho - 1
        ) {
            senha[posicao++] = (char)caractere;
            printf("*");
        }
    }

    senha[posicao] = '\0';

    printf("\n");

    return 1;
}

// ========================================
// PROTECAO DE SENHAS
// ========================================

static int gerarHashSenha(
    const char *senha,
    unsigned char *salt,
    unsigned char *hash
) {
    if (RAND_bytes(salt, TAM_SALT) != 1) {
        printf("Erro ao gerar salt para a senha.\n");
        return 0;
    }

    int resultado = PKCS5_PBKDF2_HMAC(
        senha,
        (int)strlen(senha),
        salt,
        TAM_SALT,
        ITERACOES_PBKDF2,
        EVP_sha256(),
        TAM_HASH,
        hash
    );

    if (resultado != 1) {
        printf("Erro ao proteger a senha.\n");
        return 0;
    }

    return 1;
}

// ========================================
// INSERIR USUARIO NO BANCO
// ========================================

static int inserirUsuario(
    sqlite3 *db,
    const char *nome,
    const char *login,
    const char *perfil,
    const char *senha,
    int *novoId
) {
    unsigned char salt[TAM_SALT];
    unsigned char hash[TAM_HASH];

    if (!gerarHashSenha(senha, salt, hash)) {
        return 0;
    }

    const char *sql =
        "INSERT INTO usuarios "
        "(nome, login, senha_hash, salt, perfil) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar usuario: %s\n",
               sqlite3_errmsg(db));

        OPENSSL_cleanse(hash, sizeof(hash));
        OPENSSL_cleanse(salt, sizeof(salt));

        return 0;
    }

    sqlite3_bind_text(
        stmt, 1, nome, -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt, 2, login, -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_blob(
        stmt, 3, hash, TAM_HASH, SQLITE_TRANSIENT
    );

    sqlite3_bind_blob(
        stmt, 4, salt, TAM_SALT, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt, 5, perfil, -1, SQLITE_TRANSIENT
    );

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_DONE) {
        if (novoId != NULL) {
            *novoId = (int)sqlite3_last_insert_rowid(db);
        }
    } else {
        printf("Erro ao cadastrar usuario: %s\n",
               sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    OPENSSL_cleanse(hash, sizeof(hash));
    OPENSSL_cleanse(salt, sizeof(salt));

    return resultado == SQLITE_DONE;
}

// ========================================
// CONTAR USUARIOS
// ========================================

static int contarUsuarios(sqlite3 *db, int *total) {
    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT COUNT(*) FROM usuarios;";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao consultar usuarios: %s\n",
               sqlite3_errmsg(db));
        return 0;
    }

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_ROW) {
        *total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return resultado == SQLITE_ROW;
}

// ========================================
// PRIMEIRO ADMINISTRADOR
// ========================================

static int criarPrimeiroAdministrador(
    sqlite3 *db,
    SessaoUsuario *sessao
) {
    char nome[121];
    char login[61];
    char senha[129];
    char confirmacao[129];

    int novoId;

    printf("\n====================================\n");
    printf("       CONFIGURACAO INICIAL\n");
    printf("====================================\n");

    printf("Nenhum usuario foi encontrado.\n");
    printf("Vamos cadastrar o primeiro administrador.\n\n");

    if (!lerCampo("Nome: ", nome, sizeof(nome))) {
        return 0;
    }

    if (!lerCampo("Login: ", login, sizeof(login))) {
        return 0;
    }

    while (1) {
        if (!lerSenha("Senha (minimo 8 caracteres): ",
                      senha, sizeof(senha))) {
            return 0;
        }

        if (strlen(senha) < 8) {
            printf("A senha precisa ter pelo menos 8 caracteres.\n");
            continue;
        }

        if (!lerSenha("Confirme a senha: ",
                      confirmacao, sizeof(confirmacao))) {
            OPENSSL_cleanse(senha, sizeof(senha));
            return 0;
        }

        if (strcmp(senha, confirmacao) != 0) {
            printf("As senhas nao correspondem.\n");
            OPENSSL_cleanse(senha, sizeof(senha));
            OPENSSL_cleanse(confirmacao, sizeof(confirmacao));
            continue;
        }

        break;
    }

    int sucesso = inserirUsuario(
        db,
        nome,
        login,
        "ADMINISTRATIVO",
        senha,
        &novoId
    );

    OPENSSL_cleanse(senha, sizeof(senha));
    OPENSSL_cleanse(confirmacao, sizeof(confirmacao));

    if (!sucesso) {
        printf("Nao foi possivel criar o administrador.\n");
        return 0;
    }

    sessao->id = novoId;

    snprintf(
        sessao->nome,
        sizeof(sessao->nome),
        "%s",
        nome
    );

    snprintf(
        sessao->perfil,
        sizeof(sessao->perfil),
        "%s",
        "ADMINISTRATIVO"
    );

    printf("\nAdministrador cadastrado com sucesso!\n");

    return 1;
}

// ========================================
// LOGIN
// ========================================

int iniciarSessao(
    sqlite3 *db,
    SessaoUsuario *sessao
) {
    int totalUsuarios = 0;

    if (!contarUsuarios(db, &totalUsuarios)) {
        return 0;
    }

    if (totalUsuarios == 0) {
        return criarPrimeiroAdministrador(db, sessao);
    }

    char login[61];
    char senha[129];

    for (int tentativa = 1; tentativa <= 3; tentativa++) {
        printf("\n====================================\n");
        printf("          LOGIN SIGA+\n");
        printf("====================================\n");

        if (!lerCampo("Login: ", login, sizeof(login))) {
            return 0;
        }

        if (!lerSenha("Senha: ", senha, sizeof(senha))) {
            return 0;
        }

        const char *sql =
            "SELECT id_usuario, nome, perfil, salt, senha_hash "
            "FROM usuarios "
            "WHERE login = ? AND ativo = 1;";

        sqlite3_stmt *stmt = NULL;

        int resultado = sqlite3_prepare_v2(
            db, sql, -1, &stmt, NULL
        );

        if (resultado != SQLITE_OK) {
            printf("Erro ao consultar login: %s\n",
                   sqlite3_errmsg(db));

            OPENSSL_cleanse(senha, sizeof(senha));
            return 0;
        }

        sqlite3_bind_text(
            stmt, 1, login, -1, SQLITE_TRANSIENT
        );

        resultado = sqlite3_step(stmt);

        int autenticado = 0;

        if (resultado == SQLITE_ROW) {
            const void *saltBanco = sqlite3_column_blob(stmt, 3);
            const void *hashBanco = sqlite3_column_blob(stmt, 4);

            int tamanhoSalt = sqlite3_column_bytes(stmt, 3);
            int tamanhoHash = sqlite3_column_bytes(stmt, 4);

            unsigned char hashCalculado[TAM_HASH];

            if (
                tamanhoSalt == TAM_SALT &&
                tamanhoHash == TAM_HASH &&
                saltBanco != NULL &&
                hashBanco != NULL
            ) {
                int calculado = PKCS5_PBKDF2_HMAC(
                    senha,
                    (int)strlen(senha),
                    saltBanco,
                    TAM_SALT,
                    ITERACOES_PBKDF2,
                    EVP_sha256(),
                    TAM_HASH,
                    hashCalculado
                );

                if (
                    calculado == 1 &&
                    CRYPTO_memcmp(
                        hashCalculado,
                        hashBanco,
                        TAM_HASH
                    ) == 0
                ) {
                    sessao->id = sqlite3_column_int(stmt, 0);

                    snprintf(
                        sessao->nome,
                        sizeof(sessao->nome),
                        "%s",
                        (const char *)sqlite3_column_text(stmt, 1)
                    );

                    snprintf(
                        sessao->perfil,
                        sizeof(sessao->perfil),
                        "%s",
                        (const char *)sqlite3_column_text(stmt, 2)
                    );

                    autenticado = 1;
                }
            }

            OPENSSL_cleanse(hashCalculado, sizeof(hashCalculado));
        } else if (resultado != SQLITE_DONE) {
            printf("Erro durante a autenticacao: %s\n",
                   sqlite3_errmsg(db));

            sqlite3_finalize(stmt);
            OPENSSL_cleanse(senha, sizeof(senha));

            return 0;
        }

        sqlite3_finalize(stmt);
        OPENSSL_cleanse(senha, sizeof(senha));

        if (autenticado) {
            printf("\nBem-vindo(a), %s!\n", sessao->nome);
            printf("Perfil: %s\n", sessao->perfil);
            return 1;
        }

        printf("\nLogin ou senha incorretos, ou usuario inativo.\n");
        printf("Tentativa %d de 3.\n", tentativa);
    }

    printf("\nLimite de tentativas atingido. Encerrando acesso.\n");

    return 0;
}

// ========================================
// CADASTRAR NOVO USUARIO
// ========================================

static void cadastrarUsuario(sqlite3 *db) {
    char nome[121];
    char login[61];
    char senha[129];
    char confirmacao[129];

    int opcaoPerfil;

    printf("\n========== CADASTRAR USUARIO ==========\n");

    if (!lerCampo("Nome: ", nome, sizeof(nome))) {
        return;
    }

    if (!lerCampo("Login: ", login, sizeof(login))) {
        return;
    }

    printf("\n1 - Administrativo\n");
    printf("2 - Pedagogico\n");

    if (!lerInteiro(
        "Perfil: ",
        1,
        2,
        &opcaoPerfil
    )) {
        return;
    }

    const char *perfil =
        opcaoPerfil == 1
        ? "ADMINISTRATIVO"
        : "PEDAGOGICO";

    while (1) {
        if (!lerSenha("Senha (minimo 8 caracteres): ",
                      senha, sizeof(senha))) {
            return;
        }

        if (strlen(senha) < 8) {
            printf("A senha precisa ter pelo menos 8 caracteres.\n");
            continue;
        }

        if (!lerSenha("Confirme a senha: ",
                      confirmacao, sizeof(confirmacao))) {
            OPENSSL_cleanse(senha, sizeof(senha));
            return;
        }

        if (strcmp(senha, confirmacao) != 0) {
            printf("As senhas nao correspondem.\n");
            OPENSSL_cleanse(senha, sizeof(senha));
            OPENSSL_cleanse(confirmacao, sizeof(confirmacao));
            continue;
        }

        break;
    }

    int sucesso = inserirUsuario(
        db,
        nome,
        login,
        perfil,
        senha,
        NULL
    );

    OPENSSL_cleanse(senha, sizeof(senha));
    OPENSSL_cleanse(confirmacao, sizeof(confirmacao));

    if (sucesso) {
        printf("\nUsuario cadastrado com sucesso!\n");
        printf("Perfil atribuido: %s\n", perfil);
    }
}

// ========================================
// LISTAR USUARIOS
// ========================================

static void listarUsuarios(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT id_usuario, nome, login, perfil, ativo "
        "FROM usuarios "
        "ORDER BY nome;";

    int resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao listar usuarios: %s\n",
               sqlite3_errmsg(db));
        return;
    }

    printf("\n================ USUARIOS ================\n");
    printf("%-5s | %-22s | %-18s | %-16s | %-10s\n",
           "ID", "NOME", "LOGIN", "PERFIL", "SITUACAO");

    printf("---------------------------------------------------------------\n");

    int encontrou = 0;

    while ((resultado = sqlite3_step(stmt)) == SQLITE_ROW) {
        encontrou = 1;

        printf(
            "%-5d | %-22s | %-18s | %-16s | %-10s\n",
            sqlite3_column_int(stmt, 0),
            (const char *)sqlite3_column_text(stmt, 1),
            (const char *)sqlite3_column_text(stmt, 2),
            (const char *)sqlite3_column_text(stmt, 3),
            sqlite3_column_int(stmt, 4) ? "ATIVO" : "INATIVO"
        );
    }

    if (!encontrou) {
        printf("Nenhum usuario cadastrado.\n");
    }

    sqlite3_finalize(stmt);
}

// ========================================
// INATIVAR USUARIO
// ========================================

static void inativarUsuario(
    sqlite3 *db,
    const SessaoUsuario *sessao
) {
    int idUsuario;
    int confirmacao;

    printf("\n========== INATIVAR USUARIO ==========\n");

    if (!lerInteiro(
        "ID do usuario: ",
        1,
        INT_MAX,
        &idUsuario
    )) {
        return;
    }

    if (idUsuario == sessao->id) {
        printf("\nVoce nao pode inativar sua propria conta.\n");
        return;
    }

    sqlite3_stmt *stmt = NULL;

    const char *sqlPerfil =
        "SELECT perfil FROM usuarios "
        "WHERE id_usuario = ? AND ativo = 1;";

    int resultado = sqlite3_prepare_v2(
        db, sqlPerfil, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao consultar usuario: %s\n",
               sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, idUsuario);

    resultado = sqlite3_step(stmt);

    if (resultado != SQLITE_ROW) {
        printf("\nUsuario nao encontrado ou ja inativo.\n");
        sqlite3_finalize(stmt);
        return;
    }

    char perfil[20];

    snprintf(
        perfil,
        sizeof(perfil),
        "%s",
        (const char *)sqlite3_column_text(stmt, 0)
    );

    sqlite3_finalize(stmt);

    // Impede a inativacao do ultimo administrador ativo
    if (strcmp(perfil, "ADMINISTRATIVO") == 0) {
        const char *sqlContagem =
            "SELECT COUNT(*) FROM usuarios "
            "WHERE perfil = 'ADMINISTRATIVO' "
            "AND ativo = 1 AND id_usuario <> ?;";

        resultado = sqlite3_prepare_v2(
            db, sqlContagem, -1, &stmt, NULL
        );

        if (resultado != SQLITE_OK) {
            printf("Erro ao verificar administradores.\n");
            return;
        }

        sqlite3_bind_int(stmt, 1, idUsuario);

        resultado = sqlite3_step(stmt);

        int outrosAdministradores = 0;

        if (resultado == SQLITE_ROW) {
            outrosAdministradores = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);

        if (outrosAdministradores == 0) {
            printf(
                "\nNao e permitido inativar o ultimo "
                "administrador ativo.\n"
            );
            return;
        }
    }

    if (!lerInteiro(
        "Confirmar inativacao? (1 - Sim / 0 - Nao): ",
        0,
        1,
        &confirmacao
    )) {
        return;
    }

    if (confirmacao == 0) {
        printf("\nOperacao cancelada.\n");
        return;
    }

    const char *sql =
        "UPDATE usuarios SET ativo = 0 "
        "WHERE id_usuario = ? AND ativo = 1;";

    resultado = sqlite3_prepare_v2(
        db, sql, -1, &stmt, NULL
    );

    if (resultado != SQLITE_OK) {
        printf("Erro ao preparar inativacao: %s\n",
               sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, idUsuario);

    resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_DONE && sqlite3_changes(db) > 0) {
        printf("\nUsuario inativado com sucesso!\n");
    } else {
        printf("\nNao foi possivel inativar o usuario.\n");
    }

    sqlite3_finalize(stmt);
}

// ========================================
// MENU DE USUARIOS
// ========================================

void gerenciarUsuarios(
    sqlite3 *db,
    const SessaoUsuario *sessao
) {
    if (strcmp(sessao->perfil, "ADMINISTRATIVO") != 0) {
        printf("\nAcesso negado. Apenas administradores podem gerenciar usuarios.\n");
        return;
    }

    int opcao;

    do {
        printf("\n====================================\n");
        printf("       GERENCIAMENTO DE USUARIOS\n");
        printf("====================================\n");
        printf("1 - Cadastrar usuario\n");
        printf("2 - Listar usuarios\n");
        printf("3 - Inativar usuario\n");
        printf("0 - Voltar ao menu principal\n");
        printf("====================================\n");

        if (!lerInteiro(
            "Escolha uma opcao: ",
            0,
            3,
            &opcao
        )) {
            return;
        }

        switch (opcao) {
            case 1:
                cadastrarUsuario(db);
                break;

            case 2:
                listarUsuarios(db);
                break;

            case 3:
                inativarUsuario(db, sessao);
                break;

            case 0:
                printf("\nVoltando ao menu principal...\n");
                break;
        }

    } while (opcao != 0);
}