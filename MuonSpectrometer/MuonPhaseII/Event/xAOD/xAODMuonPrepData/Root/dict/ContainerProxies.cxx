/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AddDVProxy.h"

// Local include(s):
#include "xAODMuonPrepData/versions/MdtDriftCircleContainer_v1.h"
#include "xAODMuonPrepData/versions/RpcStripContainer_v1.h"
#include "xAODMuonPrepData/versions/TgcStripContainer_v1.h"

// Set up the collection proxies:
ADD_NS_DV_PROXY(xAOD, MdtDriftCircleContainer_v1);
ADD_NS_DV_PROXY(xAOD, RpcStripContainer_v1);
ADD_NS_DV_PROXY(xAOD, TgcStripContainer_v1);
