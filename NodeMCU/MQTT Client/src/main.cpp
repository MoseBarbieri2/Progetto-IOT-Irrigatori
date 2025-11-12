#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <Arduino.h>

#define WIFI_SSID "unibo-wifi"
#define WIFI_PASSWORD "unibo-wifi"
#define MQTT_HOST "rpi10.local"
#define MQTT_PORT 1883
#define MQTT_PUB_TEST "testTopic"

AsyncMqttClient mqttClient;

unsigned long previousMillis = 0;
const long interval = 10000;

void connectToWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to Wi-Fi.");
    } else {
      Serial.println("\nFailed to connect to Wi-Fi.");
    }
  }
}

void connectToMqtt() {
  if (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged. packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void loop() {
  // Controlla la connessione Wi-Fi e tenta la riconnessione se necessario
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }

  // Controlla la connessione MQTT e tenta la riconnessione se necessario
  if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
    connectToMqtt();
  }

  // Pubblica un messaggio ogni 'interval' millisecondi
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval && mqttClient.connected()) {
    previousMillis = currentMillis;

    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEST, 1, true, "eccolo");
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i\n", MQTT_PUB_TEST, packetIdPub1);
  }
}
