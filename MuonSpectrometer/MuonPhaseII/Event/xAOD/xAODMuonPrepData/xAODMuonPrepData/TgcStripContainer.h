/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_TGCSTRIPCONTAINER_H
#define XAODMUONPREPDATA_TGCSTRIPCONTAINER_H

#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/versions/TgcStripContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
typedef TgcStripContainer_v1 TgcStripContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::TgcStripContainer , 1245357318 , 1 )

#endif