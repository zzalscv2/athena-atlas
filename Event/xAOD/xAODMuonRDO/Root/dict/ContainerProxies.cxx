/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AddDVProxy.h"

// Local include(s):
#include "xAODMuonRDO/versions/NRPCRDOContainer_v1.h"
#include "xAODMuonRDO/versions/NSWTPRDOContainer_v1.h"


// Set up the collection proxies:
ADD_NS_DV_PROXY( xAOD, NRPCRDOContainer_v1 );
ADD_NS_DV_PROXY( xAOD, NSWTPRDOContainer_v1 );


