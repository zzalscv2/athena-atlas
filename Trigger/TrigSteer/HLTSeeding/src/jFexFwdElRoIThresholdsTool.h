/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HLTSEEDING_JFEXFWDELROITHRESHOLDSTOOL_H
#define HLTSEEDING_JFEXFWDELROITHRESHOLDSTOOL_H

#include "HLTSeedingRoIToolDefs.h"
#include "HLTSeeding/IRoIThresholdsTool.h"
#include "xAODTrigger/jFexFwdElRoI.h"

class jFexFwdElRoIThresholdsTool : public HLTSeedingRoIToolDefs::jFexFwdEl::ThresholdBaseClass {
public:
  jFexFwdElRoIThresholdsTool(const std::string& type, const std::string& name, const IInterface* parent)
  : HLTSeedingRoIToolDefs::jFexFwdEl::ThresholdBaseClass(type, name, parent) {}

  virtual uint64_t getPattern(const xAOD::jFexFwdElRoI& roi,
                              const ThrVec& menuThresholds,
                              const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const override;
};

#endif // HLTSEEDING_JFEXFWDELROITHRESHOLDSTOOL_H
