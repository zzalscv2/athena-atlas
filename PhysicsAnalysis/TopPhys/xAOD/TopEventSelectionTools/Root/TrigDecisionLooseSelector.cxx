/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#include "TopEventSelectionTools/TrigDecisionLooseSelector.h"
#include "TopEvent/Event.h"
#include "TopConfiguration/TopConfig.h"

#include <sstream>
#include <iostream>
#include <RootCoreUtils/StringUtil.h>

namespace top {
  TrigDecisionLooseSelector::TrigDecisionLooseSelector(const std::string& selectorName,
                                                       std::shared_ptr<top::TopConfig> config) {
    m_triggers = config->allTriggers_Loose(selectorName);
  }

  bool TrigDecisionLooseSelector::apply(const top::Event& event) const {
    // this selector does nothing for non-loose events
    bool loose = event.m_isLoose;

    if (!loose) return true;

    bool orOfAllTriggers(false);
    for (const auto& trigger : m_triggers) {
      bool passThisTrigger(false);
      auto trigger_name = RCU::substitute(RCU::substitute(trigger.first, ".", "p"), "-", "_");
      if (event.m_info->isAvailable<char>("TRIGDEC_" + trigger_name)) {
        if (event.m_info->auxdataConst<char>("TRIGDEC_" + trigger_name) == 1) {
          passThisTrigger = true;
        }
      }

      orOfAllTriggers |= passThisTrigger;
    }

    return orOfAllTriggers;
  }

  std::string TrigDecisionLooseSelector::name() const {
    std::string name = "TRIGDEC_LOOSE ";
    for (auto trigger : m_triggers)
      name += " " + trigger.first;

    return name;
  }
}
