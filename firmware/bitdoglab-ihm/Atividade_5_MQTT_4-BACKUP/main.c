/**
 * @file main.c
 * @brief Núcleo 0 - Controle com Joystick, Botões e MQTT com Auto-Reconexão.
 */

#include "fila_circular.h"
#include "rgb_pwm_control.h"
#include "configura_geral.h"
#include "oled_utils.h"
#include "ssd1306_i2c.h"
#include "mqtt_lwip.h"
#include "lwip/ip_addr.h"
#include "pico/multicore.h"
#include <stdio.h>
#include "estado_mqtt.h"
#include <stdbool.h>
#include "pico/time.h"
#include "main_auxiliar.h"
#include "neopixel_driver.h"
#include "hardware/adc.h"
#include <stdint.h>

// --- Definições de Hardware BitDogLab ---
#define JOY_X_PIN 26
#define JOY_Y_PIN 27
#define JOY_PB    22
#define BOTAO_A     5
#define BOTAO_B     6

// --- Variáveis de Controle ---
volatile uint32_t intervalo_ping_ms = 60000;
volatile uint8_t cor_rgb_pendente = 255;
absolute_time_t tempo_rgb_expiracao;
bool exibir_cor_agendada = false;
absolute_time_t tempo_pico_w = {0};

// --- Declarações Externas ---
extern void funcao_wifi_nucleo1(void);
extern void espera_usb();
extern void tratar_ip_binario(uint32_t ip_bin);
extern void tratar_mensagem(MensagemWiFi msg);
extern void executar_cena_local(uint16_t codigo);

// --- Protótipos Locais ---
void inicia_hardware();
void inicia_core1();
void verificar_fifo(void);
void tratar_fila(void);
void inicializar_mqtt_se_preciso(void);
void enviar_ping_periodico(void);
void processar_controles(void);
void mostrar_cor_rgb(uint8_t codigo); // Ajustado para uint8_t conforme main_auxiliar.h

FilaCircular fila_wifi;
absolute_time_t proximo_envio;
bool ip_recebido = false;

int main() {
    inicia_hardware();
    inicia_core1();

    while (true) {
        verificar_fifo();               // Mensagens do Core 1 (IP, Pings, MQTT)
        tratar_fila();                  // Processa fila e atualiza OLED
        inicializar_mqtt_se_preciso();  // Lógica de Auto-Reconexão
        enviar_ping_periodico();        // Keep-alive MQTT
        processar_controles();          // Lê sensores e botões

        // Publicação de status online (Retain)
        if (publicar_online && cliente_mqtt_ativo()) {
            if (is_nil_time(tempo_pico_w)) {
                tempo_pico_w = make_timeout_time_ms(2000);
            } else if (absolute_time_diff_us(get_absolute_time(), tempo_pico_w) <= 0) {
                publicar_online_retain();
                publicar_online = false;
                tempo_pico_w = nil_time;
            }
        }

        // Timer para resetar cor do LED RGB (Feedback visual)
        if (exibir_cor_agendada && to_ms_since_boot(get_absolute_time()) >= to_ms_since_boot(tempo_rgb_expiracao)) {
            mostrar_cor_rgb(cor_rgb_pendente);
            exibir_cor_agendada = false;
            cor_rgb_pendente = 255;
        }

        sleep_ms(100); 
    }
    return 0;
}

void inicializar_mqtt_se_preciso(void) {
    static absolute_time_t proxima_tentativa = {0};
    
    if (!mqtt_iniciado && ultimo_ip_bin != 0) {
        if (absolute_time_diff_us(get_absolute_time(), proxima_tentativa) <= 0) {
            printf("[SISTEMA] Tentando estabelecer conexão MQTT...\n");
            iniciar_mqtt_cliente();
            mqtt_iniciado = true; 
            proximo_envio = make_timeout_time_ms(5000);
            proxima_tentativa = make_timeout_time_ms(10000);
        }
    }
}

void enviar_ping_periodico(void) {
    absolute_time_t agora = get_absolute_time();
    if (to_us_since_boot(agora) >= to_us_since_boot(proximo_envio)) {
        if(cliente_mqtt_ativo()) {
            publicar_mensagem_mqtt(TOPICO_PING, "PING");
            printf("[MQTT] Enviando PING de rotina...\n");
        }
        proximo_envio = make_timeout_time_ms(intervalo_ping_ms);
    }
}

