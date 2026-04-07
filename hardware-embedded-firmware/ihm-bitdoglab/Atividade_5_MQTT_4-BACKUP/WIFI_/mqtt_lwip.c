/**
 * @file mqtt_lwip.c
 * @brief Implementação completa do cliente MQTT com correção de retroalimentação.
 */

#include <stdio.h>
#include <string.h>
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "configura_geral.h"
#include "display_utils.h"
#include "mqtt_lwip.h"

// ========================
// VARIÁVEIS GLOBAIS INTERNAS
// ========================

bool publicar_online = false;
static char topico_recebido[64] = {0};
static mqtt_client_t *client;
static struct mqtt_connect_client_info_t ci;
static bool publicacao_em_andamento = false;

// ========================
// CALLBACKS DE ASSINATURA E DADOS
// ========================

static void mqtt_mensagem_cb(void *arg, const char *topic, u32_t tot_len) {
    // Mantendo seus logs de debug originais
    printf("[MQTT_LWIP DEBUG] TOPICO CRU RECEBIDO: \"%s\" (len: %u)\n", topic, tot_len);

    strncpy(topico_recebido, topic, sizeof(topico_recebido) - 1);
    topico_recebido[sizeof(topico_recebido) - 1] = '\0';
    
    printf("[MQTT_LWIP DEBUG] TOPICO ARMAZENADO: \"%s\"\n", topico_recebido);
}

static void mqtt_dados_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char buffer[16] = {0};
    memcpy(buffer, data, len < sizeof(buffer) - 1 ? len : sizeof(buffer) - 1);
    buffer[len] = '\0'; 

    printf("[MQTT_LWIP DEBUG] === Topico Recebido: \"%s\" ===\n", topico_recebido);
    printf("[MQTT_LWIP DEBUG] === Payload Recebido: \"%s\" ===\n", buffer);

    // --- 1. Intervalo do PING ---
    if (strncmp(topico_recebido, TOPICO_CONFIG_INTERVALO, strlen(TOPICO_CONFIG_INTERVALO)) == 0) {
        uint32_t novo_valor = (uint32_t) atoi(buffer);
        if (novo_valor >= 1000 && novo_valor <= 60000) {
            multicore_fifo_push_blocking((0xABCD << 16) | (novo_valor & 0xFFFF));
            printf("[MQTT] Novo intervalo recebido: %u ms\n", novo_valor);
        } else {
            printf("[MQTT] Intervalo fora do limite: %u\n", novo_valor);
        }
    }

    // --- 2. Controle do LED RGB ---
    else if (strncmp(topico_recebido, TOPICO_COMANDO_RGB, strlen(TOPICO_COMANDO_RGB)) == 0) {
        uint16_t cor = 0xFFFF;
        if      (strcasecmp(buffer, "APAGAR")   == 0) cor = 0;
        else if (strcasecmp(buffer, "AZUL")     == 0) cor = 1;
        else if (strcasecmp(buffer, "VERDE")    == 0) cor = 2;
        else if (strcasecmp(buffer, "CIANO")    == 0) cor = 3;
        else if (strcasecmp(buffer, "VERMELHO") == 0) cor = 4;
        else if (strcasecmp(buffer, "MAGENTA")  == 0) cor = 5;
        else if (strcasecmp(buffer, "AMARELO")  == 0) cor = 6;
        else if (strcasecmp(buffer, "BRANCO")   == 0) cor = 7;

        if (cor <= 7) {
            uint32_t pacote = (0xB1B1 << 16) | cor;
            multicore_fifo_push_blocking(pacote);
            printf("[MQTT] Comando RGB recebido: %s (código %u)\n", buffer, cor);
        } else {
            printf("[MQTT] Comando RGB inválido: %s\n", buffer);
        }
    }

    // --- 3. Tópico de Cenário (AQUI ESTÁ A MUDANÇA) ---
    else if (strncmp(topico_recebido, TOPICO_CENARIO, strlen(TOPICO_CENARIO)) == 0) {
        uint16_t comando_cenario = 0xFFFF; 

        if      (strcasecmp(buffer, "cena1")   == 0) comando_cenario = 0x01;
        else if (strcasecmp(buffer, "cena2")   == 0) comando_cenario = 0x02;
        else if (strcasecmp(buffer, "cena3")   == 0) comando_cenario = 0x03;
        else if (strcasecmp(buffer, "cena4")   == 0) comando_cenario = 0x04;
        else if (strcasecmp(buffer, "cena5")   == 0) comando_cenario = 0x0A;
        else if (strcasecmp(buffer, "all_off") == 0) comando_cenario = 0x0F;
        else if (strcasecmp(buffer, "cena6")   == 0) comando_cenario = 0x06;

        if (comando_cenario != 0xFFFF) {
            // ALTERAÇÃO: Trocamos 0xC0DE por 0xD1D1 para mensagens que vem da REDE
            uint32_t pacote = (0xD1D1 << 16) | comando_cenario; 
            
            printf("[MQTT_LWIP DEBUG] Enviando para FIFO (REDE): Comando=0x%04X, Valor=0x%04X\n", (pacote >> 16), (pacote & 0xFFFF));
            multicore_fifo_push_blocking(pacote);
            printf("[MQTT] Comando de cenário via rede: %s (código 0x%04X)\n", buffer, comando_cenario);
        } else {
            printf("[MQTT] Gesto de cenário desconhecido: %s\n", buffer);
        }
    }
    else {
        printf("[MQTT] Tópico não tratado: %s\n", topico_recebido);
    }
}

