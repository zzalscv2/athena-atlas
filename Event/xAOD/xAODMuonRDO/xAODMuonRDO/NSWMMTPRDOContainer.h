/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NSWMMTPRDOOCONTAINER_H
#define XAODMUONRDO_NSWMMTPRDOOCONTAINER_H

#include "xAODMuonRDO/NSWMMTPRDO.h"
#include "xAODMuonRDO/versions/NSWMMTPRDOContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the NSW MM RDO container
    typedef NSWMMTPRDOContainer_v1 NSWMMTPRDOContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NSWMMTPRDOContainer , 1095524642 , 1 )

#endif // XAODMUONRDO_NSWMMTPRDOOCONTAINER_H

