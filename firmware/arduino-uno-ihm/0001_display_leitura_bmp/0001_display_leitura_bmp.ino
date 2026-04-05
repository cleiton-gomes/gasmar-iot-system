#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SD.h>
#include <SPI.h>

MCUFRIEND_kbv tft;

// --- PROTÓTIPOS ---
void bmpDraw(char *filename, int x, int y);
uint16_t read16(File f);
uint32_t read32(File f);

#define SD_CS 10 

void setup() {
    // 115200 é a velocidade ideal para falar com o Raspberry sem atraso
    Serial.begin(115200); 
    
    uint16_t ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(1); // Modo Paisagem
    tft.fillScreen(0x0000); 

    if (!SD.begin(SD_CS)) {
        Serial.println("Erro no SD!");
        return;
    }

    // Carrega sua imagem de fundo
    bmpDraw("logo.bmp", 0, 0); 
    
    Serial.println("Arduino Pronto e aguardando Raspberry...");
}

void loop() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();

        // Lista de comandos válidos para evitar que ruídos apaguem a tela
        if (cmd == '1' || cmd == '2' || cmd == 'F' || cmd == 'U' || 
            cmd == 'D' || cmd == 'L' || cmd == 'R') {
            
            tft.setTextSize(2);
            
            // Limpa a faixa inferior apenas quando um comando novo chega
            tft.fillRect(0, 200, 320, 40, 0x0000); 
            tft.setCursor(10, 215);

            switch (cmd) {
                case '1':
                    tft.setTextColor(0x07FF); // Ciano
                    tft.print("BOTAO A: CENA 1");
                    break;
                case '2':
                    tft.setTextColor(0x07E0); // Verde
                    tft.print("BOTAO B: CENA 2");
                    break;
                case 'F':
                    tft.setTextColor(0xF800); // Vermelho
                    tft.print("SISTEMA: ALL OFF");
                    break;
                case 'U':
                    tft.setTextColor(0xFFFF); // Branco
                    tft.print("JOYSTICK: CIMA");
                    break;
                case 'D':
                    tft.setTextColor(0xFFFF); // Branco
                    tft.print("JOYSTICK: BAIXO");
                    break;
                case 'L':
                    tft.setTextColor(0xFEE0); // Amarelo
                    tft.print("JOYSTICK: ESQUERDA");
                    break;
                case 'R':
                    tft.setTextColor(0xFEE0); // Amarelo
                    tft.print("JOYSTICK: DIREITA");
                    break;
            }
        }
        // Se quiser um comando específico para limpar tudo
        else if (cmd == 'C') {
            tft.fillRect(0, 200, 320, 40, 0x0000);
        }
    }
}

// ============================================================
// FUNÇÃO bmpDraw (Otimizada para carregar o fundo)
// ============================================================
void bmpDraw(char *filename, int x, int y) {
    File bmpFile;
    int bmpWidth, bmpHeight;
    uint8_t bmpDepth;
    uint32_t bmpImageoffset;
    uint32_t rowSize;
    uint8_t sdbuffer[3 * 80]; 
    uint8_t buffidx = sizeof(sdbuffer);
    boolean flip = true;
    int w, h, row, col;
    uint8_t r, g, b;
    uint32_t pos = 0;

    if ((x >= tft.width()) || (y >= tft.height())) return;
    if ((bmpFile = SD.open(filename)) == NULL) return;

    if (read16(bmpFile) == 0x4D42) { 
        read32(bmpFile); read32(bmpFile); 
        bmpImageoffset = read32(bmpFile);
        read32(bmpFile); 
        bmpWidth = read32(bmpFile);
        bmpHeight = read32(bmpFile);
        if (read16(bmpFile) == 1) {
            bmpDepth = read16(bmpFile);
            if (bmpDepth == 24 && read32(bmpFile) == 0) {
                rowSize = (bmpWidth * 3 + 3) & ~3;
                if (bmpHeight < 0) { bmpHeight = -bmpHeight; flip = false; }
                w = bmpWidth; h = bmpHeight;
                if ((x + w - 1) >= tft.width())  w = tft.width() - x;
                if ((y + h - 1) >= tft.height()) h = tft.height() - y;

                for (row = 0; row < h; row++) {
                    if (flip) pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
                    else      pos = bmpImageoffset + row * rowSize;
                    if (bmpFile.position() != pos) {
                        bmpFile.seek(pos);
                        buffidx = sizeof(sdbuffer);
                    }
                    for (col = 0; col < w; col++) {
                        if (buffidx >= sizeof(sdbuffer)) {
                            bmpFile.read(sdbuffer, sizeof(sdbuffer));
                            buffidx = 0;
                        }
                        b = sdbuffer[buffidx++];
                        g = sdbuffer[buffidx++];
                        r = sdbuffer[buffidx++];
                        tft.drawPixel(x + col, y + row, tft.color565(r, g, b));
                    }
                }
            }
        }
    }
    bmpFile.close();
}

uint16_t read16(File f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read();
    ((uint8_t *)&result)[1] = f.read();
    return result;
}

uint32_t read32(File f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read();
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read();
    return result;
}