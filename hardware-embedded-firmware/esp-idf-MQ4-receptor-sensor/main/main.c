#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "mqtt_client.h"

#include "ra01s.h"

static const char *TAG = "MAIN";

/* ===========================
   🔧 CONFIGURAÇÕES (EDITAR)
   =========================== */

// ⚠️ USE WIFI 2.4GHz (NÃO 5GHz)
#define WIFI_SSID        "REDE_LOCAL"
#define WIFI_PASS        "@141785Xd"

// IP do seu notebook (onde está o Mosquitto)
#define MQTT_BROKER_URI  "mqtt://10.203.36.232"

/* =========================== */

esp_mqtt_client_handle_t mqtt_client;

/* ===========================
   📡 MQTT
   =========================== */

void mqtt_start(void)
{
    esp_mqtt_client_config_t config = {
        .broker.address.uri = MQTT_BROKER_URI,
    };

    mqtt_client = esp_mqtt_client_init(&config);
    esp_mqtt_client_start(mqtt_client);

    ESP_LOGI(TAG, "MQTT iniciado");
}

/* ===========================
   📡 WIFI EVENT HANDLER
   =========================== */

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi desconectado! Reconectando...");
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi conectado com IP!");
        mqtt_start(); // 🔥 MQTT inicia aqui (correto)
    }
}

/* ===========================
   📡 WIFI INIT
   =========================== */

void wifi_init(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Eventos
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Conectando ao WiFi...");
}

/* ===========================
   📡 LORA RX
   =========================== */

void task_rx(void *pvParameters)
{
    ESP_LOGI("RX", "Start");

    uint8_t buf[255];

    while (1)
    {
        uint8_t rxLen = LoRaReceive(buf, sizeof(buf));

        if (rxLen > 0)
        {
            ESP_LOGI("RX", "| %d byte packet received:[%.*s] |", rxLen, rxLen, buf);

            int8_t rssi, snr;
            GetPacketStatus(&rssi, &snr);

            ESP_LOGI("RX", "| RSSI=%d[dBm] | SNR=%d[dB] |", rssi, snr);

            // 🔥 ENVIO MQTT
            if (mqtt_client != NULL)
            {
                char data[50];
                sprintf(data, "%.*s", rxLen, buf);

                ESP_LOGI("MQTT", "Publicando: %s", data);

                esp_mqtt_client_publish(mqtt_client, "sensor/gas", data, 0, 1, 0);
            }
        }

        vTaskDelay(1);
    }
}

/* ===========================
   🚀 MAIN
   =========================== */

void app_main(void)
{
    // NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 🔹 WiFi (inicia conexão)
    wifi_init();

    // 🔹 LoRa
    LoRaInit();

    int8_t txPowerInDbm = 22;
    uint32_t frequencyInHz = 915000000;

    ESP_LOGI(TAG, "Frequency is 915MHz");

    float tcxoVoltage = 3.3;
    bool useRegulatorLDO = true;

    if (LoRaBegin(frequencyInHz, txPowerInDbm, tcxoVoltage, useRegulatorLDO) != 0)
    {
        ESP_LOGE(TAG, "LoRa não detectado!");
        while (1)
        {
            vTaskDelay(1);
        }
    }

    uint8_t spreadingFactor = 7;
    uint8_t bandwidth = 4;
    uint8_t codingRate = 1;

    LoRaConfig(spreadingFactor, bandwidth, codingRate, 8, 0, true, false);

    // 🔹 Task RX
    xTaskCreate(&task_rx, "RX", 4096, NULL, 5, NULL);
}