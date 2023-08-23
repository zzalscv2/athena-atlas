
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_TIMESIG_H
#define TRIGJETCONDITIONCONFIG_TIMESIG_H


#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigJetConditionConfig_timesig:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_timesig(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:

  Gaudi::Property<std::string>
    m_minTimeSignificance{this, "min", {}, "min time significance value"};
  
  Gaudi::Property<std::string>
    m_maxTime{this, "max", {}, "max time value"};

  StatusCode checkVals()  const;
};
#endif
