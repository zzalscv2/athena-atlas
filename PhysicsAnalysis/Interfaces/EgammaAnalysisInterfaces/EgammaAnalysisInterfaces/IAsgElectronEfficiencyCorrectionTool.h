/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// IAsgElectronEfficiencyCorrection.h to be used in the tool.
#ifndef EGAMMAANALYSISINTERFACES__IASGELECTRONEFFICIENCYCORRECTION__
#define EGAMMAANALYSISINTERFACES__IASGELECTRONEFFICIENCYCORRECTION__

#include "AsgTools/IAsgTool.h"
#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "xAODEgamma/ElectronFwd.h"
namespace xAOD {
class IParticle;
}

class IAsgElectronEfficiencyCorrectionTool : virtual public CP::ISystematicsTool
{
  ASG_TOOL_INTERFACE(IAsgElectronEfficiencyCorrectionTool)

public:
  virtual CP::CorrectionCode getEfficiencyScaleFactor(
    const xAOD::Electron& inputObject,
    double& efficiencyScaleFactor) const = 0;

  virtual CP::CorrectionCode applyEfficiencyScaleFactor(
    const xAOD::Electron& inputObject) const = 0;

  virtual int systUncorrVariationIndex(
    const xAOD::Electron& inputObject) const = 0;

  virtual int getNumberOfToys() const = 0;

  virtual const CP::SystematicSet& appliedSystematics() const = 0;

  /// print available/implemented correlation models
  virtual void printCorrelationModels() const = 0;

  virtual ~IAsgElectronEfficiencyCorrectionTool() = default;
};

#endif
