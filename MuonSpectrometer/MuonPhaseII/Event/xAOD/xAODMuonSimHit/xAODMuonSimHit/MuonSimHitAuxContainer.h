/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONSIMHIT_MUONSIMHITAUXCONTAINER_H
#define XAODMUONSIMHIT_MUONSIMHITAUXCONTAINER_H

#include "xAODMuonSimHit/versions/MuonSimHitAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
using MuonSimHitAuxContainer =  MuonSimHitAuxContainer_v1;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MuonSimHitAuxContainer , 1086502646 , 1 )
#endif