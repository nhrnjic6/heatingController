#include "TemperatureManagementService.h"
#include <EEPROM.h>

TemperatureManagementService::TemperatureManagementService(){
}

std::vector<byte>TemperatureManagementService::getTemperature(){
  std::vector<byte> temperature;

  for(int i = TEMP_STARTING_ADDRESS; i < TEMP_STARTING_ADDRESS + 96; i++){
    byte value = EEPROM.read(i);

    Serial.print("Reading temp from address = ");
    Serial.print(TEMP_STARTING_ADDRESS + i);
    Serial.print(" value = ");
    Serial.println(value);
    //temperature.push_back(value);
  }

  return temperature;
}

void TemperatureManagementService::saveCurrentTemperature(byte temp, int hour, int minute){
  int minutesToday = (hour * 60) + minute;
  int index = minutesToday / 15;

  Serial.print("Saving temp at address = ");
  Serial.print(TEMP_STARTING_ADDRESS + index);
  Serial.print(" value = ");
  Serial.println(temp);

  EEPROM.write(TEMP_STARTING_ADDRESS + index, temp);
}
