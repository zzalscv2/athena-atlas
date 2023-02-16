/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_VERSION_NRPCRDOCONTAINER_V1_H
#define XAODMUONRDO_VERSION_NRPCRDOCONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODMuonRDO/versions/NRPCRDO_v1.h"

namespace xAOD {
    /// The container is a simple typedef for now
    typedef DataVector< xAOD::NRPCRDO_v1 > NRPCRDOContainer_v1;
}

#endif // XAODMUONRDO_VERSION_NRPCRDOCONTAINER_V1_H
