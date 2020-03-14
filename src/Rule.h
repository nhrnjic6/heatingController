#include <NTPClient.h>

class Rule {
  private:
    int m_id;
    int m_day;
    int m_startHour;
    int m_startMinute;
    double m_maxTemperature;

  public:
    Rule();
    Rule(int id ,int day, int startHour, int startMinute, double maxTemp);
    int getId();
    int getDay();
    int getStartHour();
    int getStartMinute();
    double getMaxTemperature();
    bool isBefore(NTPClient ntpClient);
    bool isDirect();
    void printInfoToSerial();
};
