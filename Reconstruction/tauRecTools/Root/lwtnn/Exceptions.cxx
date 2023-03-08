/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/lwtnn/Exceptions.h"
#include <string>

namespace lwtDev {

  // ______________________________________________________________________
  // excpetions
  LightweightNNException::LightweightNNException(const std::string& problem):
    std::logic_error(problem)
  {}
  NNConfigurationException::NNConfigurationException(const std::string& problem):
    LightweightNNException(problem)
  {}
  NNEvaluationException::NNEvaluationException(const std::string& problem):
    LightweightNNException(problem)
  {}
  OutputRankException::OutputRankException(const std::string& problem):
    NNEvaluationException(problem)
  {}
}
