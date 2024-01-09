/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
  TFCSPredictExtrapWeights::unit_test();
  ATH_MSG_NOCLASS(logger, "Program ends");
  return 0;
}
