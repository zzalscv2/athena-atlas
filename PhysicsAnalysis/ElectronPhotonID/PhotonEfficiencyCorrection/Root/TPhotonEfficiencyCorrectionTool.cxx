/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class TPhotonEfficiencyCorrectionTool
   @brief Calculate the photon scale factors and assosiated errors in pure ROOT

   @author Michael Pitt
   @date   April 2014
*/


// This class header
#include "PhotonEfficiencyCorrection/TPhotonEfficiencyCorrectionTool.h"

Root::TPhotonEfficiencyCorrectionTool::TPhotonEfficiencyCorrectionTool(
    const char* name)
    : Root::TElectronEfficiencyCorrectionTool(name) {}

Root::TPhotonEfficiencyCorrectionTool::~TPhotonEfficiencyCorrectionTool() =
    default;

int Root::TPhotonEfficiencyCorrectionTool::initialize() {
  return Root::TElectronEfficiencyCorrectionTool::initialize();
}

int Root::TPhotonEfficiencyCorrectionTool::calculate(
    const PATCore::ParticleDataType::DataType dataType,
    const unsigned int runnumber, const double cluster_eta,
    const double et, /* in MeV */
    Root::TPhotonEfficiencyCorrectionTool::Result& sf_and_err) const {

  Root::TElectronEfficiencyCorrectionTool::Result result;
  const int status = Root::TElectronEfficiencyCorrectionTool::calculate(
      dataType, runnumber, cluster_eta, et, /* in MeV */
      result,true);

  // if status 0 something went wrong
  if (!status) {
    sf_and_err.scaleFactor = -999;
    sf_and_err.totalUncertainty = 1;
    ATH_MSG_DEBUG(
        "Something went wrong ... for debugging, "
        << "look for a message from TElectronEfficiencyCorrectionTool");
    return 0;
  }
  // For Photons we only support one correlation model
  sf_and_err.scaleFactor = result.SF;
  sf_and_err.totalUncertainty = result.Total;
  return status;
}
