/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_SPACEPOINT_CONTAINER_H
#define XAODINDETMEASUREMENT_SPACEPOINT_CONTAINER_H

#include "xAODInDetMeasurement/SpacePoint.h"
#include "xAODInDetMeasurement/versions/SpacePointContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the space point container
    typedef SpacePointContainer_v1 SpacePointContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::SpacePointContainer, 1318142340, 1 )

#endif

