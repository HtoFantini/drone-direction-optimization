#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações do Ponto de Acesso
const char* AP_SSID = "ESP8266_AP";       
const char* AP_PASSWORD = "12345678";    

// Configurações MQTT
const char* MQTT_BROKER = "192.168.4.2"; // acha com ipconfig dps de conectar na rede do esp
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "ESP8266_AP_Client";
const char* MQTT_TOPIC_PUB = "test/esp8266/out";
const char* MQTT_TOPIC_SUB = "test/esp8266/in";
const char* MQTT_TOPIC_LED = "test/esp8266/led";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_ap() {
  Serial.begin(115200);
  Serial.println("\nConfigurando ponto de acesso...");
  
  // Cria o ponto de acesso
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Senha: ");
  Serial.println(AP_PASSWORD);
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT Broker...");
    
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("Conectado!");
      client.subscribe(MQTT_TOPIC_SUB);
      client.subscribe(MQTT_TOPIC_LED);
      Serial.println("Inscrito nos tópicos");
      client.publish(MQTT_TOPIC_PUB, "ESP8266 AP Conectado!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5s...");
      delay(5000);
    }
  }
}

void setup() {
  setup_ap();
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publica mensagem periódica
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 10000) {
    lastMsg = millis();
    String msg = "Tempo: " + String(millis()/1000) + "s";
    client.publish(MQTT_TOPIC_PUB, msg.c_str());
    Serial.println("Enviado: " + msg);
  }
  
}
