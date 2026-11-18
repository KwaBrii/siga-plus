# SIGA+

**Sistema Integrado de Gestão Acadêmica e de Matrículas**

O SIGA+ é um sistema de gerenciamento acadêmico desenvolvido em linguagem C como parte de um projeto acadêmico do curso de Análise e Desenvolvimento de Sistemas. A aplicação foi concebida para a instituição fictícia Acesso+ Educação, com o objetivo de organizar o cadastro de estudantes, cursos e matrículas, além de oferecer controle de acesso e relatórios administrativos.

O projeto utiliza SQLite para armazenar os dados relacionais da aplicação e MongoDB como estrutura documental complementar para demonstração do registro e consulta de eventos do sistema.

## Tecnologias utilizadas

- **C (C11):** desenvolvimento da aplicação de console.
- **SQLite:** armazenamento dos dados relacionais.
- **OpenSSL:** recursos criptográficos para proteção das senhas.
- **MongoDB:** armazenamento documental de eventos demonstrativos.
- **MongoDB Shell (mongosh):** execução dos scripts de criação, inserção e consulta.
- **GCC:** compilação do código-fonte.

## Funcionalidades

### Gerenciamento de estudantes
- Cadastro e consulta de estudantes.
- Pesquisa de registros.
- Inativação de estudantes.
- Preservação dos registros históricos.

### Gerenciamento de cursos
- Cadastro e consulta de cursos.
- Controle de capacidade de vagas.
- Inativação de cursos.
- Restrição de novas matrículas em cursos inativos.

### Gerenciamento de matrículas
- Criação de matrículas pendentes.
- Confirmação de matrículas.
- Cancelamento e conclusão.
- Expiração automática de matrículas pendentes após 48 horas, verificada durante a execução da aplicação.
- Consulta do histórico e da situação das matrículas.

### Autenticação e controle de acesso
- Autenticação de usuários.
- Cadastro, consulta e inativação de contas.
- Perfis de acesso Administrativo e Pedagógico.
- Restrição de funcionalidades conforme o perfil.
- Armazenamento de senhas com PBKDF2-HMAC-SHA256, utilizando salt individual.

### Relatórios
- Quantidade de estudantes ativos e inativos.
- Quantidade de cursos ativos e inativos.
- Matrículas agrupadas por situação.
- Ocupação de vagas por curso.

### Banco documental MongoDB
- Criação da coleção `eventos_sistema`.
- Validação da estrutura dos documentos com JSON Schema.
- Criação de índices para consultas.
- Inserção de eventos demonstrativos.
- Consulta de eventos por tipo e por entidade.
- Agrupamento e contagem de eventos por categoria.

## Regras de negócio

- Um estudante inativo não pode realizar novas matrículas.
- Um curso inativo não aceita novas matrículas.
- Um estudante não pode possuir mais de uma matrícula pendente ou ativa no mesmo curso.
- Matrículas pendentes e ativas ocupam vagas disponíveis.
- A confirmação de uma matrícula pendente não deve ocupar uma segunda vaga.
- Matrículas canceladas, concluídas ou expiradas deixam de ocupar vagas.
- Matrículas pendentes expiram após 48 horas.
- Registros de estudantes, cursos e matrículas são preservados para manter o histórico.
- O acesso aos recursos administrativos depende do perfil autenticado.

A verificação de expiração é realizada durante a inicialização e antes de operações relevantes. Não há um processo em segundo plano executando essa verificação enquanto a aplicação está fechada.

## Estrutura do projeto

```text
siga-plus/
│
├── src/
│   ├── main.c
│   ├── database.c
│   ├── database.h
│   ├── estudantes.c
│   ├── estudantes.h
│   ├── cursos.c
│   ├── cursos.h
│   ├── matriculas.c
│   ├── matriculas.h
│   ├── auth.c
│   ├── auth.h
│   ├── relatorios.c
│   └── relatorios.h
│
├── mongodb/
│   ├── 01_criar_estrutura.js
│   ├── 02_inserir_eventos_demo.js
│   └── 03_consultar_eventos.js
│
├── .gitignore
└── README.md
```

## Requisitos para execução

Para compilar e executar a aplicação principal, são necessários:

- Compilador GCC.
- Biblioteca SQLite e seus arquivos de desenvolvimento.
- Biblioteca OpenSSL e seus arquivos de desenvolvimento.
- Ambiente Windows com suporte às ferramentas de compilação utilizadas.

Para executar os scripts complementares, também são necessários:

- MongoDB Community Server em execução.
- MongoDB Shell (`mongosh`).

## Compilação e execução

Na raiz do projeto, execute o comando abaixo no PowerShell:

```powershell
gcc -Wall -Wextra -std=c11 src/main.c src/database.c src/estudantes.c src/cursos.c src/matriculas.c src/auth.c src/relatorios.c -o siga-plus.exe -lsqlite3 -lcrypto
```

Após a compilação, execute:

```powershell
.\siga-plus.exe
```

O banco SQLite é utilizado pela aplicação para persistir os dados relacionais. O arquivo do banco é mantido localmente e não deve ser versionado.

## Execução dos scripts MongoDB

Com o MongoDB Server em execução e o terminal aberto na raiz do projeto, execute os scripts na seguinte ordem:

**1. Criar a estrutura documental:**

```powershell
mongosh --file .\mongodb\01_criar_estrutura.js
```

**2. Inserir os eventos demonstrativos:**

```powershell
mongosh --file .\mongodb\02_inserir_eventos_demo.js
```

**3. Consultar os eventos e verificar a validação:**

```powershell
mongosh --file .\mongodb\03_consultar_eventos.js
```

Os scripts criam a coleção `eventos_sistema`, inserem documentos de demonstração e executam consultas para apresentar os recursos de armazenamento e recuperação documental.

## Testes realizados

Foram realizados testes manuais na aplicação para verificar:

- Cadastro, consulta e inativação de estudantes e cursos.
- Criação, confirmação, cancelamento e conclusão de matrículas.
- Bloqueio de matrículas duplicadas.
- Bloqueio de matrículas para estudantes ou cursos inativos.
- Controle de capacidade de vagas.
- Permissões de acesso conforme o perfil do usuário.
- Geração dos relatórios administrativos.

Na estrutura MongoDB, foram verificados:

- Criação da coleção e dos índices.
- Inserção de oito documentos demonstrativos.
- Consulta de eventos por tipo e por matrícula.
- Agrupamento de documentos por tipo de evento.
- Rejeição de documentos que não atendem às regras de validação.

**Observação:** o comportamento de expiração de matrículas está implementado na aplicação, mas seu cenário de expiração após 48 horas ainda não foi validado por um teste prático específico.

## Arquitetura e limitações atuais

O SQLite é o banco de dados principal da aplicação, responsável pelas operações de cadastro, gerenciamento e consulta dos registros acadêmicos.

O MongoDB foi incorporado como uma estrutura documental complementar, utilizada para demonstrar o armazenamento e a consulta de eventos relacionados às operações do SIGA+.

Atualmente, os dois bancos não possuem integração automática. Os documentos MongoDB são inseridos por scripts executados separadamente pelo MongoDB Shell, sem sincronização direta com a aplicação desenvolvida em C.

## Possíveis melhorias futuras

- Integração automática entre a aplicação em C e o MongoDB.
- Registro automático de eventos durante as operações do sistema.
- Implementação de testes automatizados.
- Ampliação dos relatórios e recursos de consulta.
- Desenvolvimento de uma interface gráfica.

---

**Projeto acadêmico — Análise e Desenvolvimento de Sistemas**  
**Instituição fictícia: Acesso+ Educação**