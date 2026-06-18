#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <EEPROM.h>

// definizione EEPROM
#define EEPROM_SIZE 256
#define CONFIG_MAGIC 0xABCD1234
#define MAX_NAME_LEN 32


//DHT sensore epompe
#define DHTTYPE DHT22
#define MAX_PUMPS 5

// --- MQTT Topics
String macAddress;
String topicHello = "greenhouse/sensor/hello";
String topicStatus;
String topicConfig;
String topicCheck;

// --- Config dinamica
String deviceName;
float thresholdMin;
float thresholdMax;
unsigned long updateInterval;
int humidityPin;
bool isConfigured = false;
unsigned long lastUpdate = 0;

// -- Configurazione salvata in EEPROM
struct StoredConfig {
  uint32_t magic;
  char deviceName[MAX_NAME_LEN];
  float thresholdMin;
  float thresholdMax;
  int humidityPin;
  unsigned long updateInterval;
  int pumpPins[MAX_PUMPS];
  int pumpCount;
};

// -- Massimo numero pompe irrigatore
int pumpPins[MAX_PUMPS];
int pumpCount = 0;

// --- DHT dinamico ---
DHT *dht = nullptr;
float simulatedHumidity = 50.0; // valore di partenza, modificalo se vuoi
unsigned long lastHumidityChange = 0;
const unsigned long humidityChangeInterval = 1000; // 1 secondo, fisso

const char *ssid = "Wokwi-GUEST";
const char *pass = "";

// MQTT Broker settings
const char *mqtt_server = "dc8a6f487f8345a28185940816d843d4.s1.eu.hivemq.cloud"; // Public broker for testing; use your broker's address for production
const int mqtt_port = 8883;                                                      // Standard MQTT port
const char *mqtt_user = "connector";                                             // Optional; for brokers requiring authentication
const char *mqtt_password = "moseJacopo1";
const char *topic = "testTopic"; // Specify your topic

const char *greenhouseSensors = "greenhouse/sensor";
WiFiClientSecure espClient;
PubSubClient client(espClient);

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(macAddress.c_str(), mqtt_user, mqtt_password))
    {

      client.subscribe(topicConfig.c_str());
      client.subscribe(topicCheck.c_str());

      client.publish(topicHello.c_str(),
                     (macAddress).c_str());

      Serial.println("MQTT Connected");
    }
    else
    {
      Serial.print("Failed to connect. Error code: ");
      int errorCode = client.state();
      Serial.print(errorCode);

      // Display more detailed error message based on error code
      switch (errorCode)
      {
      case -4:
        Serial.println(" (MQTT_CONNECTION_TIMEOUT)");
        break;
      case -3:
        Serial.println(" (MQTT_CONNECTION_LOST)");
        break;
      case -2:
        Serial.println(" (MQTT_CONNECT_FAILED)");
        break;
      case -1:
        Serial.println(" (MQTT_DISCONNECTED)");
        break;
      case 1:
        Serial.println(" (MQTT_CONNECT_BAD_PROTOCOL)");
        break;
      case 2:
        Serial.println(" (MQTT_CONNECT_BAD_CLIENT_ID)");
        break;
      case 3:
        Serial.println(" (MQTT_CONNECT_UNAVAILABLE)");
        break;
      case 4:
        Serial.println(" (MQTT_CONNECT_BAD_CREDENTIALS)");
        break;
      case 5:
        Serial.println(" (MQTT_CONNECT_UNAUTHORIZED)");
        break;
      default:
        Serial.println(" (Unknown Error)");
        break;
      }

      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void saveConfigToEEPROM()
{
  StoredConfig cfg;
  cfg.magic = CONFIG_MAGIC;
  deviceName.toCharArray(cfg.deviceName, MAX_NAME_LEN);
  cfg.thresholdMin = thresholdMin;
  cfg.thresholdMax = thresholdMax;
  cfg.humidityPin = humidityPin;
  cfg.updateInterval = updateInterval;
  cfg.pumpCount = pumpCount;

  for (int i = 0; i < pumpCount; i++) {
    cfg.pumpPins[i] = pumpPins[i];
  }

  EEPROM.put(0, cfg);
  EEPROM.commit(); // fondamentale su ESP32, altrimenti non scrive davvero
  Serial.println("Configurazione salvata in EEPROM");
}

// Callback function for receiving messages
void callback(char *topicCallBack, byte *payload, unsigned int length)
{

  String message;
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  String incomingTopic = String(topicCallBack);

  Serial.println("Topic: " + incomingTopic);
  Serial.println("Payload: " + message);

  // CONFIG
  if (incomingTopic == topicConfig)
  {

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
      Serial.println("Errore parsing JSON");
      return;
    }

    deviceName = doc["name"].as<String>();
    thresholdMin = doc["thresholdMin"].as<float>();
    thresholdMax = doc["thresholdMax"].as<float>();
    humidityPin = doc["pinHumidity"].as<int>();
    updateInterval = doc["updateInterval"].as<unsigned long>() * 1000; // Conversione in millisecondi
    JsonArray pumps = doc["pinPumps"];
    pumpCount = 0;

    for (JsonVariant v : pumps)
    {
      if (pumpCount < MAX_PUMPS)
      {
        pumpPins[pumpCount] = v.as<int>();
        pinMode(pumpPins[pumpCount], OUTPUT);
        digitalWrite(pumpPins[pumpCount], LOW);
        pumpCount++;
      }
    }

    if (dht != nullptr)
    {
      delete dht;
    }

    dht = new DHT(humidityPin, DHTTYPE);
    dht->begin();

    Serial.println("Configurazione aggiornata");
    isConfigured = true;
    Serial.println("Device configurato → ACTIVE");

    saveConfigToEEPROM();

  }

  // CHECK (ping/pong)
  if (incomingTopic == topicCheck)
  {
    client.publish(topicCheck.c_str(), "pong");
  }
}



