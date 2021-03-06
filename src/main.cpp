#include <Arduino.h>
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
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
const char* mqttServer = "167.172.171.209";
const char* mqttUsername = "nhrnjic";
const char* mqttPassword = "RBKz2rmmnFh77ACR62pR";
const char* actionTopic = "sensors/heatingControl/action";
const char* statusTopic = "sensors/heatingControl/status";

long currentTimeOffset = 3600;

SystemRuleConfig systemConfig;
long lastActiveRuleUpdate = 0;
String requestId;

// Pin config

int LED = 13;
int RELAY = 12;
int TEMP = 14;

// Blink duration
int BLINK_DURATION = 2000;
unsigned long RULE_WAIT_TIME = 30; // number of seconds to wait before inspecting rules

void setupWifi(){
  WiFi.begin(ssid, password);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
      // ... and resubscribe
      client.subscribe(actionTopic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

std::vector<byte> getTemperature(){
  std::vector<byte> temperature;

  for(int i = 10 ; i < 106; i++){
    temperature.push_back(EEPROM.read(i));
  }

  return temperature;
}

void saveCurrentTemperature(byte temperature){
  int minutesToday = (timeClient.getHours() * 60) + timeClient.getMinutes();
  int offset = minutesToday / 15;
  Serial.print("Saving at offset = ");
  Serial.print(offset);
  Serial.print(" Value = ");
  Serial.print(temperature);

  EEPROM.write(10 + offset, temperature);
}

void sendSystemStatus(float temperature, int ruleId){
    const size_t capacity = JSON_OBJECT_SIZE(4) + (5 * 1024);
    char* systemStatusJson = new char[capacity];
    DynamicJsonDocument doc(capacity);

    doc["requestId"] = requestId;
    doc["id"] = ruleId;
    doc["temperature"] = temperature;
    doc["updatedAt"] = timeClient.getEpochTime();

    if(lastActiveRuleUpdate == 0 && systemConfig.isSet()){

        doc["rulesMode"] = systemConfig.getRulesMode();
        // this is first update since new rules are set
        // send those rules back to client for verification
        JsonArray rulesJsonArray = doc.createNestedArray("rules");
        JsonArray tempArray = doc.createNestedArray("tempList");

        for (unsigned int i = 0; i < systemConfig.getSetpoints().size(); i++) {
          JsonObject rule = rulesJsonArray.createNestedObject();
          rule["id"] = systemConfig.getSetpoints()[i]->getId();
          rule["day"] = systemConfig.getSetpoints()[i]->getDay();
          rule["hour"] = systemConfig.getSetpoints()[i]->getStartHour();
          rule["minute"] = systemConfig.getSetpoints()[i]->getStartMinute();
          rule["temperature"] = systemConfig.getSetpoints()[i]->getMaxTemperature();
        }

        std::vector<byte> temperature = getTemperature();

        for(unsigned int i = 0; i < temperature.size(); i++){
            tempArray.add(temperature[i]);
        }
    }

    serializeJson(doc, systemStatusJson, capacity);
    client.publish(statusTopic, systemStatusJson);

    doc.clear();
    delete[] systemStatusJson;
}

void adjustTimeShifting(){
  TimeElements now;
  breakTime(timeClient.getEpochTime(), now);
  time_t nowMilis = makeTime(now);

  TimeElements summerTime;
  summerTime.Year = now.Year;
  summerTime.Month = 3;
  summerTime.Day = 29;
  summerTime.Hour = 2;
  time_t summerTimeMilis = makeTime(summerTime);

  TimeElements winterTime;
  winterTime.Year = now.Year;
  winterTime.Month = 10;
  winterTime.Day = 25;
  winterTime.Hour = 3;
  time_t winterTimeMilis = makeTime(winterTime);

  if(currentTimeOffset != 3600 && (nowMilis < summerTimeMilis)){
    timeClient.setTimeOffset(3600);
    currentTimeOffset = 3600;
    return;
  }

  if(currentTimeOffset != 7200 && (nowMilis >= summerTimeMilis && nowMilis < winterTimeMilis)){
    timeClient.setTimeOffset(7200);
    currentTimeOffset = 7200;
    return;
  }

  if(currentTimeOffset != 3600 && (nowMilis >= winterTimeMilis)){
    timeClient.setTimeOffset(3600);
    currentTimeOffset = 3600;
    return;
  }
}

void inspectRules(){
  if(systemConfig.isSet() && timeClient.getEpochTime() < (lastActiveRuleUpdate + RULE_WAIT_TIME)){
    return;
  }

  adjustTimeShifting();

  int activeRuleId = -1; // not mached

  sensors.requestTemperatures();
  float currentTemperature = sensors.getTempCByIndex(0);
  saveCurrentTemperature(currentTemperature);

  if(systemConfig.getRulesMode() == 0){
    digitalWrite(RELAY, LOW);
    sendSystemStatus(currentTemperature, activeRuleId);
    lastActiveRuleUpdate = timeClient.getEpochTime();
    return;
  }

  if(systemConfig.getRulesMode() == 1){
    digitalWrite(RELAY, HIGH);
    sendSystemStatus(currentTemperature, activeRuleId);
    lastActiveRuleUpdate = timeClient.getEpochTime();
    return;
  }

  Rule activeRule;
  bool isRuleMatched = false;

  Serial.print("Free Heap size = ");
  Serial.println(system_get_free_heap_size());

  Serial.println("Inspetcting rules..");

  Serial.print("Current temp = ");
  Serial.println(currentTemperature);

  Serial.print("Day = ");
  Serial.println(timeClient.getDay());

  Serial.print("Hour = ");
  Serial.println(timeClient.getHours());

  Serial.print("Minute = ");
  Serial.println(timeClient.getMinutes());

  for(int i = (systemConfig.getSetpoints().size() - 1); i >= 0; i--){
    if(systemConfig.getSetpoints()[i]->isBefore(timeClient)){
      activeRule = *systemConfig.getSetpoints()[i];
      isRuleMatched = true;
      break;
    }
  }

  if(isRuleMatched){
    if(currentTemperature < activeRule.getMaxTemperature()){
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

void callback(char* topic, byte* payload, unsigned int length) {

  char* payloadCopy = new char[strlen((char*) payload) + 1];
  strcpy(payloadCopy, (char*)payload);

  DynamicJsonDocument doc(5 * 1024);
  DeserializationError error = deserializeJson(doc, payloadCopy);

  if(!error){
    const char* setActionType = "set_setpoints";
    const char* getActionType = "get_setpoints";
    const char* actionTypeValue = doc["actionType"];
    const char* reqId = doc["requestId"];

    if(strcmp(actionTypeValue, setActionType) == 0){
      requestId = reqId; // extract to system object

      ruleService.saveRules((char*) payload);

      systemConfig.clear();
      systemConfig = ruleService.getSystemConfig();

      lastActiveRuleUpdate = 0; // extract to system object
    }

    if(strcmp(actionTypeValue, getActionType) == 0){
      requestId = reqId;
      lastActiveRuleUpdate = 0; // extract to system object
      inspectRules();
    }
  }

  doc.clear();
  delete[] payloadCopy;
}

void setupMqtt(){
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  client.subscribe(actionTopic);
}

void setup() {
  Serial.begin(9600);
  delay(10);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  setupWifi();
  setupMqtt();

  systemConfig.clear();
  systemConfig = ruleService.getSystemConfig();

  sensors.begin();
  timeClient.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  timeClient.update();
  inspectRules();
}
