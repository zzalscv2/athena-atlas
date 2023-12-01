/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
  Instantiator for PT Condition
 */
#include "TrigJetConditionConfig_emf.h"
#include "GaudiKernel/StatusCode.h"
#include "./EMFCondition.h"
#include "./ArgStrToDouble.h"


TrigJetConditionConfig_emf::TrigJetConditionConfig_emf(const std::string& type,
						     const std::string& name,
						     const IInterface* parent) :
  base_class(type, name, parent){
  
}


StatusCode TrigJetConditionConfig_emf::initialize() {
  return StatusCode::SUCCESS;
}


Condition TrigJetConditionConfig_emf::getCondition() const {
  auto a2d = ArgStrToDouble();
  return std::make_unique<EMFCondition>(a2d(m_min));
}
				     

StatusCode TrigJetConditionConfig_emf::checkVals() const {
  return StatusCode::SUCCESS;
}
