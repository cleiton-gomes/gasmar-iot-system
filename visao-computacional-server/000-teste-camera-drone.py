import cv2

# Endereço que o MediaMTX gera
RTSP_URL = "rtsp://localhost:8554/drone"

def iniciar_servidor_visao():
    # CAP_FFMPEG é o driver mais rápido para o seu Windows
    cap = cv2.VideoCapture(RTSP_URL, cv2.CAP_FFMPEG)
    
    # BUFFERSIZE 1 impede que o vídeo fique "atrasado" se o PC travar um pouco
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

    if not cap.isOpened():
        print("Erro: Não foi possível conectar ao MediaMTX.")
        print("Verifique se o drone está transmitindo.")
        return

    print("--- GASMAR VISION SERVER ATIVO ---")
    print("Pressione 'ESC' para fechar.")

    while True:
        ret, frame = cap.read()
        
        if not ret:
            print("Aguardando frames...")
            continue

        # --- AQUI VAI ENTRAR O YOLO NO PRÓXIMO PASSO ---
        # Por enquanto, vamos apenas colocar um título na imagem
        cv2.putText(frame, "GASMAR - DRONE LIVE", (30, 50), 
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # Mostra a imagem na tela do Windows
        cv2.imshow("Vision Intelligence Service", frame)

        # Tecla 27 é o ESC
        if cv2.waitKey(1) == 27:
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    iniciar_servidor_visao()