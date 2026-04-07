const ctx = document.getElementById('gasChart').getContext('2d');
let gasData = [];
let labels = [];

// Configuração do Gráfico Chart.js
const chart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: labels,
        datasets: [{
            label: 'Nível de Gás',
            data: gasData,
            borderColor: '#38bdf8',
            backgroundColor: 'rgba(56, 189, 248, 0.1)',
            borderWidth: 3,
            fill: true,
            tension: 0.4,
            pointRadius: 0
        }]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: {
            y: { beginAtZero: true, grid: { color: '#334155' } },
            x: { grid: { display: false } }
        },
        plugins: { legend: { display: false } }
    }
});

// Função para atualizar elementos de status e gráfico
function atualizarTela(data) {
    const { sensor_id, gas_level, alerta } = data;

    const sensorIdEl = document.getElementById('sensor-id');
    const gasLevelEl = document.getElementById('gas-level');
    const statusTexto = document.getElementById('status-texto');
    const diagContainer = document.getElementById('diagnostico-container');
    const chip = document.getElementById('status-chip');
    const msg = document.getElementById('alerta-msg');

    // Atualiza dados básicos
    sensorIdEl.innerText = sensor_id;
    gasLevelEl.innerText = gas_level;

    // Atualiza status e cores
    if (alerta) {
        chip.className = 'status-badge status-alerta';
        chip.innerText = '⚠️ ALERTA CRÍTICO';
        msg.style.display = 'block';

        statusTexto.innerText = "⚠️ VAZAMENTO DE GÁS!";
        statusTexto.style.color = "var(--danger)";
        diagContainer.style.borderColor = "var(--danger)";
        gasLevelEl.style.color = "var(--danger)";
    } else {
        chip.className = 'status-badge status-ok';
        chip.innerText = 'SISTEMA ONLINE';
        msg.style.display = 'none';

        statusTexto.innerText = "✅ Ar Seguro / Normal";
        statusTexto.style.color = "var(--success)";
        diagContainer.style.borderColor = "#334155";
        gasLevelEl.style.color = "var(--accent)";
    }

    // Atualiza gráfico (mantendo últimos 20 pontos)
    const now = new Date().toLocaleTimeString();
    if (labels.length > 20) {
        labels.shift();
        gasData.shift();
    }
    labels.push(now);
    gasData.push(gas_level);
    chart.update();
}

// --- 1️⃣ Carrega último estado do banco ao abrir a página ---
async function carregarUltimoEstado() {
    try {
        const response = await fetch('http://localhost:3000/sensor/historico');
        const data = await response.json();
        atualizarTela(data);
    } catch (error) {
        console.error("Falha na conexão com a API GásMar");
        document.getElementById('status-texto').innerText = "❌ API OFFLINE";
        document.getElementById('status-texto').style.color = "gray";
    }
}

// --- 2️⃣ Conecta WebSocket para dados MQTT em tempo real ---
const socket = io('http://localhost:3000'); // Endereço do seu servidor Node.js
socket.on('mqtt-dado', (data) => {
    atualizarTela(data);
});

// --- 3️⃣ Função para baixar PDF ---
document.getElementById('btn-download-pdf').addEventListener('click', async () => {
    const { jsPDF } = window.jspdf;
    const pdf = new jsPDF();

    // Título do PDF
    pdf.setFontSize(18);
    pdf.text("Relatório de Leitura de Gás - GásMar", 10, 20);

    // Dados do sensor
    const sensorId = document.getElementById('sensor-id').innerText;
    const gasLevel = document.getElementById('gas-level').innerText;
    const status = document.getElementById('status-texto').innerText;

    pdf.setFontSize(12);
    pdf.text(`Sensor ID: ${sensorId}`, 10, 40);
    pdf.text(`Nível Atual (PPM): ${gasLevel}`, 10, 50);
    pdf.text(`Status: ${status}`, 10, 60);

    // Adiciona gráfico (convertendo canvas em imagem)
    const canvas = document.getElementById('gasChart');
    const imgData = canvas.toDataURL('image/png');
    pdf.addImage(imgData, 'PNG', 10, 70, 180, 90);

    pdf.save(`relatorio_gas_${new Date().toISOString().slice(0,10)}.pdf`);
});

// Inicializa
carregarUltimoEstado();