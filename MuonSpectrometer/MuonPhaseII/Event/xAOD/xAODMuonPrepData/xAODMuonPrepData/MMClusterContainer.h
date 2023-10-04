/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_MMClusterCONTAINER_H
#define XAODMUONPREPDATA_MMClusterCONTAINER_H

#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/versions/MMClusterContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
typedef MMClusterContainer_v1 MMClusterContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MMClusterContainer, 1171576513, 1)

#endif