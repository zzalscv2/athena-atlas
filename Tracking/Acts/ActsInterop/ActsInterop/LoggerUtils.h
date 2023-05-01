/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSINTEROP_LOGGER_UTILS_H
#define ACTSINTEROP_LOGGER_UTILS_H

#include "Acts/Utilities/Logger.hpp"
#include "GaudiKernel/MsgStream.h"

namespace ActsTrk {
  Acts::Logging::Level actsLevelVector(MSG::Level lvl);
  MSG::Level athLevelVector(Acts::Logging::Level lvl);
}

#endif