bool loadConfigFromEEPROM()
{
  StoredConfig cfg;
  EEPROM.get(0, cfg);

  if (cfg.magic != CONFIG_MAGIC) {
    Serial.println("Nessuna configurazione valida in EEPROM");
    return false;
  }

  deviceName = String(cfg.deviceName);
  thresholdMin = cfg.thresholdMin;
  thresholdMax = cfg.thresholdMax;
  humidityPin = cfg.humidityPin;
  updateInterval = cfg.updateInterval;
  pumpCount = cfg.pumpCount;

  for (int i = 0; i < pumpCount; i++) {
    pumpPins[i] = cfg.pumpPins[i];
    pinMode(pumpPins[i], OUTPUT);
    digitalWrite(pumpPins[i], LOW);
  }

  if (dht != nullptr) {
    delete dht;
  }
  dht = new DHT(humidityPin, DHTTYPE);
  dht->begin();

  Serial.println("Configurazione caricata da EEPROM");
  return true;
}

void setup()
{
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);

  Serial.print("Connecting to WiFi...");
  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");

  // DHT inizializzato dopo config

  macAddress = WiFi.macAddress();
  macAddress.replace(":", "");

  // Costruzione topic dinamici
  topicStatus = "greenhouse/sensor/status/" + macAddress;
  topicConfig = "greenhouse/sensor/config/" + macAddress;
  topicCheck = "greenhouse/sensor/check/" + macAddress;

  if (loadConfigFromEEPROM()) {
    isConfigured = true;
    Serial.println("Device già configurato → ACTIVE da subito");
  }

  espClient.setCACert(root_ca);
  // Set up MQTT client
  Serial.println("Connecting to: " + String(mqtt_server));
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

bool useSimulatedHumidity = true; // put it false to use real DHT 

float readHumidity()
{
  if (useSimulatedHumidity)
  {
    return simulatedHumidity;
  }

  if (dht == nullptr)
    return -1;

  float h = dht->readHumidity();
  if (isnan(h))
    return -1;

  return h;
}

void controlPumps(float humidity)
{

  if (humidity < 0)
    return;

  if (humidity < thresholdMin)
  {
    for (int i = 0; i < pumpCount; i++)
    {
      digitalWrite(pumpPins[i], HIGH);
    }
  }

  if (humidity > thresholdMax)
  {
    for (int i = 0; i < pumpCount; i++)
    {
      digitalWrite(pumpPins[i], LOW);
    }
  }
}

void updateSimulatedHumidity()
{
  if (millis() - lastHumidityChange >= humidityChangeInterval)
  {
    lastHumidityChange = millis();

    bool pumpActive = false;
    for (int i = 0; i < pumpCount; i++)
    {
      if (digitalRead(pumpPins[i]) == HIGH)
      {
        pumpActive = true;
        break;
      }
    }

    simulatedHumidity += pumpActive ? 1 : -1;

    // clamp tra 0 e 100
    if (simulatedHumidity > 100)
      simulatedHumidity = 100;
    if (simulatedHumidity < 0)
      simulatedHumidity = 0;
  }
}

void loop()
{
  // Ensure MQTT connection
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // Publish a test message every 5 seconds
  static unsigned long lastMsg = 0;
  if (!isConfigured)
  {
    return;
  }
  updateSimulatedHumidity();

  if (millis() - lastUpdate > updateInterval)
  {
    lastUpdate = millis();

    float humidity = readHumidity();
    controlPumps(humidity);
    bool pumpActive = false;

    for (int i = 0; i < pumpCount; i++)
    {
      if (digitalRead(pumpPins[i]) == HIGH)
      {
        pumpActive = true;
        break;
      }
    }

    StaticJsonDocument<256> statusDoc;
    statusDoc["name"] = deviceName;
    statusDoc["humidity"] = humidity;
    statusDoc["pumpActive"] = pumpActive;

    String output;
    serializeJson(statusDoc, output);

    client.publish(topicStatus.c_str(), output.c_str());

    Serial.println(output);
  }
}