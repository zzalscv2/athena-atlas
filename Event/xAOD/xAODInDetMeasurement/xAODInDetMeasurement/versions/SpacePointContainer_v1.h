/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_VERSION_SPACEPOINTCONTAINER_V1_H
#define XAODINDETMEASUREMENT_VERSION_SPACEPOINTCONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODInDetMeasurement/versions/SpacePoint_v1.h"

namespace xAOD {
  /// The container is a simple typedef for now
  typedef DataVector< xAOD::SpacePoint_v1 > SpacePointContainer_v1;
}

#endif
