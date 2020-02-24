#include "RulesManagementService.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Arduino.h>

RulesManagementService::RulesManagementService(){
  EEPROM.begin(MEMORY_MAX_BYTES);
}

void RulesManagementService::saveRules(const char *rulesAsText){
  unsigned int i;

  // clear memory
  for(i = 0; i < MEMORY_MAX_BYTES; i++){
    EEPROM.write(i, 0);
  }

  // save new rules to memory
  for(i = 0; i < strlen(rulesAsText); i++){
    EEPROM.write(i + RULES_STARTING_ADDRESS, rulesAsText[i]);
  }

  // mark end of rules with sign "!"
  EEPROM.write(i + RULES_STARTING_ADDRESS, '!');
  EEPROM.commit();
}

std::vector<Rule> RulesManagementService::getSavedRules(){
  std::vector<Rule> rules;
  char* jsonFromMemory = new char[MEMORY_MAX_BYTES];

  unsigned int index = 0;

  while(index < MEMORY_MAX_BYTES){
    char currentChar = EEPROM.read(index + RULES_STARTING_ADDRESS);
    if(currentChar == '!') break; // end of rules

    jsonFromMemory[index++] = currentChar;
  }

  DynamicJsonDocument doc(MEMORY_MAX_BYTES);
  DeserializationError error = deserializeJson(doc, jsonFromMemory);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
  }else{
    Serial.println(F("deserializeJson() success"));
  }

  int rulesSize = doc["rulesSize"];
  JsonArray rulesJson = doc["rules"];

  for(int i = 0; i < rulesSize; i++){
    JsonObject ruleJson = rulesJson[i];
    Rule rule(ruleJson["day"], ruleJson["startHour"], ruleJson["startMinute"], ruleJson["temperature"]);
    rules.push_back(rule);
  }

  return rules;
}
