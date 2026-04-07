
# 📑 API GásMar - Documentação Técnica

**Base URL:** `http://localhost:3000`

**Tecnologia:** Node.js (Express)

## 1. Envio de Dados (Hardware -> API)

Utilizado pelo **ESP32** para registrar novas leituras de gás.

### **POST** `/sensor/data`

* **Descrição:** Recebe o nível de gás e processa se há necessidade de alerta.
* **Corpo da Requisição (JSON):**

```json
{
  "sensor_id": "ESP32_SALA_01",
  "gas_level": 400
}

```

* **Resposta de Sucesso (200 OK):**

```json
{
  "status": "sucesso",
  "dados": {
    "sensor_id": "ESP32_SALA_01",
    "gas_level": 400,
    "alerta": false,
    "timestamp": "22:00:12"
  }
}

```

---

## 2. Consulta de Histórico (Banco de Dados)

Utilizado para gerar gráficos ou tabelas de logs no Frontend.

### **GET** `/sensor/historico`

* **Descrição:** Retorna a lista completa de registros armazenados no banco.
* **Resposta de Sucesso (200 OK):**

```json
[
  {
    "id": 21,
    "sensor_id": "ESP32_SALA_01",
    "gas_level": 400,
    "alerta": 0,
    "data_hora": "2026-03-04 01:00:12"
  },
  ...
]

```

---

## 3. Status Atual (Tempo Real)

Utilizado para exibir o estado imediato do sensor no Dashboard.

### **GET** `/sensor/status`

* **Descrição:** Retorna o último estado capturado ou o estado inicial de espera.
* **Resposta (Estado Inicial):**

```json
{
  "sensor_id": "Aguardando...",
  "gas_level": 0,
  "alerta": false
}

```
