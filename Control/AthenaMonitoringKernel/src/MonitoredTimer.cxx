/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaMonitoringKernel/MonitoredTimer.h"
#include <stdexcept>

namespace Monitored {
  void checkNamingConvention( const std::string& name ) {
    // Enforce some naming convention for timers
    if (name.size() < 5 || name.substr(0,5) != std::string("TIME_")) {
      throw std::runtime_error("Name of Timer \"" + name + "\" needs to start with \"TIME_\"");
    }

  }

} // namespace Monitored
