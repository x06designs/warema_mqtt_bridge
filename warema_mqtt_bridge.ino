#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Include your RCSwitchWarema extension
#include "RCSwitchWarema.h"

// Include a file containing:
//   extern const char* ssid;
//   extern const char* password;
//   extern const char* mqtt_username;
//   extern const char* mqtt_password;
// This file is not shown here for security reasons.
#include "secure.h"

// =========================
// Pin definitions (ESP32)
// =========================
#define LED_WIFI_RED      5    // Adjust to your wiring
#define LED_WIFI_YELLOW   14   // Adjust to your wiring
#define TRANSMITTER_PIN   27   // Adjust to your wiring for 433 MHz transmitter

// =========================
// MQTT Broker & Topics
// =========================
// Replace as needed. If you do not have mDNS set up, consider using a static IP like "192.168.1.123".
const char* mqtt_broker = "homeassistant.local";
const int   mqtt_port   = 1883;

const char* topic                 = "cmnd/rf-warema-bridge/data";
const char* topic_state_connection= "stat/rf-warema-bridge/connection";

// Global objects
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);
RCSwitchWarema mySwitch;

// Function forward-declarations
void callback(char* topic, byte* payload, unsigned int length);
void connectMQTTClient();
void sendMQTTClientInfos();
void writeConnected(bool state);

// =========================
// Setup
// =========================
void setup() {
  // Start serial
  Serial.begin(115200);
  delay(100);

  // Initialize LED pins
  pinMode(LED_WIFI_RED, OUTPUT);
  pinMode(LED_WIFI_YELLOW, OUTPUT);

  // Initially, consider "not connected"
  writeConnected(false);

  // Set up the transmitter pin for RCSwitch
  // This calls our custom class method
  mySwitch.enableTransmit(TRANSMITTER_PIN);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up MQTT
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(callback);

  // Connect to MQTT
  connectMQTTClient();
}

// =========================
// Loop
// =========================
void loop() {
  // Indicate MQTT connection state via LED
  writeConnected(mqttClient.connected());

  // If client is disconnected, try to reconnect
  if (!mqttClient.loop()) {
    connectMQTTClient();
  }
}

// =========================
// MQTT: Connect & Reconnect
// =========================
void connectMQTTClient() {
  while (!mqttClient.connected()) {
    String client_id = "rf-warema-bridge-";
    client_id += String(WiFi.macAddress());

    Serial.printf("Trying to connect to MQTT Broker as %s\n", client_id.c_str());

    // Attempt connection
    if (mqttClient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT broker connected.");

      // Once connected, subscribe and optionally publish a hello message
      mqttClient.publish(topic, "Hello from rf-warema-bridge!");
      mqttClient.subscribe(topic);

      // Publish info about the client
      sendMQTTClientInfos();

    } else {
      Serial.print("Failed to connect. State = ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// =========================
// MQTT Callback
// =========================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  // Safely copy payload into a local buffer
  char command[256];
  if (length > 255) length = 255;  // Truncate if too long
  memcpy(command, payload, length);
  command[length] = '\0';

  Serial.print("Payload: ");
  Serial.println(command);

  // Forward the command to our Warema transmitter
  // Adjust timings if needed
  mySwitch.sendMC(command, 1780, 5000, 3, 10000);

  Serial.println("-----------------------");
}

// =========================
// Misc helper functions
// =========================

// Publishes connection info to 'topic_state_connection'
void sendMQTTClientInfos() {
  StaticJsonDocument<256> doc;

  doc["IP"]            = WiFi.localIP().toString();
  doc["MAC"]           = WiFi.macAddress();
  doc["RSSI"]          = WiFi.RSSI();
  doc["HostName"]      = WiFi.getHostname();
  doc["ConnectedSSID"] = WiFi.SSID();

  char jsonOut[256];
  size_t n = serializeJson(doc, jsonOut);

  bool result = mqttClient.publish(topic_state_connection, jsonOut);
  Serial.printf("Publishing on topic '%s': %s (result: %s)\n", 
                topic_state_connection, jsonOut, result ? "OK" : "FAILED");
}

// Simple LED write
void writeConnected(bool state) {
  // Red LED ON if connected, OFF otherwise
  digitalWrite(LED_WIFI_RED, state ? LOW : HIGH);
  // Yellow LED OFF if connected, ON otherwise
  digitalWrite(LED_WIFI_YELLOW, state ? HIGH : LOW);
}