/**
 * @file main_auxiliar.c
 * @brief Funções auxiliares atualizadas com o novo CENÁRIO 6.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"

// Includes do Projeto
#include "fila_circular.h"
#include "rgb_pwm_control.h"
#include "configura_geral.h"
#include "oled_utils.h"
#include "ssd1306_i2c.h"
#include "mqtt_lwip.h"
#include "lwip/ip_addr.h"
#include "estado_mqtt.h"
#include "neopixel_driver.h"
#include "main_auxiliar.h"

#ifndef NUM_PIXELS
#define NUM_PIXELS 25
#endif

#define FATOR_BRILHO_10_PER_CENT 0.05f 
#define MAX_BRILHO_CANAL 255

static int falhas_ping_consecutivas = 0;

void espera_usb() {
    while (!stdio_usb_connected()) {
        sleep_ms(200);
    }
    printf("Conexão USB estabelecida!\n");
}

// ======= 1. TOPO: ENDEREÇO IP =======
void tratar_ip_binario(uint32_t ip_bin) {
    char ip_str[24];
    uint8_t ip[4];

    ip[0] = (ip_bin >> 24) & 0xFF;
    ip[1] = (ip_bin >> 16) & 0xFF;
    ip[2] = (ip_bin >> 8) & 0xFF;
    ip[3] = ip_bin & 0xFF;

    snprintf(ip_str, sizeof(ip_str), ">%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    ssd1306_clear_area(buffer_oled, 0, 0, 127, 10);
    ssd1306_draw_utf8_string(buffer_oled, 0, 0, ip_str);
    render_on_display(buffer_oled, &area);

    printf("[NÚCLEO 0] Endereço IP: %s\n", ip_str);
    ultimo_ip_bin = ip_bin;
}

// ======= 2. STATUS WIFI / MQTT =======

void tratar_mensagem(MensagemWiFi msg) {
    const char *descricao = "";

    if (msg.tentativa == 0x9999) {
        ssd1306_clear_area(buffer_oled, 0, 54, 127, 63);
        if (msg.status == 0) {
            ssd1306_draw_utf8_string(buffer_oled, 0, 56, "ACK PING: OK");
            set_rgb_pwm(0, 65535, 0); 
            falhas_ping_consecutivas = 0;
        } else {
            falhas_ping_consecutivas++;
            ssd1306_draw_utf8_string(buffer_oled, 0, 56, "ACK PING: FALHOU");
            set_rgb_pwm(65535, 0, 0);

            if (falhas_ping_consecutivas >= 3) {
                mqtt_iniciado = false; 
                exibir_status_mqtt("RECONECTANDO...");
                falhas_ping_consecutivas = 0;
            }
        }
        render_on_display(buffer_oled, &area);
        return;
    }

    if (mqtt_iniciado) return;

    switch (msg.status) {
        case 0: descricao = "WIFI: BUSCANDO..."; break;
        case 1: descricao = "WIFI: CONECTADO!"; break;
        case 2: descricao = "WIFI: ERRO";      break;
        default: descricao = "WIFI: OFF";     break;
    }

    ssd1306_clear_area(buffer_oled, 0, 14, 127, 24);
    ssd1306_draw_utf8_string(buffer_oled, 0, 15, descricao);
    render_on_display(buffer_oled, &area);
}

void exibir_status_mqtt(const char *texto) {
    ssd1306_clear_area(buffer_oled, 0, 14, 127, 24);
    ssd1306_draw_utf8_string(buffer_oled, 0, 15, "MQTT:");
    ssd1306_draw_utf8_string(buffer_oled, 40, 15, texto);
    render_on_display(buffer_oled, &area);
}

// ======= 3. CENTRO: CENÁRIOS E AÇÕES =======

void exibir_mensagem_cenario_oled(const char* titulo, const char* mensagem) {
    ssd1306_clear_area(buffer_oled, 0, 28, 127, 50); 
    ssd1306_draw_utf8_string(buffer_oled, 0, 32, titulo);
    ssd1306_draw_utf8_string(buffer_oled, 0, 42, mensagem); 
    render_on_display(buffer_oled, &area);
    
    sleep_ms(1500); 
    
    ssd1306_clear_area(buffer_oled, 0, 28, 127, 50);
    render_on_display(buffer_oled, &area);
}

// --- Funções de Cena Individuais ---

void cena_all_off(void) {
    exibir_mensagem_cenario_oled("CENARIO:", "TUDO DESLIGADO");
    set_rgb_pwm(0, 0, 0);
    iluminacao_all_off();
}

void cena1(void) {
    exibir_mensagem_cenario_oled("CENARIO:", "CENA 1 ATIVA");
    set_rgb_pwm(PWM_STEP, 0, 0);
    iluminacao_cena1();
}

void cena2(void) {
    exibir_mensagem_cenario_oled("CENARIO:", "CENA 2 ATIVA");
    set_rgb_pwm(0, PWM_STEP, 0);
    iluminacao_cena2();
}

void cena3(void) {
    exibir_mensagem_cenario_oled("CENARIO:", "CENA 3 ATIVA");
    set_rgb_pwm(0, 0, PWM_STEP);
    iluminacao_cena3();
}

void cena4(void) {
    exibir_mensagem_cenario_oled("CENARIO:", "CENA 4 ATIVA");
    set_rgb_pwm(PWM_STEP, PWM_STEP, 0);
    iluminacao_cena4();
}

void cena5(void) {
    exibir_mensagem_cenario_oled("CENARIO:", "CENA 5 ATIVA");
    set_rgb_pwm(0, PWM_STEP, PWM_STEP);
    iluminacao_cena5();
}

// NOVO CENÁRIO 6
void cena6(void) {
    exibir_mensagem_cenario_oled("CENARIO:", "CENA 6 ATIVA");
    set_rgb_pwm(PWM_STEP, 0, PWM_STEP); // Exemplo: Cor Magenta no LED RGB
    iluminacao_cena6();
}

void executar_cena_local(uint16_t comando_cenario) {
    switch (comando_cenario) {
        case 0x0F: cena_all_off(); break;
        case 0x01: cena1(); break;
        case 0x02: cena2(); break;
        case 0x03: cena3(); break;
        case 0x04: cena4(); break;
        case 0x0A: cena5(); break;
        case 0x06: cena6(); break; // Adicionado 0x06
        default:
            exibir_mensagem_cenario_oled("CENARIO:", "DESCONHECIDO");
            break;
    }
}

void processar_comando_cenario(uint16_t comando_cenario) {
    const char* payload = "none";

    executar_cena_local(comando_cenario);

    switch (comando_cenario) {
        case 0x0F: payload = "all_off"; break;
        case 0x01: payload = "cena1";   break;
        case 0x02: payload = "cena2";   break;
        case 0x03: payload = "cena3";   break;
        case 0x04: payload = "cena4";   break;
        case 0x0A: payload = "cena5";   break;
        case 0x06: payload = "cena6";   break; // Adicionado
    }

    if (cliente_mqtt_ativo() && strcmp(payload, "none") != 0) {
        publicar_mensagem_mqtt("casa/cenario", payload);
    }
}

// ======= 4. NEOPIXELS =======

void iluminacao_all_off(void) { npClear(); npWrite(); }

void iluminacao_cena1(void) {
    npClear();
    uint8_t brilho = (uint8_t)(MAX_BRILHO_CANAL * FATOR_BRILHO_10_PER_CENT);
    npSetAll(0, 0, brilho); 
    npWrite();
}

void iluminacao_cena2(void) {
    npClear();
    uint8_t brilho = (uint8_t)(MAX_BRILHO_CANAL * FATOR_BRILHO_10_PER_CENT);
    for(int i=0; i<5; i++) npSetLED(getLEDIndex(i, i), 0, brilho, 0);
    npWrite();
}

void iluminacao_cena3(void) {
    npClear();
    uint8_t brilho = (uint8_t)(MAX_BRILHO_CANAL * FATOR_BRILHO_10_PER_CENT);
    for(int i=0; i<5; i++) {
        npSetLED(getLEDIndex(i, 0), brilho, 0, 0);
        npSetLED(getLEDIndex(i, 4), brilho, 0, 0);
    }
    npWrite();
}

void iluminacao_cena4(void) {
    npClear();
    for (uint i = 0; i < NUM_PIXELS; i++) {
        npSetLED(i, rand()%20, rand()%20, rand()%20);
    }
    npWrite();
}

void iluminacao_cena5(void) {
    npClear();
    uint8_t b = (uint8_t)(MAX_BRILHO_CANAL * FATOR_BRILHO_10_PER_CENT);
    int coracao[] = {7, 11, 12, 13, 17};
    for(int i=0; i<5; i++) npSetLED(coracao[i], b, 0, 0);
    npWrite();
}

// IMPLEMENTAÇÃO DA ILUMINAÇÃO DO CENÁRIO 6
void iluminacao_cena6(void) {
    npClear();
    uint8_t brilho = (uint8_t)(MAX_BRILHO_CANAL * FATOR_BRILHO_10_PER_CENT);
    // Exemplo: Acende as 4 bordas da matriz 5x5 em Magenta (Azul + Vermelho)
    for(int i=0; i<5; i++) {
        npSetLED(getLEDIndex(0, i), brilho, 0, brilho); // Linha superior
        npSetLED(getLEDIndex(4, i), brilho, 0, brilho); // Linha inferior
        npSetLED(getLEDIndex(i, 0), brilho, 0, brilho); // Coluna esquerda
        npSetLED(getLEDIndex(i, 4), brilho, 0, brilho); // Coluna direita
    }
    npWrite();
}

void set_novo_intervalo_ping(uint32_t novo_intervalo) {
    if (novo_intervalo >= 1000 && novo_intervalo <= 60000) {
        intervalo_ping_ms = novo_intervalo;
        char buffer_msg[32];
        snprintf(buffer_msg, sizeof(buffer_msg), "Ping: %u ms", novo_intervalo);
        ssd1306_clear_area(buffer_oled, 0, 44, 127, 54);
        ssd1306_draw_utf8_string(buffer_oled, 0, 46, buffer_msg);
        render_on_display(buffer_oled, &area);
    }
}