
const banco = db.getSiblingDB("siga_plus");
const nomeColecao = "eventos_sistema";

// Verifica se a colecao ja existe.
const colecoesExistentes = banco.getCollectionNames();

if (!colecoesExistentes.includes(nomeColecao)) {
    banco.createCollection(nomeColecao, {
        validator: {
            $jsonSchema: {
                bsonType: "object",
                required: [
                    "codigo_evento",
                    "tipo_evento",
                    "entidade",
                    "id_entidade",
                    "data_hora",
                    "origem",
                    "detalhes"
                ],
                properties: {
                    codigo_evento: {
                        bsonType: "string",
                        description: "Identificador unico do evento"
                    },
                    tipo_evento: {
                        bsonType: "string",
                        enum: [
                            "MATRICULA_CRIADA",
                            "MATRICULA_CONFIRMADA",
                            "MATRICULA_CANCELADA",
                            "MATRICULA_CONCLUIDA",
                            "MATRICULA_EXPIRADA",
                            "ESTUDANTE_INATIVADO",
                            "CURSO_INATIVADO"
                        ]
                    },
                    entidade: {
                        bsonType: "string",
                        enum: [
                            "estudante",
                            "curso",
                            "matricula"
                        ]
                    },
                    id_entidade: {
                        bsonType: ["int", "long"]
                    },
                    codigo_entidade: {
                        bsonType: "string"
                    },
                    data_hora: {
                        bsonType: "date"
                    },
                    origem: {
                        bsonType: "string"
                    },
                    detalhes: {
                        bsonType: "object"
                    }
                }
            }
        },
        validationLevel: "strict",
        validationAction: "error"
    });

    print("Colecao eventos_sistema criada com sucesso!");
} else {
    print("A colecao eventos_sistema ja existe.");
}

// Cria indices para facilitar as consultas.

const colecao = banco.getCollection(nomeColecao);

colecao.createIndex(
    { codigo_evento: 1 },
    { unique: true, name: "uk_codigo_evento" }
);

colecao.createIndex(
    { tipo_evento: 1, data_hora: -1 },
    { name: "idx_tipo_evento_data" }
);

colecao.createIndex(
    { entidade: 1, id_entidade: 1, data_hora: -1 },
    { name: "idx_entidade_id_data" }
);

print("Indices criados com sucesso!");
print("Estrutura do banco SIGA+ preparada.");
