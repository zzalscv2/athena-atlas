/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NRPCRDOCONTAINER_H
#define XAODMUONRDO_NRPCRDOCONTAINER_H

#include "xAODMuonRDO/NRPCRDO.h"
#include "xAODMuonRDO/versions/NRPCRDOContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the NRPC RDO container
    typedef NRPCRDOContainer_v1 NRPCRDOContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NRPCRDOContainer , 1129912423 , 1 )

#endif // XAODMUONRDO_NRPCRDOCONTAINER_H

