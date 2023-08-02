/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NSWMMTPRDOAUXCONTAINER_H
#define XAODMUONRDO_NSWMMTPRDOAUXCONTAINER_H

// Local include(s):
#include "xAODMuonRDO/versions/NSWMMTPRDOAuxContainer_v1.h"

namespace xAOD {
   /// Definition of the current NSW MM RDO auxiliary container
   ///
   typedef NSWMMTPRDOAuxContainer_v1 NSWMMTPRDOAuxContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NSWMMTPRDOAuxContainer , 1099671437 , 1 )
#endif // XAODMUONRDO_NSWMMTPRDOAUXCONTAINER_H
