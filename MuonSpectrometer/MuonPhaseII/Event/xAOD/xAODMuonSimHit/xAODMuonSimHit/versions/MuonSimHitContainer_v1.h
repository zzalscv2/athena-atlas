/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAOMUONSIMHIT_VERSINO_MUONSIMHITCONTAINER_V1_H
#define XAOMUONSIMHIT_VERSINO_MUONSIMHITCONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODMuonSimHit/versions/MuonSimHit_v1.h"

namespace xAOD {
/// The container is a simple typedef for now
using MuonSimHitContainer_v1 = DataVector<xAOD::MuonSimHit_v1>;
}  // namespace xAOD

#endif