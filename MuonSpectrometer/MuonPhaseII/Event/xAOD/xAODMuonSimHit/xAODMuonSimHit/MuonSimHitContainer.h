/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONSIMHIT_MUONSIMHITCONTAINER_H
#define XAODMUONSIMHIT_MUONSIMHITCONTAINER_H

#include "xAODMuonSimHit/MuonSimHit.h"
#include "xAODMuonSimHit/versions/MuonSimHitContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
using MuonSimHitContainer =  MuonSimHitContainer_v1;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MuonSimHitContainer , 1326558483 , 1 )
#endif