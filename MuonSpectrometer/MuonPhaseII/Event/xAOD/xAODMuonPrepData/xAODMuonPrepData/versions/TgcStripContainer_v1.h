/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_TGCSTRIPCONTAINER_V1_H
#define XAODMUONPREPDATA_VERSION_TGCSTRIPCONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/versions/TgcStrip_v1.h"

namespace xAOD {
/// The container is a simple typedef for now
typedef DataVector<xAOD::TgcStrip_v1> TgcStripContainer_v1;
}  // namespace xAOD

#endif