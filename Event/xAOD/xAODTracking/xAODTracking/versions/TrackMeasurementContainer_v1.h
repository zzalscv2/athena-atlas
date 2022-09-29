/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKMEASUREMENTCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKMEASUREMENTCONTAINER_V1_H

#include "AthContainers/DataVector.h"

#include "xAODTracking/versions/TrackMeasurement_v1.h"
namespace xAOD {
    typedef DataVector<xAOD::TrackMeasurement_v1> TrackMeasurementContainer_v1;
}


#endif