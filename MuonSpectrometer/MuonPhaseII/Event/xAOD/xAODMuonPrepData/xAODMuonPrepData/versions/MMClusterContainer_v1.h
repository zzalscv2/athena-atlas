/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_MMCLUSTERCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSION_MMCLUSTERCONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/versions/MMCluster_v1.h"

namespace xAOD {
/// The container is a simple typedef for now
typedef DataVector<xAOD::MMCluster_v1> MMClusterContainer_v1;
}  // namespace xAOD

#endif