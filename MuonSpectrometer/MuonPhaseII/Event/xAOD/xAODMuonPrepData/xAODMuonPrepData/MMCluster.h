/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MMCLUSTER_H
#define XAODMUONPREPDATA_MMCLUSTER_H

#include "xAODMuonPrepData/versions/MMCluster_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the MMCluster
typedef MMCluster_v1 MMCluster;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MMCluster, 95201827, 1)

#endif  // XAODMUONPREPDATA_MMCluster_H
