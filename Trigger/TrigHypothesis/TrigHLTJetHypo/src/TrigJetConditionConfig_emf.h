/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_EMF_H
#define TRIGJETCONDITIONCONFIG_EMF_H

#include "ITrigJetConditionConfig.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "./ConditionsDefs.h"

class TrigJetConditionConfig_emf:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_emf(const std::string& type,
                          const std::string& name,
                          const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:
  
  Gaudi::Property<std::string>
    m_min{this, "min", {}, "single jet min EMF"};
  
  Gaudi::Property<std::string>
    m_max{this, "max", {}, "single jet max EMF"};
  
  StatusCode checkVals()  const;
 
};
#endif
