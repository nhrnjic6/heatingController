#include <NTPClient.h>

class Rule {
  private:
    int m_day;
    int m_startHour;
    int m_startMinute;
    double m_maxTemperature;

  public:
    Rule();
    Rule(int day, int startHour, int startMinute, double maxTemp);
    int getDay();
    int getStartHour();
    int getStartMinute();
    double getMaxTemperature();
    bool isActive(NTPClient ntpClient, float currentTemperature);
    bool isDirect();
    void printInfoToSerial();
};
