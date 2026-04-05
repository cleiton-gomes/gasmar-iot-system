// const SensorModel = require('../models/sensorModel');

// // Variável em memória para o dashboard rápido
// let ultimoStatus = { sensor_id: "Aguardando...", gas_level: 0, alerta: false };

// exports.receberDados = async (req, res) => {
//     try {
//         const { sensor_id, gas_level } = req.body;
//         const alerta = gas_level > 2500;

//         // Atualiza estado local para o frontend
//         ultimoStatus = { 
//             sensor_id, 
//             gas_level, 
//             alerta, 
//             timestamp: new Date().toLocaleTimeString() 
//         };

//         // Salva no Postgres via Model e armazena o retorno para o log
//         const novaLeitura = await SensorModel.salvarLeitura(sensor_id, gas_level, alerta);
        
//         // Agora o log funciona pois está dentro da função async
//         console.log("✅ Dado inserido com sucesso! ID:", novaLeitura.id);

//         res.status(201).json({ status: "sucesso", dados: ultimoStatus });
//     } catch (error) {
//         console.error("❌ Erro ao salvar leitura:", error.message);
//         res.status(500).json({ status: "erro", msg: error.message });
//     }
// };

// exports.getHistorico = async (req, res) => {
//     try {
//         const historico = await SensorModel.buscarHistorico();
//         res.status(200).json(historico);
//     } catch (error) {
//         console.error("❌ Erro ao buscar histórico:", error.message);
//         res.status(500).json({ status: "erro", msg: error.message });
//     }
// };


// exports.getUltimoStatus = (req, res) => {
//     res.json(ultimoStatus);
// };
const SensorModel = require('../models/sensorModel');

// Mantém o último status para o frontend
let ultimoStatus = { sensor_id: "Aguardando...", gas_level: 0, alerta: false, timestamp: null };

const SensorController = {
    /**
     * Recebe novos dados do sensor (via POST ou MQTT)
     */
    async receberDados(sensor_id, gas_level) {
        try {
            const alerta = gas_level > 2500;

            // Atualiza status em memória
            ultimoStatus = {
                sensor_id,
                gas_level,
                alerta,
                timestamp: new Date().toLocaleTimeString()
            };

            // Salva no banco via Model
            const novaLeitura = await SensorModel.salvarLeitura(sensor_id, gas_level, alerta);

            console.log("✅ Dado inserido com sucesso! ID:", novaLeitura.id);

            return ultimoStatus;
        } catch (error) {
            console.error("❌ Erro ao salvar leitura:", error.message);
            throw error;
        }
    },

    /**
     * Rota GET: Retorna o último status do sensor
     */
    getUltimoStatus(req, res) {
        res.json(ultimoStatus);
    },

    /**
     * Rota GET: Retorna histórico de leituras
     */
    async getHistorico(req, res) {
        try {
            const historico = await SensorModel.buscarHistorico();
            res.status(200).json(historico);
        } catch (error) {
            console.error("❌ Erro ao buscar histórico:", error.message);
            res.status(500).json({ status: "erro", msg: error.message });
        }
    }
};

module.exports = SensorController;