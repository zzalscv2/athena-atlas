/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSINTEROP_LOGGER_UTILS_H
#define ACTSINTEROP_LOGGER_UTILS_H

#include "Acts/Utilities/Logger.hpp"
#include "GaudiKernel/MsgStream.h"

namespace ActsTrk {

  Acts::Logging::Level actsLevelVector(MSG::Level lvl) {
    // MSG::NIL and MSG::ALWAYS are not available in Acts. Need to protect against these
    // For MSG::NIL we can return a Acts::Logging::Level::FATAL;
    // For MSG::ALWAYS we can return a Acts::Logging::Level::VERBOSE

    // Gaudi definitions are +1 w.r.t. Acts definitions  
    static const std::array<Acts::Logging::Level, 8> actsLevelVector{
      Acts::Logging::Level::FATAL, // MSG::NIL
      Acts::Logging::Level::VERBOSE,
      Acts::Logging::Level::DEBUG,
      Acts::Logging::Level::INFO,
      Acts::Logging::Level::WARNING,
      Acts::Logging::Level::ERROR,
      Acts::Logging::Level::FATAL,
      Acts::Logging::Level::VERBOSE // MSG::ALWAYS
	};

    return actsLevelVector[static_cast<int>(lvl)];
  }
  
  MSG::Level athLevelVector(Acts::Logging::Level lvl) {
    // All Acts log levels are available in Gaudi, no need for protections
    static const std::array<MSG::Level, 6> athLevelVector{
      MSG::VERBOSE,
      MSG::DEBUG,
      MSG::INFO,
      MSG::WARNING,
      MSG::ERROR,
      MSG::FATAL
	};   
    return athLevelVector[static_cast<int>(lvl)];
  }

}

#endif
