/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCSTRIPAUXCONTAINER_H
#define XAODMUONPREPDATA_RPCSTRIPAUXCONTAINER_H

#include "xAODMuonPrepData/versions/RpcStripAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the RpcStrip
typedef RpcStripAuxContainer_v1 RpcStripAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::RpcStripAuxContainer, 1313829788, 1)
#endif  // XAODMUONRDO_NRPCRDO_H
