/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NNSWTPRDOAUXCONTAINER_H
#define XAODMUONRDO_NNSWTPRDOAUXCONTAINER_H

// Local include(s):
#include "xAODMuonRDO/versions/NSWTPRDOAuxContainer_v1.h"

namespace xAOD {
   /// Definition of the current NRPC RDO auxiliary container
   ///
   typedef NSWTPRDOAuxContainer_v1 NSWTPRDOAuxContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NSWTPRDOAuxContainer , 1099671436 , 1 )
#endif // XAODMUONRDO_NRPCRDOAUXCONTAINER_H
