#include "Rule.h"

Rule::Rule(){
}

Rule::Rule(int day, int startHour, int startMinute, double maxTemp){
  m_day = day;
  m_startHour = startHour;
  m_startMinute = startMinute;
  m_maxTemperature = maxTemp;
}

int Rule::getDay(){
  return m_day;
}

int Rule::getStartHour(){
  return m_startHour;
}

int Rule::getStartMinute(){
  return m_startMinute;
}

double Rule::getMaxTemperature(){
  return m_maxTemperature;
}

bool Rule::isActive(NTPClient ntpClient, float currentTemperature){
  bool isDayActive = false;

  if(m_day == 0){
    // day = 0 is a special setpoint which means that direct control was used
    // and heater should be turned ON without checking temperature status
    return true;
  }

  if(m_day == 8 && ntpClient.getDay() >= 1 && ntpClient.getDay() <= 5){
    // 8 represent work week group
    isDayActive = true;
  }

  if(m_day == 9 && ntpClient.getDay() >= 6 && ntpClient.getDay() <= 7){
    // 9 represent weekend group
    isDayActive = true;
  }

  if(m_day == ntpClient.getDay()){
    isDayActive = true;
  }

  return
    (isDayActive &&
     ntpClient.getHours() >= m_startHour &&
     ntpClient.getMinutes() >= m_startMinute);
}

bool Rule::isDirect(){
  return m_day == 0;
}

void Rule::printInfoToSerial(){
  Serial.println("--------------------------------------");
  Serial.print("day = ");
  Serial.println(m_day);
  Serial.print("start hour = ");
  Serial.println(m_startHour);
  Serial.print("start minute = ");
  Serial.println(m_startMinute);
  Serial.print("max temperature = ");
  Serial.println(m_maxTemperature);
  Serial.println("---------------------------------------");
}
