#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- Configurações de Rede ---
const char* ssid = "Bolsolula";
const char* password = "verde3522";

// --- Configurações da API ---
// Substitua pelo IP da sua máquina onde o Node.js está rodando
const char* serverName = "http://192.168.0.107:3000/sensor/data"; 

void setup() {
  Serial.begin(115200);

  // Conectando ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado com sucesso!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Inicializa a conexão
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // --- Simulação dos Dados ---
    // Gera um valor entre 300 e 600 para simular o nível de gás
    int simulatedGasLevel = random(300, 600); 
    
    // Criando o JSON para envio
    StaticJsonDocument<200> doc;
    doc["sensor_id"] = "ESP32_SALA_01";
    doc["gas_level"] = simulatedGasLevel;

    String requestBody;
    serializeJson(doc, requestBody);

    // Envia o POST
    Serial.print("Enviando nível: ");
    Serial.println(simulatedGasLevel);
    
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Resposta da API: " + response);
    } else {
      Serial.print("Erro no envio: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  // Espera 5 segundos para o próximo envio
  delay(5000);
}
