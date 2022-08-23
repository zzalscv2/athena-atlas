/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class TPhotonEfficiencyCorrectionTool
   @brief Calculate the photon scale factors and assosiated errors in pure ROOT

   @author Michael Pitt
   @date   April 2014
*/


// This class header
#include "PhotonEfficiencyCorrection/TPhotonEfficiencyCorrectionTool.h"


Root::TPhotonEfficiencyCorrectionTool::TPhotonEfficiencyCorrectionTool(const char* name):
    Root::TElectronEfficiencyCorrectionTool(name){
    }

Root::TPhotonEfficiencyCorrectionTool::~TPhotonEfficiencyCorrectionTool()= default;

int Root::TPhotonEfficiencyCorrectionTool::initialize(){
    //Apparently the TResult needs a "specific convention" for the 1st  2
   return Root::TElectronEfficiencyCorrectionTool::initialize();
}

using Result = Root::TPhotonEfficiencyCorrectionTool::Result;
const Result Root::TPhotonEfficiencyCorrectionTool::calculate( const PATCore::ParticleDataType::DataType dataType,
                                  const unsigned int runnumber,
                                  const double cluster_eta,
                                  const double et /* in MeV */
                                  ) const {

    size_t CorrIndex{0},MCToysIndex{0};
    std::vector<double> result;
    const int status = Root::TElectronEfficiencyCorrectionTool::calculate(dataType,
									  runnumber,
									  cluster_eta,
									  et, /* in MeV */
									  result,
									  CorrIndex,
									  MCToysIndex
									  );
    
    Result output;

    // if status 0 something went wrong
    if (!status) {
      output.scaleFactor=-999;
      output.totalUncertainty=1;
      ATH_MSG_DEBUG("Something went wrong ... in the future we should report an CP::CorrectionCode::OutOfValidityRange");
      return output;
    }

    // For Photons we only support one correlation model
    output.scaleFactor= result[static_cast<size_t>(Root::TElectronEfficiencyCorrectionTool::Position::SF)];
    output.totalUncertainty=result[static_cast<size_t>(Root::TElectronEfficiencyCorrectionTool::Position::Total)];
    return output;
}
