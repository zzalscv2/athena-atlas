/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1DECODER_EFEXEMROITHRESHOLDSTOOL_H
#define L1DECODER_EFEXEMROITHRESHOLDSTOOL_H

#include "L1Decoder/IRoIThresholdsTool.h"
#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexEMRoIContainer.h"

namespace eFexEMRoIThresholdsToolParams {
  extern const char ContainerName[];
  extern const char ThresholdType[];
  using BaseClass = RoIThresholdsTool<xAOD::eFexEMRoI, xAOD::eFexEMRoIContainer, ContainerName, ThresholdType>;
}

class eFexEMRoIThresholdsTool : public eFexEMRoIThresholdsToolParams::BaseClass {
public:
  eFexEMRoIThresholdsTool(const std::string& type, const std::string& name, const IInterface* parent)
  : eFexEMRoIThresholdsToolParams::BaseClass(type, name, parent) {}

  virtual uint64_t getPattern(const xAOD::eFexEMRoI& roi,
                              const ThrVec& menuThresholds,
                              const TrigConf::L1ThrExtraInfoBase& menuExtraInfo) const override;
};

#endif // L1DECODER_EFEXEMROITHRESHOLDSTOOL_H
