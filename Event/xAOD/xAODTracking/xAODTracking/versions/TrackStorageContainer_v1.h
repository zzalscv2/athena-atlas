/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKBACKENDCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKBACKENDCONTAINER_V1_H

#include "AthContainers/DataVector.h"

#include "xAODTracking/versions/TrackStorage_v1.h"
namespace xAOD {
    typedef DataVector<xAOD::TrackStorage_v1> TrackStorageContainer_v1;
}


#endif