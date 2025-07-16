import pygame
import paho.mqtt.client as mqtt
import json
import sys
import os
import threading
import time

# Configurações da tela
SCREEN_WIDTH = 1367

SCREEN_HEIGHT = 768

DRONE_SIZE = 40  # Tamanho da imagem do drone

ANTENNA_SIZE = 50
ANTENNA_LAT=-15.779
ANTENNA_LON = -47.929
CIRCLE_GEO_RADIUS = 15.0

# Configurações MQTT

MQTT_BROKER = "192.168.4.2" 

MQTT_PORT = 1883

MQTT_TOPIC = "drone/+/to_antenna"  # Recebe mensagens de todos os drones



MIN_LAT = -90.0

MAX_LAT = 90.0

MIN_LON = -180.0

MAX_LON = 180.0


# Dicionário para armazenar todos os drones

drones = {}  # Formato: {drone_id: {"lat": float, "lon": float, "angle": float}}
drones_lock = threading.Lock()

pygame.init()

screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))

pygame.display.set_caption("Visualizador de Drones")

font = pygame.font.SysFont("Arial", 20)


# Carrear imagens

def load_image(name, size=None):

    try:

        image = pygame.image.load(name)

        if size:

            image = pygame.transform.scale(image, size)

        return image.convert_alpha()

    except pygame.error:

        #  imagem substituta

        surf = pygame.Surface(size or (50, 50), pygame.SRCALPHA)

        pygame.draw.circle(surf, (30, 30, 150), (size[0]//2, size[1]//2), size[0]//3)

        pygame.draw.rect(surf, (100, 100, 100), (size[0]//4, 0, size[0]//2, size[0]//6))

        return surf


#  mapa e drone

try:

    map_image = pygame.image.load("mapa.png")

    map_image = pygame.transform.scale(map_image, (SCREEN_WIDTH, SCREEN_HEIGHT))

except:

    #mapa simples como fallback

    map_image = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))

    map_image.fill((100, 200, 255))  # Céu azul

    pygame.draw.rect(map_image, (50, 180, 70), (0, SCREEN_HEIGHT*2//3, SCREEN_WIDTH, SCREEN_HEIGHT//3))


try:

    drone_image = pygame.image.load("drone.png")

    drone_image = pygame.transform.scale(drone_image, (DRONE_SIZE, DRONE_SIZE))

except:


    drone_image = pygame.Surface((DRONE_SIZE, DRONE_SIZE), pygame.SRCALPHA)

    pygame.draw.circle(drone_image, (30, 30, 150), (DRONE_SIZE//2, DRONE_SIZE//2), DRONE_SIZE//3)  # Corpo

    pygame.draw.rect(drone_image, (100, 100, 100), (DRONE_SIZE//4, 0, DRONE_SIZE//2, DRONE_SIZE//6))  # Hélices


# Callback quando uma mensagem MQTT é recebida

try:
    antenna_image = pygame.image.load("antena.png")
    antenna_image = pygame.transform.scale(antenna_image, (ANTENNA_SIZE, ANTENNA_SIZE))
except:
    antenna_image = pygame.Surface((ANTENNA_SIZE,ANTENNA_SIZE), pygame.SRCALPHA)
    pygame.draw.rect(antenna_image, (150,150,150),(ANTENNA_SIZE//2-2,0,4, ANTENNA_SIZE))

try:
    pixels_per_degree_lat = SCREEN_HEIGHT / (MAX_LAT-MIN_LAT)
    circle_pixel_radius = int(CIRCLE_GEO_RADIUS *pixels_per_degree_lat)
except:
    circle_pixel_radius = 50
    
def on_message(client, userdata, msg):

    try:

        payload = msg.payload.decode()

        data = json.loads(payload)

        

        # Verificar se é uma mensagem de posição

        if data.get("tipo") == "posicao":

            # Extrair o ID do drone do tópico

            topic_parts = msg.topic.split('/')

            drone_id = topic_parts[1] if len(topic_parts) > 1 else "unknown"

            

            # Criar novo drone se não existir

            if drone_id not in drones:

                print(f"Novo drone detectado: {drone_id}")

            

            # Atualizar dados do drone
            with drones_lock:
                drones[drone_id] = {

                    "lat": data["dados"]["lat"],

                    "lon": data["dados"]["lon"],

                    "angle": data["dados"]["angulo"],
                    
                    "last_update": time.time()
            }

            

    except Exception as e:

        print(f"Erro na mensagem: {str(e)}")


# Função para mapear coordenadas para pixels na tela

def map_coords_to_screen(lat, lon):

    try:

        # Normaliza as coordenadas para o intervalo [0, 1]

        normalized_x = (lon - MIN_LON) / (MAX_LON - MIN_LON)

        normalized_y = 1.0 - ((lat - MIN_LAT) / (MAX_LAT - MIN_LAT))  # Invertido para Y

        

        # Converte para coordenadas de tela

        screen_x = int(normalized_x * SCREEN_WIDTH)

        screen_y = int(normalized_y * SCREEN_HEIGHT)

        

        return screen_x, screen_y

    except:

        # Retornar centro da tela em caso de erro

        return SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2


# Configurar cliente MQTT

client = mqtt.Client()

client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)

client.subscribe(MQTT_TOPIC)

client.loop_start()


# Loop principal

running = True

clock = pygame.time.Clock()


while running:

    for event in pygame.event.get():

        if event.type == pygame.QUIT:

            running = False

        elif event.type == pygame.KEYDOWN:

            if event.key == pygame.K_ESCAPE:

                running = False

    
    current_time = time.time()
    #with drones_lock:
        #inactive_drones = [
         #   drone_id for drone_id, data in drones.itens()
          #  if current_time - data.get("last update", 0)>5
        #]
        
        #for drone_if in inactive_drones:
        #    del drones[drone_id]
        
    with drones_lock:
        drones_copy = drones.copy()
        
    # Desenha o mapa de fundo

    screen.blit(map_image, (0, 0))

    center_x, center_y = map_coords_to_screen(ANTENNA_LAT, ANTENNA_LON)
    pygame.draw.circle(screen, (255,100,100), (center_x,center_y), circle_pixel_radius,2)
    antenna_rect = antenna_image.get_rect(center=(center_x,center_y))
    screen.blit(antenna_image, antenna_rect)

    # Desenha todos os drones

    for drone_id, drone_data in drones_copy.items():

        if "lat" in drone_data and "lon" in drone_data:

            screen_x, screen_y = map_coords_to_screen(

                drone_data["lat"], 

                drone_data["lon"]

            )

            

            # Desenha a imagem do drone

            drone_rect = drone_image.get_rect(center=(screen_x, screen_y))

            screen.blit(drone_image, drone_rect)

            

            # Desenha o ID do drone acima da imagem

            id_text = font.render(drone_id, True, (255, 255, 255))

            id_bg = pygame.Rect(

                screen_x - id_text.get_width() // 2 - 5,

                screen_y - 40,

                id_text.get_width() + 10,

                id_text.get_height() + 5

            )

            pygame.draw.rect(screen, (0, 0, 0, 180), id_bg, border_radius=3)

            screen.blit(id_text, (id_bg.x + 5, id_bg.y + 3))

    

    # contagem de drones

    count_text = font.render(f"Drones ativos: {len(drones)}", True, (255, 255, 255))

    screen.blit(count_text, (10, 10))

    

    # Atualiza a tela

    pygame.display.flip()

    clock.tick(30)  # Limita a 30 FPS


# Encerra

client.loop_stop()

pygame.quit()

sys.exit() 
