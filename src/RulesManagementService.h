#include "SystemRuleConfig.h"

class RulesManagementService{
  public:
    RulesManagementService();
    void saveRules(const char* rulesAsText);
    SystemRuleConfig getSystemConfig();

  private:
    static const int RULES_STARTING_ADDRESS = 106;
    static const int MAX_RULES_SIZE = 50;
    static const int MEMORY_MAX_BYTES = 5 * 1024;
};
