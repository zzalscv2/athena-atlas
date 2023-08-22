/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
  Instantiator for SIGNIFICANCE TIMING Condition
 */
#include "TrigJetConditionConfig_timesig.h"
#include "GaudiKernel/StatusCode.h"
#include "./TimeSignificanceCondition.h"
#include "./ArgStrToDouble.h"

TrigJetConditionConfig_timesig::TrigJetConditionConfig_timesig(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent){
}


StatusCode TrigJetConditionConfig_timesig::initialize() {
  CHECK(checkVals());
  
  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_timesig::getCondition() const {
  auto a2d = ArgStrToDouble();
  return std::make_unique<TimeSignificanceCondition>(a2d(m_minTimeSignificance), a2d(m_maxTime));
}

 
StatusCode TrigJetConditionConfig_timesig::checkVals() const {
  return StatusCode::SUCCESS;
}
