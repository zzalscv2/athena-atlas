/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_NRPCRDOAUXCONTAINER_H
#define XAODMUONRDO_NRPCRDOAUXCONTAINER_H

// Local include(s):
#include "xAODMuonRDO/versions/NRPCRDOAuxContainer_v1.h"

namespace xAOD {
   /// Definition of the current NRPC RDO auxiliary container
   ///
   typedef NRPCRDOAuxContainer_v1 NRPCRDOAuxContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NRPCRDOAuxContainer , 1178508698 , 1 )

#endif // XAODMUONRDO_NRPCRDOAUXCONTAINER_H
