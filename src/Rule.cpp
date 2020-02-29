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

bool Rule::isBefore(NTPClient ntpClient){
  bool isActiveDay = false;

  // IS DIRECT OR CURRENT DAY
  if(m_day == 0 || m_day == ntpClient.getDay()){
    isActiveDay = true;
  }

  // IS WORK WEEK GROUP
  if(m_day == 8 && (ntpClient.getDay() == 6 || ntpClient.getDay() == 7)){
    isActiveDay = true;
  }

  // IS WEEKEND GROUP
  if(m_day == 9 && (ntpClient.getDay() >= 1 && ntpClient.getDay() <= 5)){
    isActiveDay = true;
  }

  return isActiveDay
    && m_startHour <= ntpClient.getHours()
    && m_startMinute < ntpClient.getMinutes();
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
