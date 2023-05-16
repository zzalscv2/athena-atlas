/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NSWTPRDO_H
#define XAODMUONRDO_NSWTPRDO_H

#include "xAODMuonRDO/versions/NSWTPRDO_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the NRPC RDO class
    typedef NSWTPRDO_v1 NSWTPRDO;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NSWTPRDO , 78146051 , 1 )


#endif // XAODMUONRDO_NSWTPRDO_H

