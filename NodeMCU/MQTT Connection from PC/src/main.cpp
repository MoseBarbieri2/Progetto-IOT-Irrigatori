#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino.h>

const char* ssid = "Wokwi-GUEST";
const char* pass = "";

// MQTT Broker settings
const char* mqtt_server = "dc8a6f487f8345a28185940816d843d4.s1.eu.hivemq.cloud";  // Public broker for testing; use your broker's address for production
const int mqtt_port = 8883;                     // Standard MQTT port
const char* mqtt_user = "connector";             // Optional; for brokers requiring authentication
const char* mqtt_password = "moseJacopo1";
const char* topic = "testTopic";               // Specify your topic

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

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT!");
      client.subscribe(topic);
    } else {
      Serial.print("Failed to connect. Error code: ");
      int errorCode = client.state();
      Serial.print(errorCode);
      
      // Display more detailed error message based on error code
      switch (errorCode) {
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

// Callback function for receiving messages
void callback(char* topicCallBack, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.print(topicCallBack);
 
  
   Serial.print(". Message: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];  // Convert *byte to string
  }
  Serial.print(message);
  String atTopic = String(topicCallBack);
  Serial.println(atTopic);
  if(atTopic.equals("ledone")){
    Serial.println("Equa");
    if(message.equals("1")){
      Serial.println("equa 1");
    }
    if(message.equals("2")){
      Serial.println("equa 2");
    }
    int numberMessage = message.toInt();
    switch(numberMessage){
      case 1:
        Serial.println("equa 111");
      break;
      case 2:
      Serial.println("equa 222");
      break;
      case 3:
      Serial.println("equa 333");
      break;
      case 4:
      Serial.println("equa 444");
      break;
      case 5:
      Serial.println("equa 555");
      break;
      case 6:
      Serial.println("equa 666");
      break;
    }
  }
  Serial.println();
}

void setup(){
  Serial.begin(115200);
  
  Serial.print("Connecting to WiFi...");
  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");

  espClient.setCACert(root_ca);
  // Set up MQTT client
  Serial.println("Connecting to: " + String(mqtt_server));
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop(){
  // Ensure MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish a test message every 5 seconds
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    String message = "Hello from ESP8266!";
    client.publish(topic, message.c_str());
    Serial.println("Message published");
  }
}