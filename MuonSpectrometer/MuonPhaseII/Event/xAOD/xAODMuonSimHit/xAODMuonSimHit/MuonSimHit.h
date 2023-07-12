/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHIT_MUONSIMHIT_H
#define XAODMUONSIMHIT_MUONSIMHIT_H

#include "xAODMuonSimHit/versions/MuonSimHit_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the MuonSimHit
using MuonSimHit = MuonSimHit_v1;
}  
// namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MuonSimHit , 31103601 , 1 )

#endif  // XAODMUONRDO_NRPCRDO_H
