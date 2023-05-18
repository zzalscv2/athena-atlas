/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NSWTPRDOOCONTAINER_H
#define XAODMUONRDO_NSWTPRDOOCONTAINER_H

#include "xAODMuonRDO/NSWTPRDO.h"
#include "xAODMuonRDO/versions/NSWTPRDOContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the NRPC RDO container
    typedef NSWTPRDOContainer_v1 NSWTPRDOContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NSWTPRDOContainer , 1095524641 , 1 )

#endif // XAODMUONRDO_NRPCRDOCONTAINER_H

