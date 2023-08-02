/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NSWMMTPRDO_H
#define XAODMUONRDO_NSWMMTPRDO_H

#include "xAODMuonRDO/versions/NSWMMTPRDO_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the NSW MM RDO class
    typedef NSWMMTPRDO_v1 NSWMMTPRDO;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NSWMMTPRDO , 78146052 , 1 )


#endif // XAODMUONRDO_NSWMMTPRDO_H

