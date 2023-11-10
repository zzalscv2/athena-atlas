/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_DIPZMLPL_H
#define TRIGJETCONDITIONCONFIG_DIPZMLPL_H

/*
Condiguration AlgTool for ht conditions to be run in FastReduction
PS 
*/

#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

// #include "TrigHLTJetHypo/TrigHLTJetHypoUtils/ConditionsDefs.h"

class TrigJetConditionConfig_dipzmlpl:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_dipzmlpl(const std::string& type,
			      const std::string& name,
			      const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:


  Gaudi::Property<std::string>
    m_min{this, "min", {}, "min value for Dipz MLPL"};
  Gaudi::Property<std::string>
    m_max{this, "max", {}, "max value for Dipz MLPL"};  
  Gaudi::Property<std::string>
    m_capacity{this, "capacity", {}, "number of jets considered"};  
  Gaudi::Property<std::string> m_decName_z{
    this, "decName_z", {}, "dipz z accessor"};
  Gaudi::Property<std::string> m_decName_negLogSigma2{
    this, "decName_sigma", {}, "dipz sigma accessor"};

  StatusCode checkVals()  const;
};
#endif
