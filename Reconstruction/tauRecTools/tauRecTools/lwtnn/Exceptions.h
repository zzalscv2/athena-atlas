/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EXCEPTIONS_HH_TAURECTOOLS
#define EXCEPTIONS_HH_TAURECTOOLS

#include <stdexcept>

namespace lwtDev {
  // ______________________________________________________________________
  // exceptions

  // base exception
  class LightweightNNException: public std::logic_error {
  public:
    LightweightNNException(const std::string& problem);
  };

  // thrown by the constructor if something goes wrong
  class NNConfigurationException: public LightweightNNException {
  public:
    NNConfigurationException(const std::string& problem);
  };

  // thrown by `compute`
  class NNEvaluationException: public LightweightNNException {
  public:
    NNEvaluationException(const std::string& problem);
  };
  class OutputRankException: public NNEvaluationException {
  public:
    OutputRankException(const std::string& problem);
  };
}

#endif // EXCEPTIONS_HH_TAURECTOOLS



