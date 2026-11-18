
const banco = db.getSiblingDB("siga_plus");
const colecao = banco.getCollection("eventos_sistema");

// 1. Consultar todos os eventos.
print("\n=== TODOS OS EVENTOS ===");

colecao.find(
    {},
    { _id: 0 }
).sort(
    { codigo_evento: 1 }
).forEach(printjson);

// 2. Consultar eventos de cancelamento.
print("\n=== MATRICULAS CANCELADAS ===");

colecao.find(
    { tipo_evento: "MATRICULA_CANCELADA" },
    { _id: 0 }
).forEach(printjson);

// 3. Consultar o historico de uma matricula.
print("\n=== HISTORICO DA MATRICULA 3 ===");

colecao.find(
    {
        entidade: "matricula",
        id_entidade: NumberInt(3)
    },
    { _id: 0 }
).sort(
    { codigo_evento: 1 }
).forEach(printjson);

// 4. Contabilizar eventos por tipo.
print("\n=== QUANTIDADE DE EVENTOS POR TIPO ===");

colecao.aggregate([
    {
        $group: {
            _id: "$tipo_evento",
            total: { $sum: 1 }
        }
    },
    {
        $sort: { _id: 1 }
    }
]).forEach(printjson);

// 5. Testar a validacao da colecao.
print("\n=== TESTE DE VALIDACAO ===");

try {
    colecao.insertOne({
        codigo_evento: "EVT999",
        tipo_evento: "MATRICULA_CRIADA",
        entidade: "matricula",
        id_entidade: NumberInt(999),
        data_hora: new Date(),
        origem: "DEMONSTRACAO_PIM_IV"
    });

    print("ERRO: o documento incompleto foi aceito.");
} catch (erro) {
    if (erro.code === 121) {
        print("SUCESSO: documento incompleto bloqueado pela validacao.");
    } else {
        throw erro;
    }
}
