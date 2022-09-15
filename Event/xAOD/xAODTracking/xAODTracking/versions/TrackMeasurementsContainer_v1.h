/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKMEASUREMENTSCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKMEASUREMENTSCONTAINER_V1_H

#include "AthContainers/DataVector.h"

#include "xAODTracking/versions/TrackMeasurements_v1.h"
namespace xAOD {
    typedef DataVector<xAOD::TrackMeasurements_v1> TrackMeasurementsContainer_v1;
}


#endif