# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(MQTT_4 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# --- Início da integração da Biblioteca LabNeoPixel ---


# Add executable. Default name is the project name, version 0.1

add_executable(MQTT_4 main.c main_auxiliar.c
        WIFI_/fila_circular.c
        WIFI_/rgb_pwm_control.c
        WIFI_/conexao.c
        OLED_/display.c
        OLED_/oled_utils.c
        OLED_/ssd1306_i2c.c
        OLED_/setup_oled.c
        WIFI_/mqtt_lwip.c
        estado_mqtt.c
        LabNeoPixel/util.c
        LabNeoPixel/neopixel_driver.c
        LabNeoPixel/efeitos.c
        LabNeoPixel/testes_cores.c  
        LabNeoPixel/efeito_curva_ar.c    
        LabNeoPixel/numeros_neopixel.c   
        )

pico_set_program_name(MQTT_4 "MQTT_4")
pico_set_program_version(MQTT_4 "0.1")

# Geração do cabeçalho PIO para ws2818b.pio, igual ao NeoControlLab.
# Isso cria um arquivo .h (ex: ws2818b.pio.h) na pasta de build que seu código C pode incluir.
pico_generate_pio_header(MQTT_4 ${CMAKE_CURRENT_LIST_DIR}/LabNeoPixel/ws2818b.pio)

# --- Fim da integração da Biblioteca LabNeoPixel ---


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(MQTT_4 0)
pico_enable_stdio_usb(MQTT_4 1)

# Add the standard library to the build
target_link_libraries(MQTT_4
        pico_stdlib
        pico_multicore
        pico_sync
        hardware_pwm
        pico_cyw43_arch_lwip_threadsafe_background
        hardware_i2c
        pico_lwip_mqtt
        # Adicione as bibliotecas de hardware do NeoControlLab que são relevantes para o NeoPixel
        hardware_pio    # Essencial para o PIO
        hardware_clocks # Importante para controle de clocks e tempo
        hardware_adc    # Descomente se precisar de ADC para algum efeito NeoPixel
        hardware_dma    # Descomente se precisar de DMA para otimização do NeoPixel
        )

# Add the standard include files to the build
target_include_directories(MQTT_4 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/WIFI_
        ${CMAKE_CURRENT_LIST_DIR}/OLED_
        # Adicione o diretório da LabNeoPixel para encontrar os cabeçalhos (.h)
        ${CMAKE_CURRENT_LIST_DIR}/LabNeoPixel
)

# Add any user requested libraries
#target_link_libraries(MQTT_4)

pico_add_extra_outputs(MQTT_4)

