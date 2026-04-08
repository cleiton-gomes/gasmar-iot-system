import cv2
import numpy as np
import threading
import time

# URLs das suas fontes
URL_HILOOK = "rtsp://admin:@senha00@10.0.0.64:554/Streaming/Channels/101"
URL_DRONE  = "rtsp://localhost:8554/drone"

class CameraStream:
    def __init__(self, url, nome):
        self.url = url
        self.nome = nome
        self.ret = False
        self.frame = None
        self.stopped = False
        self.conectado = False
        # Inicia a thread de captura
        threading.Thread(target=self.update, args=(), daemon=True).start()

    def update(self):
        while not self.stopped:
            cap = cv2.VideoCapture(self.url, cv2.CAP_FFMPEG)
            cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            
            # Tenta ler frames enquanto a câmera estiver aberta
            while not self.stopped:
                grabbed, frame = cap.read()
                if grabbed:
                    self.ret = True
                    self.frame = frame
                    self.conectado = True
                else:
                    self.ret = False
                    self.conectado = False
                    break # Sai do loop interno para tentar reconectar o objeto cap
            
            cap.release()
            if not self.stopped:
                print(f"[{self.nome}] Sinal perdido ou offline. Tentando em 3s...")
                time.sleep(3) # Tempo de espera para nova tentativa

    def read(self):
        return self.ret, self.frame

def iniciar_central_gasmar():
    largura, altura = 640, 480
    
    # Inicializa as threads para cada câmera
    cam_solo = CameraStream(URL_HILOOK, "HILOOK")
    cam_ar   = CameraStream(URL_DRONE, "DRONE")

    print("--- CENTRAL GASMAR: Mosaico Resiliente Ativo ---")

    while True:
        # Puxa os dados da Thread da HiLook
        ret1, frame1 = cam_solo.read()
        if ret1 and frame1 is not None:
            img1 = cv2.resize(frame1, (largura, altura))
            cv2.putText(img1, "SOLO - LIVE", (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        else:
            img1 = np.zeros((altura, largura, 3), np.uint8)
            cv2.putText(img1, "HILOOK: RECONECTANDO...", (100, 240), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

        # Puxa os dados da Thread do Drone
        ret2, frame2 = cam_ar.read()
        if ret2 and frame2 is not None:
            img2 = cv2.resize(frame2, (largura, altura))
            cv2.putText(img2, "AR - LIVE", (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
        else:
            img2 = np.zeros((altura, largura, 3), np.uint8)
            cv2.putText(img2, "DRONE: OFFLINE", (180, 240), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

        # Monta o Mosaico
        mosaico = np.hstack((img1, img2))
        cv2.imshow("GASMAR - Central Intelligence", mosaico)

        # Tecla ESC para sair
        if cv2.waitKey(1) == 27:
            break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    iniciar_central_gasmar()