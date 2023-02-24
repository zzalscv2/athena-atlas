/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_SPACEPOINT_H
#define XAODINDETMEASUREMENT_SPACEPOINT_H

#include "xAODInDetMeasurement/versions/SpacePoint_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
  typedef SpacePoint_v1 SpacePoint;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::SpacePoint, 229413835, 1 )

#endif
