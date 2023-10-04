/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MMClusterAUXCONTAINER_H
#define XAODMUONPREPDATA_MMClusterAUXCONTAINER_H

#include "xAODMuonPrepData/versions/MMClusterAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the MMCluster
typedef MMClusterAuxContainer_v1 MMClusterAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MMClusterAuxContainer, 1268473872, 1)
#endif  // XAODMUONRDO_NMMRDO_H
