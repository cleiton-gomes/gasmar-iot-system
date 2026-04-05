#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#include "ra01s.h"

static const char *TAG = "MAIN";

/* ================= ADC CONFIG ================= */

#define GAS_SENSOR_CHAN     ADC_CHANNEL_3  // GPIO 4
#define LIMITE_ALERTA       2500
#define AMOSTRAS            20

adc_oneshot_unit_handle_t adc1_handle;

/* Variável global compartilhada */
volatile int gas_value = 0;

/* ================= TASK SENSOR ================= */

void task_sensor(void *pvParameters)
{
    ESP_LOGI("SENSOR", "Start");

    while (1) {
        int leitura_acumulada = 0;
        int valor_atual = 0;

        for (int i = 0; i < AMOSTRAS; i++) {
            adc_oneshot_read(adc1_handle, GAS_SENSOR_CHAN, &valor_atual);
            leitura_acumulada += valor_atual;
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        gas_value = leitura_acumulada / AMOSTRAS;

        if (gas_value > LIMITE_ALERTA) {
            ESP_LOGW("SENSOR", "!!! ALERTA GAS: %d !!!", gas_value);
        } else {
            ESP_LOGI("SENSOR", "Gas: %d", gas_value);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ================= TASK TX ================= */

void task_tx(void *pvParameters)
{
    ESP_LOGI("TX", "Start");

    uint8_t buf[255];

    while(1) {
        int current_gas = gas_value;

        int txLen = sprintf((char *)buf, "GAS:%d", current_gas);

        ESP_LOGI("TX", "Enviando: %s", buf);

        if (LoRaSend(buf, txLen, SX126x_TXMODE_SYNC) == false) {
            ESP_LOGE("TX", "LoRaSend fail");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ================= MAIN ================= */

void app_main()
{
    /* ==== ADC INIT ==== */
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, GAS_SENSOR_CHAN, &config));

    /* ==== LORA INIT ==== */
    LoRaInit();

    int8_t txPowerInDbm = 22;
    uint32_t frequencyInHz = 915000000;

    float tcxoVoltage = 3.3;
    bool useRegulatorLDO = true;

    if (LoRaBegin(frequencyInHz, txPowerInDbm, tcxoVoltage, useRegulatorLDO) != 0) {
        ESP_LOGE(TAG, "Does not recognize the module");
        while(1) vTaskDelay(1);
    }

    LoRaConfig(7, 4, 1, 8, 0, true, false);

    /* ==== TASKS ==== */
    xTaskCreate(task_sensor, "SENSOR", 4096, NULL, 5, NULL);
    xTaskCreate(task_tx, "TX", 4096, NULL, 5, NULL);
}