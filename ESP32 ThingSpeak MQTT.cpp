#include <LiquidCrystal_I2C.h>
#include <DHTesp.h>
#include <stdio.h>
#include <math.h>
#include "WiFi.h"
#include "HTTPClient.h"
#include "PubSubClient.h"

const int pino_led_vermelho = 14; 
const int pino_led_azul = 4;
const int pinoSensor = 27;

// nome da rede wifi e senha
const char* nome_rede = "Wokwi-GUEST";
const char* senha = "";

int field1;
int field2;
char url[200];

DHTesp sensor;

LiquidCrystal_I2C display(0x27, 16,2);

// parametros do conexao MQTT
const char* mqttServer = "broker.hivemq.com"; 
const int port = 1883;
const char* topic= "/ESP32-ThingSpeak-MQTT";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char tempStr[8];
char umidStr[8];

void setup() {
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  pinMode(pino_led_vermelho, OUTPUT);
  pinMode(pino_led_azul, OUTPUT);

  display.init();   //setup do display
 
  sensor.setup(pinoSensor, DHTesp::DHT22);  //setup do sensor

  //Conectando a rede wifi
  WiFi.begin(nome_rede, senha);

  Serial.print("Conectando a Rede WiFi");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado a Rede");

  //configuracoes para broker mqtt 
	Serial.println("Conectando ao broker"); 
	mqttClient.setServer(mqttServer, port); 

	while (!mqttClient.connected()){ 
		String clientId = "EspClient-"; 
		clientId += String(random(0xffff), HEX);
		mqttClient.connect(clientId.c_str());
  }
  Serial.println("Conectado ao broker");
}

void loop() {
  HTTPClient httpClient;
  display.setCursor(0,0);

  float temp = sensor.getTemperature();
  float umidade = sensor.getHumidity();
  display.print(temp);
  display.print(" graus");
  display.setCursor(0,8);
  display.print(umidade);
  display.print(" % umidade");

  //Converter o valor float para int
  field1 = (int)temp;
  field2 = (int)umidade;

  //Criacao da URL usando snprintf para concatenar os valores
    snprintf(url, sizeof(url), "https://api.thingspeak.com/update?api_key=FWGXT86FNXO78MQL&field1=%d&field2=%d", field1, field2);

  httpClient.begin(url);

  int httpCode = httpClient.GET();

  Serial.println(httpCode);

  if (temp > 35) {
    digitalWrite(pino_led_vermelho, HIGH); // Acionar led vermelho
    } else {
    digitalWrite(pino_led_vermelho, LOW); // Desligar led vermelho
  }

  if (umidade > 70) {
    digitalWrite(pino_led_azul, HIGH); // Acionar led azul
  } else {
    digitalWrite(pino_led_azul, LOW); // Desligar led azul
  }

  Serial.println(temp);
  Serial.println(umidade);
  printf("\n");

  // Converter os valores para strings
  dtostrf(temp, 6, 2, tempStr);
  dtostrf(umidade, 6, 2, umidStr);

// Criar payload no formato JSON
  char payload[64];
  snprintf(payload, sizeof(payload), "{\"Temperatura\": %s, \"Umidade\": %s}", tempStr, umidStr);

  // Publicar a mensagem JSON no topico
  mqttClient.publish(topic, payload);

  delay(5000); // Repete o processo a cada 5 segundos
}