/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NRPCRDO_H
#define XAODMUONRDO_NRPCRDO_H

#include "xAODMuonRDO/versions/NRPCRDO_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the NRPC RDO class
    typedef NRPCRDO_v1 NRPCRDO;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NRPCRDO , 243725689 , 1 )


#endif // XAODMUONRDO_NRPCRDO_H

