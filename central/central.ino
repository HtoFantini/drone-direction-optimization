#include "sMQTTBroker.h"

sMQTTBroker broker;

// Use IP na mesma faixa do seu computador
IPAddress local_IP(192, 168, 56, 1);  // Mude para sua faixa de rede
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);    // Máscara correta (/24)

void setup() {
  delay(2000);
  Serial.begin(9600);  // Aumente para 115200
  Serial.println("\n\n--- Iniciando Broker MQTT ---");

  // 1. Configuração WiFi
  WiFi.disconnect(true);
  delay(1000);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);  // Fundamental!

  // 2. Configure seu WiFi corretamente
  const char* ssid = "BLFIBRA URUBU DO PIX ";  // Remova espaço final!
  const char* password = "apto502blc06";

  // 3. Configuração IP
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("[ERRO] Falha na configuração IP");
  } else {
    Serial.println("[INFO] IP estático configurado");
  }

  // 4. Conexão WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 30000) {
      Serial.println("\n[ERRO] Timeout WiFi!");
      ESP.restart();
    }
    Serial.print(".");
    delay(500);
  }
  
  // 5. Informações de rede
  Serial.println("\nConectado!");
  Serial.println("SSID: " + WiFi.SSID());
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Máscara: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // 6. Inicialização do Broker
  Serial.println("Iniciando broker MQTT...");
  
  if (broker.init(1883)) {
    Serial.println("Broker iniciado na porta 1883");
    Serial.print("Endereço do broker: ");
    Serial.print(WiFi.localIP());
    Serial.println(":1883");
  } else {
    Serial.println("[ERRO] Falha ao iniciar broker");
  }
}

void loop() {
  broker.update();
}