void processar_controles() {
    adc_select_input(0); // Eixo Y
    uint16_t y_val = adc_read();
    adc_select_input(1); // Eixo X
    uint16_t x_val = adc_read();

    bool sw_val = !gpio_get(JOY_PB);
    bool btn_a  = !gpio_get(BOTAO_A);
    bool btn_b  = !gpio_get(BOTAO_B);

    static uint32_t ultimo_debounce = 0;
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    if (agora - ultimo_debounce > 300) { 
        if (btn_a) {
            processar_comando_cenario(0x01); 
            ultimo_debounce = agora;
        }
        else if (btn_b) {
            processar_comando_cenario(0x02); 
            ultimo_debounce = agora;
        }
        else if (sw_val) {
            processar_comando_cenario(0x0F); 
            ultimo_debounce = agora;
        }

        if (x_val > 3800) {
            processar_comando_cenario(0x03);
            ultimo_debounce = agora;
        } 
        else if (x_val < 300) {
            processar_comando_cenario(0x04);
            ultimo_debounce = agora;
        }

        if (y_val > 3800) {
            processar_comando_cenario(0x0A); 
            ultimo_debounce = agora;
        } 
        else if (y_val < 300) {
            processar_comando_cenario(0x06);
            ultimo_debounce = agora;
        }
    }
}

void inicia_hardware() {
    stdio_init_all();
    setup_init_oled();
    espera_usb();

    npInit(LED_PIN);
    npClear();
    npWrite();

    adc_init();
    adc_gpio_init(JOY_X_PIN);
    adc_gpio_init(JOY_Y_PIN);

    uint pins[] = {JOY_PB, BOTAO_A, BOTAO_B};
    for(int i=0; i<3; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);
    }

    oled_clear(buffer_oled, &area);
    render_on_display(buffer_oled, &area);
}

void verificar_fifo(void) {
    if (!multicore_fifo_rvalid()) return;
    uint32_t pacote = multicore_fifo_pop_blocking();
    uint16_t comando = pacote >> 16;
    uint16_t valor = pacote & 0xFFFF;

    if (comando == 0xABCD) { set_novo_intervalo_ping((uint32_t)valor); return; }
    
    if (comando == 0xFFFE) {
        uint32_t ip_bin = multicore_fifo_pop_blocking();
        tratar_ip_binario(ip_bin);
        ip_recebido = true;
        return;
    }

    if (comando == 0xB1B1) {
        cor_rgb_pendente = (uint8_t)valor;
        tempo_rgb_expiracao = make_timeout_time_ms(500);
        exibir_cor_agendada = true;
        return;
    }

    if (comando == 0xD1D1) { 
        printf("[NÚCLEO 0] Comando via MQTT recebido: 0x%02X\n", valor);
        executar_cena_local(valor); 
        return; 
    }

    if (comando == 0xC0DE) { 
        processar_comando_cenario(valor); 
        return; 
    }

    MensagemWiFi msg = {.tentativa = comando, .status = valor};
    fila_inserir(&fila_wifi, msg);
}

void tratar_fila(void) {
    MensagemWiFi msg_recebida;
    if (fila_remover(&fila_wifi, &msg_recebida)) tratar_mensagem(msg_recebida);
}

void inicia_core1(){
    ssd1306_draw_utf8_multiline(buffer_oled, 0, 0, "Nucleo 0 OK");
    render_on_display(buffer_oled, &area);
    sleep_ms(500);
    init_rgb_pwm();
    fila_inicializar(&fila_wifi);
    multicore_launch_core1(funcao_wifi_nucleo1);
}

/**
 * @brief Converte o código de cor recebido em níveis PWM para o driver.
 * Implementação local para satisfazer o Linker e o main_auxiliar.h
 */
void mostrar_cor_rgb(uint8_t codigo) {
    uint16_t r = 0, g = 0, b = 0;
    uint16_t brilho = 4000; 

    switch (codigo) {
        case 1: b = brilho; break;                         // AZUL
        case 2: g = brilho; break;                         // VERDE
        case 3: g = brilho; b = brilho; break;             // CIANO
        case 4: r = brilho; break;                         // VERMELHO
        case 5: r = brilho; b = brilho; break;             // MAGENTA
        case 6: r = brilho; g = brilho; break;             // AMARELO
        case 7: r = brilho; g = brilho; b = brilho; break; // BRANCO
        default: r = 0; g = 0; b = 0; break;               // APAGAR
    }
    set_rgb_pwm(r, g, b); 
}