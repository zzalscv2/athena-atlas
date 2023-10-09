/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaMonitoringKernel/MonitoredTimer.h"
#include <stdexcept>
#include <string_view>

namespace Monitored {
  void checkNamingConvention( std::string_view name ) {
    // Enforce some naming convention for timers
    if ( name.substr(0,5) != "TIME_") {
      std::string error("Name of Timer \"");
      error+= name;
      error+=  "\" needs to start with \"TIME_\"";
      throw std::runtime_error(error);
    }
  }

} // namespace Monitored
