/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// test interface for a wrapper module that can open various kinds of neural
// network graph file

// include things to test
#include "ISF_FastCaloSimEvent/TFCSEnergyAndHitGANV2.h"
#include "ISF_FastCaloSimEvent/TFCSPredictExtrapWeights.h"

// include generic utilities
#include "ISF_FastCaloSimEvent/MLogging.h"
#include <fstream>
#include <iostream>
#include <vector>

using ISF_FCS::MLogging;

int main() {
  ISF_FCS::MLogging logger;
  ATH_MSG_NOCLASS(logger, "Running TFCSPredictExtrapWeights");
  const std::string this_file = __FILE__;
  const std::string parent_dir = this_file.substr(0, this_file.find("/test/"));
  const std::string norm_path = parent_dir + "/share/NormPredExtrapSample/";
  std::string net_path = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/"
                         "FastCaloSim/LWTNNPredExtrapSample/";
  TFCSPredictExtrapWeights::test_path(net_path, norm_path);
  // This causes timeouts in CI
  //net_path = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastCaloSim/"
  //           "ONNXPredExtrapSample/";
  //TFCSPredictExtrapWeights::test_path(net_path, norm_path);
  ATH_MSG_NOCLASS(logger, "Program ends");
  return 0;
}
