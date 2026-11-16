
#include <stdio.h>
#include <string.h>
#include "estudantes.h"

// Limpa os caracteres restantes da entrada
static void limparBufferEstudantes(void) {
    int caractere;

    while ((caractere = getchar()) != '\n' && caractere != EOF) {
    }
}

// Le um campo de texto
static int lerCampo(const char *mensagem, char *campo,
                    size_t tamanho, int obrigatorio) {

    printf("%s", mensagem);

    if (fgets(campo, (int)tamanho, stdin) == NULL) {
        return 0;
    }

    if (strchr(campo, '\n') != NULL) {
        campo[strcspn(campo, "\n")] = '\0';
    } else {
        limparBufferEstudantes();
    }

    if (obrigatorio && campo[0] == '\0') {
        printf("Este campo e obrigatorio!\n");
        return 0;
    }

    return 1;
}

// Cadastra um estudante
static void cadastrarEstudante(sqlite3 *db) {

    char codigo[30];
    char nome[100];
    char email[100];
    char telefone[30];

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "INSERT INTO estudantes "
        "(codigo, nome, email, telefone) "
        "VALUES (?, ?, ?, ?);";

    printf("\n--- CADASTRO DE ESTUDANTE ---\n");

    if (!lerCampo("Codigo: ", codigo, sizeof(codigo), 1)) return;
    if (!lerCampo("Nome: ", nome, sizeof(nome), 1)) return;
    if (!lerCampo("Email: ", email, sizeof(email), 0)) return;
    if (!lerCampo("Telefone: ", telefone, sizeof(telefone), 0)) return;

    // Prepara a instrucao SQL
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar cadastro: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Vincula os dados aos parametros
    sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, nome, -1, SQLITE_TRANSIENT);

    if (email[0] != '\0') {
        sqlite3_bind_text(stmt, 3, email, -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 3);
    }

    if (telefone[0] != '\0') {
        sqlite3_bind_text(stmt, 4, telefone, -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 4);
    }

    // Executa o cadastro
    int resultado = sqlite3_step(stmt);

    if (resultado == SQLITE_DONE) {
        printf("\nEstudante cadastrado com sucesso!\n");
        printf("ID gerado: %lld\n",
               (long long)sqlite3_last_insert_rowid(db));
    } else {
        printf("\nErro ao cadastrar estudante: %s\n",
               sqlite3_errmsg(db));
        printf("Verifique se o codigo ja esta cadastrado.\n");
    }

    sqlite3_finalize(stmt);
}

// Lista os estudantes cadastrados
static void listarEstudantes(sqlite3 *db) {

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "SELECT id_estudante, codigo, nome, email, telefone, ativo "
        "FROM estudantes ORDER BY nome;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao consultar estudantes: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- ESTUDANTES CADASTRADOS ---\n");

    int resultado;
    int encontrou = 0;

    while ((resultado = sqlite3_step(stmt)) == SQLITE_ROW) {

        encontrou = 1;

        const unsigned char *codigo = sqlite3_column_text(stmt, 1);
        const unsigned char *nome = sqlite3_column_text(stmt, 2);
        const unsigned char *email = sqlite3_column_text(stmt, 3);
        const unsigned char *telefone = sqlite3_column_text(stmt, 4);
        printf("\nID: %d\n", sqlite3_column_int(stmt, 0));
        printf("Codigo: %s\n", codigo ? (const char *)codigo : "-");
        printf("Nome: %s\n", nome ? (const char *)nome : "-");
        printf("Email: %s\n", email ? (const char *)email : "-");
        printf("Telefone: %s\n", telefone ? (const char *)telefone : "-");

        printf("Situacao: %s\n",
               sqlite3_column_int(stmt, 5) ? "Ativo" : "Inativo");
        printf("--------------------------------\n");
    }

    if (!encontrou) {
        printf("Nenhum estudante cadastrado.\n");
    }
    if (resultado != SQLITE_DONE) {
        printf("Erro durante a consulta: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// Menu de gerenciamento de estudantes
void gerenciarEstudantes(sqlite3 *db) {
    int opcao = -1;
    do {
        printf("\n================================\n");
        printf("     GERENCIAR ESTUDANTES\n");
        printf("================================\n");
        printf("1 - Cadastrar estudante\n");
        printf("2 - Listar estudantes\n");
        printf("0 - Voltar ao menu principal\n");
        printf("================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            if (feof(stdin)) {
                break;
            }
            printf("\nEntrada invalida!\n");
            limparBufferEstudantes();
            opcao = -1;
            continue;
        }
        limparBufferEstudantes();
        switch (opcao) {
            case 1:
                cadastrarEstudante(db);
                break;

            case 2:
                listarEstudantes(db);
                break;

            case 0:
                printf("\nVoltando ao menu principal...\n");
                break;

            default:
                printf("\nOpcao inexistente!\n");
                break;
        }
    } while (opcao != 0);
}
