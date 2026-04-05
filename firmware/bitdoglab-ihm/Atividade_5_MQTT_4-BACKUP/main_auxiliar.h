// main_auxiliar.h
#ifndef MAIN_AUXILIAR_H
#define MAIN_AUXILIAR_H

#include <stdint.h> // Necessário para uint16_t (usado em processar_comando_cenario)
// #include "fila_circular.h" // Se MensagemWiFi for usada em alguma função declarada aqui
// #include "configura_geral.h" // Se PWM_STEP for usado em alguma função de cena declarada aqui
// #include "oled_utils.h" // Se buffer_oled ou area forem usados nas funções de cena
// #include "ssd1306_i2c.h" // Se buffer_oled ou area forem usados nas funções de cena
// #include "lwip/ip_addr.h" // Se ultimo_ip_bin for usado em alguma função declarada aqui
// #include "estado_mqtt.h" // Se intervalo_ping_ms ou ultimo_ip_bin forem usados

// IMPORTANTE:
// As linhas comentadas acima (#include "fila_circular.h", etc.)
// são para você DESCOMENTAR APENAS SE FOR USAR AS ESTRUTURAS/MACROS/VARIÁVEIS
// DESSES ARQUIVOS DENTRO DOS *PROTÓTIPOS* DAS FUNÇÕES AQUI DECLARADAS.
// No geral, para evitar dependências cíclicas ou desnecessárias,
// tente incluir o mínimo possível de .h aqui.
// Para as funções que já estão no main_auxiliar.c e você quer que o main.c as veja,
// geralmente basta ter o protótipo da função.

// --- Protótipos das funções existentes em main_auxiliar.c que são chamadas em main.c ---
void espera_usb(void);
void tratar_mensagem(MensagemWiFi msg); // Note: MensagemWiFi deve ser definida em um .h incluído aqui ou no main.c
void tratar_ip_binario(uint32_t ip_bin);
void exibir_status_mqtt(const char *texto);
void set_novo_intervalo_ping(uint32_t novo_intervalo);
void mostrar_cor_rgb(uint8_t codigo);

// --- Protótipos das novas funções de cena e de processamento de cenário ---
void exibir_mensagem_cenario_oled(const char* titulo, const char* mensagem); // Função auxiliar para OLED de cenários

void cena_all_off(void);
void cena1(void);
void cena2(void);
void cena3(void);
void cena4(void);
void cena5(void); // Se você usa cena5 para 0x0A
void cena6(void);


// Esta é a função principal de cenário chamada pelo main.c
void processar_comando_cenario(uint16_t comando_cenario);

// --- Protótipos das novas funções de iluminação para cada cena ---
void iluminacao_all_off(void);
void iluminacao_cena1(void);
void iluminacao_cena2(void);
void iluminacao_cena3(void);
void iluminacao_cena4(void);
void iluminacao_cena5(void);
void iluminacao_cena6(void);

#endif // MAIN_AUXILIAR_H