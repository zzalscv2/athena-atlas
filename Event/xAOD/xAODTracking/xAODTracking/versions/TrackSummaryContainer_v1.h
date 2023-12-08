/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKSUMMARYCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKSUMMARYCONTAINER_V1_H

#include "AthContainers/DataVector.h"

#include "xAODTracking/versions/TrackSummary_v1.h"
namespace xAOD {
    typedef DataVector<xAOD::TrackSummary_v1> TrackSummaryContainer_v1;
}


#endif