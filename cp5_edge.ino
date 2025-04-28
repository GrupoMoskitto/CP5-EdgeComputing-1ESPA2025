//integrantes: João Vitor de Matos Araujo;Gabriel Couto;Gabriel Kato; Ana Luiza

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// Configurações Wi-Fi
const char* ssid = "Wokwi-GUEST";  // Nome da rede Wi-Fi
const char* password = "";         // Senha da rede Wi-Fi

// Configurações do Broker MQTT
const char* mqttServer = "20.55.19.44"; // IP do Broker MQTT (Orion Context Broker)
const int mqttPort = 1883;              // Porta do Broker MQTT
const char* mqttUser = "";              // Usuário do MQTT (se necessário)
const char* mqttPassword = "";          // Senha do MQTT (se necessário)
const char* mqttClientID = "fiware_001"; // ID do cliente MQTT

// Tópicos MQTT
const char* topicTemperature = "/TEF/device001/attrs/temperature";
const char* topicHumidity = "/TEF/device001/attrs/humidity";
const char* topicLuminosity = "/TEF/device001/attrs/luminosity";

// Pinos
const int LDR_PIN = 34; // Pino analógico para o sensor LDR
const int DHT_PIN = 21; // Pino digital para o sensor DHT22
const int default_D4 = 2; // Pino do LED onboard

// Inicialização dos sensores
DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Inicia o monitor serial
  Serial.begin(115200);
  
  // Conecta no Wi-Fi
  setupWiFi();

  // Inicializa o sensor DHT22
  dht.setup(DHT_PIN, DHTesp::DHT22);
  
  // Inicializa o cliente MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
  
  // Aguarda a conexão
  delay(5000);
}

void loop() {
  // Mantém a conexão MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Lê os valores dos sensores
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
  int luminosity = analogRead(LDR_PIN);

  // Envia os dados via MQTT
  sendDataToFIWARE(temperature, humidity, luminosity);

  // Aguarda 10 segundos antes de enviar novamente
  delay(10000);
}

void setupWiFi() {
  // Conecta-se à rede Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando no Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Exibe o IP atribuído
  Serial.println();
  Serial.println("Conectado à rede Wi-Fi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Tenta reconectar ao Broker MQTT
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    
    if (client.connect(mqttClientID, mqttUser, mqttPassword)) {
      Serial.println("Conectado ao Broker MQTT");
    } else {
      Serial.print("Falha na conexão, tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Função para implementação futura de receber mensagens/payloads remotos  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Mensagem recebida: ");
  Serial.println(message);
}

void sendDataToFIWARE(float temperature, float humidity, int luminosity) {
  // Formata a mensagem para temperatura e umidade
  String tempMsg = String(temperature);
  String humidityMsg = String(humidity);
  
  // Formata o valor da luminosidade
  int luminosityPercent = map(luminosity, 0, 4095, 0, 100); // Mapeia para percentual (0 a 100)
  String luminosityMsg = String(luminosityPercent);
  
  // Envia os dados de temperatura, umidade e luminosidade via MQTT
  Serial.println("Enviando os dados para o FIWARE...");
  
  client.publish(topicTemperature, tempMsg.c_str());
  client.publish(topicHumidity, humidityMsg.c_str());
  client.publish(topicLuminosity, luminosityMsg.c_str());
  
  Serial.print("Temperatura: ");
  Serial.println(tempMsg);
  Serial.print("Umidade: ");
  Serial.println(humidityMsg);
  Serial.print("Luminosidade: ");
  Serial.println(luminosityMsg);
}
