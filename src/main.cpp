#include <Arduino.h>
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include "RulesManagementService.h"

WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(14);
DallasTemperature sensors(&oneWire);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 3600);
RulesManagementService ruleService;

const char* ssid     = "net_4016";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "LCZTAUGQVCLCZTAUGQVC";     // The password of the Wi-Fi network
const char* mqttServer = "192.168.1.7";
const char* topicName = "sensors/heatingControl/1";
const char* systemStatusTopic = "sensors/heatingControl/1/status";

std::vector<Rule> rules;
long lastActiveRuleUpdate = 0;

bool isRelayOn = false;

// Pin config
int LED = 13;
int RELAY = 12;
int TEMP = 14;

// Blink duration
int BLINK_DURATION = 2000;
unsigned long RULE_WAIT_TIME = 30; // number of seconds to wait before inspecting rules

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Saving new rules to memory");
  ruleService.saveRules((char*) payload);

  Serial.println("Loading rules from memory");
  rules = ruleService.getSavedRules();

  lastActiveRuleUpdate = 0;
}

void setupWifi(){
  WiFi.begin(ssid, password);             // Connect to the network

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
}

void setupMqtt(){
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  client.subscribe(topicName);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      // Once connected, publish an announcement...
      client.publish(topicName, "esp8266 connected");
      // ... and resubscribe
      client.subscribe(topicName);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendSystemStatus(float temperature){
    const size_t capacity = JSON_OBJECT_SIZE(1) + 20;
    char* systemStatusJson = new char[capacity];
    DynamicJsonDocument doc(capacity);
    doc["temperature"] = temperature;
    serializeJson(doc, systemStatusJson, capacity);
    client.publish(systemStatusTopic, systemStatusJson);
}

void inspectRules(){
  if(timeClient.getEpochTime() < (lastActiveRuleUpdate + RULE_WAIT_TIME)){
    return;
  }

  Rule activeRule;
  bool isRuleMatched = false;

  Serial.println("Inspetcting rules..");

  sensors.requestTemperatures();
  float currentTemperature = sensors.getTempCByIndex(0);

  sendSystemStatus(currentTemperature);

  Serial.print("Current temp = ");
  Serial.println(currentTemperature);

  Serial.print("Day = ");
  Serial.println(timeClient.getDay());

  Serial.print("Hour = ");
  Serial.println(timeClient.getHours());

  Serial.print("Minute = ");
  Serial.println(timeClient.getMinutes());

  for(unsigned int i = 0; i < rules.size(); i++){
    if(rules[i].isActive(timeClient, currentTemperature)){
      activeRule = rules[i];
      isRuleMatched = true;
    }
  }

  if(isRuleMatched){
    if(currentTemperature < activeRule.getMaxTemperature() || activeRule.isDirect()){
      Serial.println("Matched Rule");
      activeRule.printInfoToSerial();
      digitalWrite(RELAY, HIGH);
    }else{
      digitalWrite(RELAY, LOW);
    }
  }else{
    digitalWrite(RELAY, LOW);
  }

  lastActiveRuleUpdate = timeClient.getEpochTime();
}

void setup() {
  Serial.begin(9600);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  setupWifi();
  setupMqtt();

  timeClient.begin();
  sensors.begin();

  Serial.println("Loading rules from memory");
  rules = ruleService.getSavedRules();

  for(unsigned int i = 0; i < rules.size(); i++){
    rules[i].printInfoToSerial();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  timeClient.update();
  inspectRules();
}
