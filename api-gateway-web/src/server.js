require('dotenv').config();
const express = require('express');
const cors = require('cors');
const mqtt = require('mqtt');
const http = require('http'); 
const { Server } = require('socket.io');

const sensorRoutes = require('./routes/sensorRoutes'); 
const SensorModel = require('./models/sensorModel');

const app = express();
app.use(cors());
app.use(express.json());

// Criando servidor HTTP e integrando Socket.io
const server = http.createServer(app);
const io = new Server(server, {
    cors: { origin: "*" }
});

console.log("-----------------------------------------");
console.log("SISTEMA GASMAR - CENTRAL OPERACIONAL");
console.log("-----------------------------------------");

// --- CONFIGURAÇÃO MQTT ---
// IP do seu notebook ou do Broker que o ESP32 está enviando
const brokerUrl = 'mqtt://10.203.36.232'; 
const topic = 'sensor/gas';

const client = mqtt.connect(brokerUrl);

client.on('connect', () => {
    console.log('✅ Conectado ao Broker MQTT!');
    client.subscribe(topic);
});

client.on('message', async (topic, message) => {
    const msgString = message.toString();
    console.log(`📡 Recebido do Drone: ${msgString}`);

    try {
        let sensor_id = 1;
        let gas_level;

        // Tenta processar se for JSON
        try {
            const payload = JSON.parse(msgString);
            sensor_id = payload.sensor_id || 1;
            gas_level = Number(payload.gas_level);
        } catch (e) {
            // Se não for JSON, tenta o formato "GAS:1940"
            const match = msgString.match(/GAS:(\d+)/);
            if (match) {
                gas_level = parseInt(match[1]);
            }
        }

        if (gas_level !== undefined) {
            const alerta = gas_level > 400;

            // Salva no SQLite
            await SensorModel.salvarLeitura(sensor_id, gas_level, alerta);
            
            // Envia para o Painel (Frontend) em tempo real
            io.emit('mqtt-dado', { sensor_id, gas_level, alerta });

            console.log(`✅ Banco Atualizado | Gás: ${gas_level} | Alerta: ${alerta}`);
        }
    } catch (err) {
        console.error("❌ Erro ao processar dado:", err.message);
    }
});

// Rotas da API
app.get('/teste-direto', (req, res) => res.send('Servidor Gasmar Online!'));
app.use('/sensor', sensorRoutes);

// Servindo a pasta public (onde fica o seu Dashboard HTML)
app.use(express.static('public'));

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`🚀 API rodando em http://localhost:${PORT}`);
    console.log(`📊 Dashboard disponível no navegador.`);
});