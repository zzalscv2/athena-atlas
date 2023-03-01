/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETCONDITIONCONFIG_BDIPS_H
#define TRIGJETCONDITIONCONFIG_BDIPS_H


#include "ITrigJetConditionConfig.h"
#include "./ConditionsDefs.h"
#include "AthenaBaseComps/AthAlgTool.h"

class TrigJetConditionConfig_bdips:
public extends<AthAlgTool, ITrigJetConditionConfig> {

 public:
  
  TrigJetConditionConfig_bdips(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual Condition getCondition() const override;

 private:

  Gaudi::Property<std::string>
    m_min{this, "min", {}, "min dips cut value"};

  Gaudi::Property<std::string>
    m_max{this, "max", {}, "max dips cut value"};

  Gaudi::Property<std::string> m_cfrac{
    this, "cfrac", {}, "charm fraction"};
  Gaudi::Property<std::string> m_name_pb{
    this, "namePb", {}, "pb accessor"};
  Gaudi::Property<std::string> m_name_pc{
    this, "namePc", {}, "pc accessor"};
  Gaudi::Property<std::string> m_name_pu{
    this, "namePu", {}, "pu accessor"};
  Gaudi::Property<std::string> m_name_valid{
    this, "nameValid", {}, "validity check"};

  StatusCode checkVals()  const;
};
#endif
