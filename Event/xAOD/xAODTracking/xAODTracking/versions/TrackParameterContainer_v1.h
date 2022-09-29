/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKING_VERSIONS_TRACKPARAMETERCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKPARAMETERCONTAINER_V1_H

#include "AthContainers/DataVector.h"

#include "xAODTracking/versions/TrackParameter_v1.h"
namespace xAOD {
    typedef DataVector<xAOD::TrackParameter_v1> TrackParameterContainer_v1;
}


#endif