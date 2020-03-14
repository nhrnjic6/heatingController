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
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 36000);
RulesManagementService ruleService;

const char* ssid     = "net_4016";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "LCZTAUGQVCLCZTAUGQVC";     // The password of the Wi-Fi network
const char* mqttServer = "192.168.1.5";
const char* topicName = "sensors/heatingControl/1";
const char* systemStatusTopic = "sensors/heatingControl/1/status";

std::vector<Rule*> rules;
long lastActiveRuleUpdate = 0;
char* rulesRaw;

bool isRelayOn = false;

// Pin config
int LED = 13;
int RELAY = 12;
int TEMP = 14;

// Blink duration
int BLINK_DURATION = 2000;
unsigned long RULE_WAIT_TIME = 30; // number of seconds to wait before inspecting rules

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Saving new rules to memory with length = ");
  Serial.println(length);

  ruleService.saveRules((char*) payload);

  for (std::vector<Rule*>::iterator it = rules.begin(); it != rules.end(); it++) {
      delete *it;
  }

  rules.clear();

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

void sendSystemStatus(float temperature, int ruleId){
    const size_t capacity = JSON_OBJECT_SIZE(4) + (5 * 1024);
    char* systemStatusJson = new char[capacity];
    DynamicJsonDocument doc(capacity);

    doc["id"] = ruleId;
    doc["temperature"] = temperature;
    doc["updatedAt"] = timeClient.getEpochTime();

    if(lastActiveRuleUpdate == 0 && rules.size() > 0){
        // this is first update since new rules are set
        // send those rules back to client for verification
        JsonArray rulesJsonArray = doc.createNestedArray("rules");

        for (std::vector<Rule*>::iterator it = rules.begin(); it != rules.end(); it++) {
          JsonObject rule = rulesJsonArray.createNestedObject();
          rule["id"] = (*it)->getId();
          rule["day"] = (*it)->getDay();
          rule["hour"] = (*it)->getStartHour();
          rule["minute"] = (*it)->getStartMinute();
          rule["temperature"] = (*it)->getMaxTemperature();
        }
    }

    serializeJson(doc, systemStatusJson, capacity);
    client.publish(systemStatusTopic, systemStatusJson);

    doc.clear();
    delete[] systemStatusJson;
}

void inspectRules(){
  if(timeClient.getEpochTime() < (lastActiveRuleUpdate + RULE_WAIT_TIME)){
    return;
  }

  Rule activeRule;
  int activeRuleId = -1; // not mached
  bool isRuleMatched = false;

  Serial.print("Free Heap size = ");
  Serial.println(system_get_free_heap_size());
  Serial.println("---------------------------");

  Serial.println("Inspetcting rules..");

  sensors.requestTemperatures();
  float currentTemperature = sensors.getTempCByIndex(0);

  Serial.print("Current temp = ");
  Serial.println(currentTemperature);

  Serial.print("Day = ");
  Serial.println(timeClient.getDay());

  Serial.print("Hour = ");
  Serial.println(timeClient.getHours());

  Serial.print("Minute = ");
  Serial.println(timeClient.getMinutes());

  for(int i = (rules.size() - 1); i >= 0; i--){
    if(rules[i]->isBefore(timeClient) || rules[i]->isDirect()){
      activeRule = *rules[i];
      isRuleMatched = true;
      break;
    }
  }

  if(isRuleMatched){
    if(currentTemperature < activeRule.getMaxTemperature() || activeRule.isDirect()){
      Serial.println("Matched Rule");
      activeRule.printInfoToSerial();
      digitalWrite(RELAY, HIGH);
      activeRuleId = activeRule.getId();
    }else{
      Serial.println("Relay LOW - INNER ELSE");
      digitalWrite(RELAY, LOW);
    }
  }else{
    Serial.println("Relay LOW - OUTER ELSE");
    digitalWrite(RELAY, LOW);
  }

  sendSystemStatus(currentTemperature, activeRuleId);
  lastActiveRuleUpdate = timeClient.getEpochTime();
}

void setup() {
  Serial.begin(9600);
  delay(10);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  setupWifi();
  setupMqtt();

  rules = ruleService.getSavedRules();

  timeClient.begin();
  sensors.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  timeClient.update();
  inspectRules();
}
