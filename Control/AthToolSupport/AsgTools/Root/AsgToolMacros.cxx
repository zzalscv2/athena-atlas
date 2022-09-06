//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "AsgTools/AsgToolMacros.h"

// System include(s).
#include <cstdio>
#include <string>

namespace asg {

std::string ptrToString (const void* p)
{
  char buf[80];
  snprintf (buf, 80, "%p", p);
  return std::string(buf);
}

} // namespace asg
