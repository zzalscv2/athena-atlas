/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKBACKENDCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKBACKENDCONTAINER_V1_H

#include "AthContainers/DataVector.h"

#include "xAODTracking/versions/TrackBackend_v1.h"
namespace xAOD {
    typedef DataVector<xAOD::TrackBackend_v1> TrackBackendContainer_v1;
}


#endif