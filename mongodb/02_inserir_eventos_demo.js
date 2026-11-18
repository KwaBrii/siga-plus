
const banco = db.getSiblingDB("siga_plus");
const colecao = banco.getCollection("eventos_sistema");

const eventos = [
    {
        codigo_evento: "EVT001",
        tipo_evento: "MATRICULA_CRIADA",
        entidade: "matricula",
        id_entidade: NumberInt(2),
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            codigo_estudante: "ALU004",
            codigo_curso: "CUR002",
            status: "PENDENTE",
            data_criacao_original: "2026-11-17 23:15:53"
        }
    },
    {
        codigo_evento: "EVT002",
        tipo_evento: "MATRICULA_CONFIRMADA",
        entidade: "matricula",
        id_entidade: NumberInt(2),
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            codigo_estudante: "ALU004",
            codigo_curso: "CUR002",
            status_anterior: "PENDENTE",
            status_novo: "ATIVA"
        }
    },
    {
        codigo_evento: "EVT003",
        tipo_evento: "MATRICULA_CONCLUIDA",
        entidade: "matricula",
        id_entidade: NumberInt(2),
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            codigo_estudante: "ALU004",
            codigo_curso: "CUR002",
            status_anterior: "ATIVA",
            status_novo: "CONCLUIDA"
        }
    },
    {
        codigo_evento: "EVT004",
        tipo_evento: "MATRICULA_CRIADA",
        entidade: "matricula",
        id_entidade: NumberInt(3),
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            codigo_estudante: "ALU005",
            codigo_curso: "CUR002",
            status: "PENDENTE",
            data_criacao_original: "2026-11-17 23:16:07"
        }
    },
    {
        codigo_evento: "EVT005",
        tipo_evento: "MATRICULA_CANCELADA",
        entidade: "matricula",
        id_entidade: NumberInt(3),
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            codigo_estudante: "ALU005",
            codigo_curso: "CUR002",
            status_anterior: "PENDENTE",
            status_novo: "CANCELADA"
        }
    },
    {
        codigo_evento: "EVT006",
        tipo_evento: "MATRICULA_CRIADA",
        entidade: "matricula",
        id_entidade: NumberInt(4),
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            codigo_estudante: "ALU006",
            codigo_curso: "CUR002",
            status: "PENDENTE",
            data_criacao_original: "2026-11-17 23:55:13"
        }
    },
    {
        codigo_evento: "EVT007",
        tipo_evento: "ESTUDANTE_INATIVADO",
        entidade: "estudante",
        id_entidade: NumberInt(5),
        codigo_entidade: "ALU005",
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            situacao_anterior: "ATIVO",
            situacao_nova: "INATIVO"
        }
    },
    {
        codigo_evento: "EVT008",
        tipo_evento: "CURSO_INATIVADO",
        entidade: "curso",
        id_entidade: NumberInt(2),
        codigo_entidade: "CUR002",
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV",
        detalhes: {
            situacao_anterior: "ATIVO",
            situacao_nova: "INATIVO"
        }
    }
];

// Insere os eventos sem duplicar registros
// caso o script seja executado novamente.

let quantidadeInserida = 0;

eventos.forEach(function(evento) {
    const resultado = colecao.updateOne(
        { codigo_evento: evento.codigo_evento },
        { $setOnInsert: evento },
        { upsert: true }
    );

    if (resultado.upsertedCount === 1) {
        quantidadeInserida++;
    }
});

print("Eventos inseridos nesta execucao: " + quantidadeInserida);
print("Total de eventos na colecao: " +
    colecao.countDocuments());
