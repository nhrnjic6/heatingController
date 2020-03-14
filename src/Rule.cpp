#include "Rule.h"

Rule::Rule(){
}

Rule::Rule(int id ,int day, int startHour, int startMinute, double maxTemp){
  m_id = id;
  m_day = day;
  m_startHour = startHour;
  m_startMinute = startMinute;
  m_maxTemperature = maxTemp;
}

int Rule::getId(){
  return m_id;
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
  int currentDay = (ntpClient.getDay() + 6) % 7 + 1;

  // IS DIRECT OR CURRENT DAY
  if(m_day == 0 || m_day == currentDay){
    isActiveDay = true;
  }

  // IS WORK WEEK GROUP
  if(m_day == 8 && (currentDay >= 1 && currentDay <= 5)){
    isActiveDay = true;
  }

  // IS WEEKEND GROUP
  if(m_day == 9 && (currentDay == 6 || currentDay == 7)){
    isActiveDay = true;
  }

  int minutesOfRule = (m_startHour * 60) + m_startMinute;
  int minutesOfToday = (ntpClient.getHours() * 60) + ntpClient.getMinutes();

  return isActiveDay && minutesOfRule < minutesOfToday;
}

bool Rule::isDirect(){
  return m_day == 0;
}

void Rule::printInfoToSerial(){
  Serial.println("--------------------------------------");
  Serial.print("id = ");
  Serial.println(m_id);
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
