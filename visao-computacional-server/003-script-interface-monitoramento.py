import cv2
import numpy as np
import threading
import time
from datetime import datetime

# Configurações das Fontes (URLs)
URL_HILOOK = "rtsp://admin:@senha00@10.0.0.64:554/Streaming/Channels/101"
URL_DRONE  = "rtsp://localhost:8554/drone"

class CameraStream:
    def __init__(self, url, nome):
        self.url = url
        self.nome = nome
        self.ret = False
        self.frame = None
        self.stopped = False
        # Inicia a thread de captura em segundo plano
        threading.Thread(target=self.update, args=(), daemon=True).start()

    def update(self):
        while not self.stopped:
            cap = cv2.VideoCapture(self.url, cv2.CAP_FFMPEG)
            cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            
            while not self.stopped:
                grabbed, frame = cap.read()
                if grabbed:
                    self.ret = True
                    self.frame = frame
                else:
                    self.ret = False
                    break # Sai do loop para tentar reconectar
            
            cap.release()
            if not self.stopped:
                time.sleep(3) # Espera antes de tentar reconectar

    def read(self):
        return self.ret, self.frame

def iniciar_central_gasmar_v1():
    largura, altura = 640, 480
    
    # Inicializa as conexões
    cam_solo = CameraStream(URL_HILOOK, "HILOOK")
    cam_ar   = CameraStream(URL_DRONE, "DRONE")

    print("--- CENTRAL GASMAR V1.0: MONITORAMENTO ATIVO ---")

    while True:
        # 1. Puxa os dados atuais das threads
        ret1, frame1 = cam_solo.read()
        ret2, frame2 = cam_ar.read()

        # 2. Processa Lado Esquerdo (Solo)
        if ret1 and frame1 is not None:
            img1 = cv2.resize(frame1, (largura, altura))
            status_solo, cor_solo = "ESTAVEL", (0, 255, 0) # Verde
        else:
            img1 = np.zeros((altura, largura, 3), np.uint8)
            status_solo, cor_solo = "RECONECTANDO...", (0, 0, 255) # Vermelho
            cv2.putText(img1, "SEM SINAL HILOOK", (180, 240), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 2)

        # 3. Processa Lado Direito (Drone)
        if ret2 and frame2 is not None:
            img2 = cv2.resize(frame2, (largura, altura))
            status_ar, cor_ar = "ESTAVEL", (0, 255, 0) # Verde
        else:
            img2 = np.zeros((altura, largura, 3), np.uint8)
            status_ar, cor_ar = "OFFLINE", (0, 0, 255) # Vermelho
            cv2.putText(img2, "AGUARDANDO DRONE...", (180, 240), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 2)

        # 4. Montagem do Visual
        mosaico_video = np.hstack((img1, img2))
        
        # Cria a Barra de Status Inferior (60px de altura)
        barra_status = np.zeros((60, largura * 2, 3), np.uint8)
        agora = datetime.now().strftime("%d/%m/%Y %H:%M:%S")

        cv2.putText(barra_status, f"SISTEMA SOLO: {status_solo}", (20, 35), cv2.FONT_HERSHEY_SIMPLEX, 0.6, cor_solo, 2)
        cv2.putText(barra_status, agora, (540, 35), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)
        cv2.putText(barra_status, f"SISTEMA AR: {status_ar}", (950, 35), cv2.FONT_HERSHEY_SIMPLEX, 0.6, cor_ar, 2)

        # Empilha tudo (Vídeo em cima, Barra embaixo)
        final_frame = np.vstack((mosaico_video, barra_status))

        cv2.imshow("GASMAR - INTELLIGENCE COMMAND CENTER --- (PRESSIONE 'Esc' PARA SAIR)", final_frame)

        if cv2.waitKey(1) == 27: # ESC
            break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    iniciar_central_gasmar_v1()