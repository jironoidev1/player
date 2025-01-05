#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>

//=================================================================================================

// String username = "";

//=================================================================================================
// WiFi และ MQTT Configuration
const char* ssid = "Pak24";
const char* password = "xit9^ohe";
const char* mqtt_broker = "broker.emqx.io";
const char* mqtt_username = "Pak";
const char* mqtt_password = "park1234";
const int mqtt_port = 1883;
const char* topic_player = "Game24/player5";
// const char* topic_username = "TOPIC_USERNAME";

WiFiClient espClient;
PubSubClient client(espClient);

//=================================================================================================

#define pin1 2
#define pin2 5
#define DEBOUNCE_TIME 12

int lastSteadyState1 = LOW;
int lastSteadyState2 = LOW;
int lastFlickerableState1 = LOW;
int lastFlickerableState2 = LOW;
int currentState1;
int currentState2;

unsigned long lastDebounceTime = 0;

//=================================================================================================

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//=================================================================================================

// void callback(char *topic, byte *payload, unsigned int length) {//รับจาก node-red
//   Serial.print("Message arrived in topic: ");
//   Serial.println(topic);

//   String message = "";
//   for (int i = 0; i < length; i++) {
//     message += (char)payload[i];
//   }
//   Serial.print("Message: ");
//   Serial.println(message);
//   Serial.println("-----------------------");

//   if (strcmp(topic, topic_username) == 0) {
//     username = message;
//   }
// }

//=================================================================================================

void reconnect() {
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public MQTT broker connected");
      client.subscribe(topic_player);
    } else {
      Serial.print("Failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

//=================================================================================================

void setup() {

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_broker, mqtt_port);
  // client.setCallback(callback);
  reconnect();

  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);

}

//=================================================================================================

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  currentState1 = digitalRead(pin1);
  currentState2 = digitalRead(pin2);

  if (currentState1 != lastFlickerableState1 || currentState2 != lastFlickerableState2) {
    lastDebounceTime = millis();
    lastFlickerableState1 = currentState1;
    lastFlickerableState2 = currentState2;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) 
  {  
    if (lastSteadyState1 == HIGH && currentState1 == LOW){
      Serial.println("The button1 is pressed");
      client.publish(topic_player, "The button1 is pressed");
      digitalWrite(pin1, HIGH);
      delay(5000);
      client.publish(topic_player, " ");
    }

    if (lastSteadyState2 == HIGH && currentState2 == LOW){
      Serial.println("The button2 is pressed");
      client.publish(topic_player, "The button2 is pressed");
      digitalWrite(pin2, HIGH);
      delay(5000);
      client.publish(topic_player, " ");
    }

    lastSteadyState1 = currentState1;
    lastSteadyState2 = currentState2;

    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    
  }

  client.loop();
}
