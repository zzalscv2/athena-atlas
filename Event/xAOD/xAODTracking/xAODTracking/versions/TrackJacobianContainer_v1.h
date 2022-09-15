/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKJACOBIANCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKJACOBIANCONTAINER_V1_H

#include "AthContainers/DataVector.h"

#include "xAODTracking/versions/TrackJacobian_v1.h"
namespace xAOD {
    typedef DataVector<xAOD::TrackJacobian_v1> TrackJacobianContainer_v1;
}


#endif