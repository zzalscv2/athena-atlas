/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTMEASUREMENT_H
#define XAODMUONPREPDATA_MDTMEASUREMENT_H

#include "xAODMuonPrepData/versions/MdtDriftCircle_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the MdtDriftCircle
typedef MdtDriftCircle_v1 MdtDriftCircle;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::MdtDriftCircle, 92538182, 1)

#endif  // XAODMUONRDO_NRPCRDO_H
