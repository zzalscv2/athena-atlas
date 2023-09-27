/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_RPCSTRIPCONTAINER_H
#define XAODMUONPREPDATA_RPCSTRIPCONTAINER_H

#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/versions/RpcStripContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
typedef RpcStripContainer_v1 RpcStripContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::RpcStripContainer, 1274417297, 1)

#endif