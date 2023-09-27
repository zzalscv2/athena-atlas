/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCSTRIP_H
#define XAODMUONPREPDATA_RPCSTRIP_H

#include "xAODMuonPrepData/versions/RpcStrip_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the RpcStrip
typedef RpcStrip_v1 RpcStrip;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::RpcStrip, 47111859, 1)

#endif  // XAODMUONPREPDATA_RPCSTRIP_H
