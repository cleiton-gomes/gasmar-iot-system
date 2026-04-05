import serial
import time

# --- CONFIGURAÇÃO DAS PORTAS ---
PORTA_PICO = 'COM16'    
PORTA_ARDUINO = 'COM13' 
BAUD_RATE = 115200

try:
    # Inicia as conexões
    pico = serial.Serial(PORTA_PICO, BAUD_RATE, timeout=0.1)
    arduino = serial.Serial(PORTA_ARDUINO, BAUD_RATE, timeout=0.1)
    
    print("===============================================")
    print("       PONTE SERIAL HÍBRIDA (JOY + MQTT)       ")
    print(f"   Lendo: {PORTA_PICO} | Enviando: {PORTA_ARDUINO}")
    print("===============================================")
    print("Aguardando comandos físicos ou via rede...")

except Exception as e:
    print(f"ERRO DE CONEXÃO: {e}")
    exit()

while True:
    try:
        if pico.in_waiting > 0:
            linha = pico.readline().decode('utf-8', errors='ignore').strip()
            
            if linha:
                # Mostra no console para debug
                print(f"[PICO W] -> {linha}")

                # --- 1. TRADUÇÃO DE COMANDOS FÍSICOS E MQTT (UNIFICADO) ---
                
                # CENA 1 / BOTÃO A
                if "Cena 1" in linha or "Botão A" in linha or "0x01" in linha:
                    arduino.write(b'1')
                    print("   >>> Comando: CENA 1 enviado ao Arduino")
                
                # CENA 2 / BOTÃO B
                elif "Cena 2" in linha or "Botão B" in linha or "0x02" in linha:
                    arduino.write(b'2')
                    print("   >>> Comando: CENA 2 enviado ao Arduino")
                
                # CENA 3 / DIREITA
                elif "Cena 3" in linha or "Direita" in linha or "0x03" in linha:
                    arduino.write(b'R')
                    print("   >>> Comando: CENA 3 (DIREITA) enviado ao Arduino")
                
                # CENA 4 / ESQUERDA
                elif "Cena 4" in linha or "Esquerda" in linha or "0x04" in linha:
                    arduino.write(b'L')
                    print("   >>> Comando: CENA 4 (ESQUERDA) enviado ao Arduino")

                # CENA 5 / CIMA (0x0A)
                elif "Cena 5" in linha or "Cima" in linha or "0x0A" in linha:
                    arduino.write(b'U') # 'U' de Up
                    print("   >>> Comando: CENA 5 (CIMA) enviado ao Arduino")

                # CENA 6 / BAIXO (0x06)
                elif "Cena 6" in linha or "Baixo" in linha or "0x06" in linha:
                    arduino.write(b'6') # Enviando '6' especificamente
                    print("   >>> Comando: CENA 6 (BAIXO) enviado ao Arduino")
                
                # ALL OFF
                elif "ALL OFF" in linha or "0x0F" in linha:
                    arduino.write(b'F')
                    print("   >>> Comando: ALL OFF enviado ao Arduino")

        time.sleep(0.01)

    except KeyboardInterrupt:
        print("\n[SISTEMA] Encerrando ponte...")
        pico.close()
        arduino.close()
        break
    except Exception as e:
        print(f"\n[ERRO]: {e}")
        break