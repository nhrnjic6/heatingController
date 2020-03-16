#include <Arduino.h>
#include <Rule.h>

class SystemRuleConfig{
  private:
    std::vector<Rule*> m_setpoints;
    byte m_rulesMode;
    bool m_isSet;

  public:
    SystemRuleConfig();
    SystemRuleConfig(std::vector<Rule*> setpoints, byte rulesMode);
    void clear();
    std::vector<Rule*> getSetpoints();
    byte getRulesMode();
    bool isSet();
};
