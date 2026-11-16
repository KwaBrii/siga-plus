
#include <stdio.h>

// Prototipos das funcoes
void exibirMenu(void);
void limparBuffer(void);
void gerenciarEstudantes(void);
void gerenciarCursos(void);
void gerenciarMatriculas(void);
void exibirRelatorios(void);

int main(void) {
    int opcao = -1;

    do {
        exibirMenu();
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            if (feof(stdin)) {
                break;
            }
            printf("\nEntrada invalida! Digite um numero.\n");
            limparBuffer();
            opcao = -1;
            continue;
        }

        limparBuffer();
        switch (opcao) {
            case 1:
                gerenciarEstudantes();
                break;
            case 2:
                gerenciarCursos();
                break;
            case 3:
                gerenciarMatriculas();
                break;
            case 4:
                exibirRelatorios();
                break;
            case 0:
                printf("\nEncerrando o SIGA+...\n");
                break;
            default:
                printf("\nOpcao inexistente! Tente novamente.\n");
                break;
        }

    } while (opcao != 0);
    return 0;
}

// Exibe o menu principal
void exibirMenu(void) {
    printf("\n");
    printf("============================================\n");
    printf("                    SIGA+\n");
    printf("    Sistema Integrado de Gestao Academica\n");
    printf("============================================\n");
    printf("1 - Gerenciar estudantes\n");
    printf("2 - Gerenciar cursos\n");
    printf("3 - Gerenciar matriculas\n");
    printf("4 - Relatorios\n");
    printf("0 - Sair\n");
    printf("============================================\n");
}

// Limpa os caracteres restantes da entrada
void limparBuffer(void) {
    int caractere;

    while ((caractere = getchar()) != '\n' && caractere != EOF) {
    }
}

// Modulo de estudantes
void gerenciarEstudantes(void) {
    printf("\nModulo de estudantes em desenvolvimento.\n");
}

// Modulo de cursos
void gerenciarCursos(void) {
    printf("\nModulo de cursos em desenvolvimento.\n");
}

// Modulo de matriculas
void gerenciarMatriculas(void) {
    printf("\nModulo de matriculas em desenvolvimento.\n");
}

// Modulo de relatorios
void exibirRelatorios(void) {
    printf("\nModulo de relatorios em desenvolvimento.\n");
}
