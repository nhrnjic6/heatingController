#include "SystemRuleConfig.h"

SystemRuleConfig::SystemRuleConfig(){
  m_isSet = false;
}

SystemRuleConfig::SystemRuleConfig(std::vector<Rule*> setpoints, byte rulesMode){
  m_setpoints = setpoints;
  m_rulesMode = rulesMode;
  m_isSet = true;
}

void SystemRuleConfig::clear(){
  for (std::vector<Rule*>::iterator it = m_setpoints.begin(); it != m_setpoints.end(); it++) {
      delete *it;
  }

  m_setpoints.clear();
  m_isSet = false;
}

std::vector<Rule*> SystemRuleConfig::getSetpoints(){
  return m_setpoints;
}

byte SystemRuleConfig::getRulesMode(){
  return m_rulesMode;
}

bool SystemRuleConfig::isSet(){
  return m_isSet;
}
