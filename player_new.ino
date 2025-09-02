#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wpa2.h"

//=================================================================================================

// String username = "";

//=================================================================================================
// WiFi และ MQTT Configuration
const char* ssid = "@SD";
const char* username = "ajima.n@sd.ac.th"; 
const char* password = "Ajima0852394403";
const char* mqtt_broker = "broker.emqx.io";
const char* mqtt_username = "Pak";
const char* mqtt_password = "park1234";
const int mqtt_port = 1883;
const char* topic_status = "Game24/status";
const char* topic_player = "Game24/player5";

String status1 = "9";
String status2 = "10";

WiFiClient espClient;
PubSubClient client(espClient);

//=================================================================================================

#define pin1 15
#define pin2 4
#define DEBOUNCE_TIME 20

int lastSteadyState1 = LOW;
int lastSteadyState2 = LOW;
int lastFlickerableState1 = LOW;
int lastFlickerableState2 = LOW;
int currentState1;
int currentState2;

bool logic = true;
bool logic1 = false;
bool logic2 = false;

unsigned long lastDebounceTime = 0;

//=================================================================================================

void setup_wifi() {
  // Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(ssid);
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("\nWiFi connected");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  Serial.print("connect to : ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  // Set ESP32 to station mode

  // WPA2 Enterprise configuration
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
  esp_wifi_sta_wpa2_ent_enable(); // Enable WPA2 Enterprise

  // Connect to Wi-Fi
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

//=================================================================================================

void callback(char *topic, byte *payload, unsigned int length) {//รับจาก node-red
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message: ");
  Serial.println(message);
  Serial.println("-----------------------");

  if (strcmp(topic, topic_status) == 0) {
    if(message == status1){
      logic1 = true;
      Serial.println(message);
    }
    if(message == status2){
      logic2 = true;
      Serial.println(message);
    }
  }
}

//=================================================================================================

void reconnect() {
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public MQTT broker connected");
      client.subscribe(topic_status);
      digitalWrite(2,HIGH);
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
  client.setCallback(callback);
  reconnect();

  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(2,OUTPUT);

}

//=================================================================================================

void loop() {
  if (!client.connected()) {
    digitalWrite(2,LOW);
    reconnect();
  }

  currentState1 = digitalRead(pin1);
  currentState2 = digitalRead(pin2);

  if(logic1){
    digitalWrite(pin1, HIGH);
    Serial.println("led1");
    delay(5000);
    logic1 = false;
  }

  if(logic2){
    digitalWrite(pin2, HIGH);
    Serial.println("led2");
    delay(5000);
    logic2 = false;
  }

  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);

  if (currentState1 != lastFlickerableState1 || currentState2 != lastFlickerableState2) {
    lastDebounceTime = millis();
    lastFlickerableState1 = currentState1;
    lastFlickerableState2 = currentState2;
  }

  if (logic){
    lastDebounceTime = 0;
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) 
    {  
      if (lastSteadyState1 == HIGH && currentState1 == LOW){
        Serial.println("The button1 is pressed");
        client.publish(topic_player, "1");
        
        logic = false;
        Serial.println("reset");
        delay(1000);
      }

      if (lastSteadyState2 == HIGH && currentState2 == LOW){
        Serial.println("The button2 is pressed");
        client.publish(topic_player, "2");
        
        logic = false;
        Serial.println("reset");
        delay(1000);
      }

      lastSteadyState1 = currentState1;
      lastSteadyState2 = currentState2;
      
    }
  }
  else{
    logic = true;
    
  }

  client.loop();
}
