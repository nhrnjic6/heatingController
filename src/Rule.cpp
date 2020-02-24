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
  return
    (m_day == ntpClient.getDay() &&
     ntpClient.getHours() >= m_startHour &&
     ntpClient.getMinutes() >= m_startMinute);
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
