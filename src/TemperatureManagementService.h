#include <Arduino.h>

class TemperatureManagementService{
  private:
    static const int TEMP_STARTING_ADDRESS = 0;
    static const int MEMORY_MAX_BYTES = 96;

  public:
    TemperatureManagementService();
    void saveCurrentTemperature(byte temp, int hour, int minute);
    std::vector<byte> getTemperature();
};
