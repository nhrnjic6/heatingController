#include "Rule.h"

class RulesManagementService{
  public:
    RulesManagementService();
    void saveRules(const char* rulesAsText);
    std::vector<Rule*> getSavedRules();
    char* getSavedRulesRaw();

  private:
    static const int RULES_STARTING_ADDRESS = 10;
    static const int MAX_RULES_SIZE = 50;
    static const int MEMORY_MAX_BYTES = 3 * 1024;
};