static void mqtt_sub_cb(void *arg, err_t result) {
    const char *topic_name = (const char *)arg; 
    if (result == ERR_OK) {
        printf("[MQTT] Tópico '%s' assinado com sucesso.\n", topic_name ? topic_name : "UNKNOWN");
    } else {
        printf("[MQTT] Falha ao assinar tópico '%s'. Código: %d\n", topic_name ? topic_name : "UNKNOWN", result);
    }
}

// ========================
// CALLBACKS DE CONEXÃO E PUBLICAÇÃO
// ========================

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        exibir_status_mqtt("CONECTADO");
        mqtt_set_inpub_callback(client, mqtt_mensagem_cb, mqtt_dados_cb, NULL);
        
        // Assinaturas mantendo sua lógica de passar o tópico no 'arg'
        mqtt_subscribe(client, TOPICO_COMANDO_RGB,      0, mqtt_sub_cb, (void *)TOPICO_COMANDO_RGB);
        mqtt_subscribe(client, TOPICO_MENSAGEM_OLED,    0, mqtt_sub_cb, (void *)TOPICO_MENSAGEM_OLED);
        mqtt_subscribe(client, TOPICO_CENARIO,          0, mqtt_sub_cb, (void *)TOPICO_CENARIO);
        mqtt_subscribe(client, TOPICO_CONFIG_INTERVALO, 0, mqtt_sub_cb, (void *)TOPICO_CONFIG_INTERVALO);
        
        publicar_online = true;
    } else {
        exibir_status_mqtt("FALHA");
    }
}

static void mqtt_pub_cb(void *arg, err_t result) {
    publicacao_em_andamento = false;
    printf("[MQTT] Publicação finalizada: %s\n", result == ERR_OK ? "OK" : "ERRO");

    uint16_t status = (result == ERR_OK) ? 0 : 1;
    uint32_t pacote = ((0x9999 << 16) | status);
    multicore_fifo_push_blocking(pacote);
}

// ========================
// FUNÇÕES PRINCIPAIS
// ========================

void iniciar_mqtt_cliente() {
    ip_addr_t broker_ip;
    if (!ip4addr_aton(MQTT_BROKER_IP, &broker_ip)) {
        printf("Endereço IP do broker inválido: %s\n", MQTT_BROKER_IP);
        return;
    }

    client = mqtt_client_new();
    if (!client) {
        printf("Erro ao criar cliente MQTT\n");
        return;
    }

    memset(&ci, 0, sizeof(ci));
    ci.client_id = "pico_lwip";
    // Configurações padrão de conexão
    mqtt_client_connect(client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);
}

void publicar_mensagem_mqtt(const char *topico, const char *mensagem) {
    if (!client) {
        printf("[MQTT] Cliente NULL\n");
        exibir_status_mqtt("CLIENTE NULL");
        return;
    }

    if (!mqtt_client_is_connected(client)) {
        printf("[MQTT] MQTT desconectado. Ignorando publicação.\n");
        exibir_status_mqtt("DESCONECTADO");
        return;
    }

    if (publicacao_em_andamento) {
        printf("[MQTT] Publicação anterior ainda não finalizada. Ignorado.\n");
        return;
    }

    err_t err = mqtt_publish(client, topico, mensagem, strlen(mensagem), 0, 0, mqtt_pub_cb, NULL);

    if (err != ERR_OK) {
        printf("[MQTT] Erro ao publicar: %d\n", err);
        exibir_status_mqtt("PUB ERRO");
    } else {
        printf("[MQTT] Publicando: \"%s\" em \"%s\"\n", mensagem, topico);
        publicacao_em_andamento = true;
    }
}

void publicar_online_retain(void) {
    if (!client || !mqtt_client_is_connected(client)) {
        printf("[MQTT] Cliente não conectado. Impossível publicar 'Pico W online'.\n");
        return;
    }

    err_t err = mqtt_publish(client, TOPICO_ONLINE, "Pico W online", strlen("Pico W online"), 0, 1, mqtt_pub_cb, NULL);

    if (err != ERR_OK) {
        printf("[MQTT] Erro ao publicar 'Pico W online': %d\n", err);
        exibir_status_mqtt("PUB ERRO");
    } else {
        printf("[MQTT] Publicado 'Pico W online' com retain.\n");
    }
}

bool cliente_mqtt_ativo(void) {
    return client && mqtt_client_is_connected(client);
}

void mqtt_loop() {
    // Reservado para manutenções futuras
}