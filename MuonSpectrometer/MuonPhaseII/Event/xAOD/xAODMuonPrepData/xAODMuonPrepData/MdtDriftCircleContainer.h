/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_MDTDRIFTCIRCLECONTAINER_H
#define XAODMUONPREPDATA_MDTDRIFTCIRCLECONTAINER_H

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/versions/MdtDriftCircleContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
typedef MdtDriftCircleContainer_v1 MdtDriftCircleContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MdtDriftCircleContainer, 1098001914, 1)

#endif