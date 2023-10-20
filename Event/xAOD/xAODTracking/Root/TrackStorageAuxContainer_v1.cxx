/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackStorageAuxContainer_v1.h"

namespace xAOD {
  TrackStorageAuxContainer_v1::TrackStorageAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(params);
    AUX_VARIABLE(covParams);
    AUX_VARIABLE(nMeasurements);
    AUX_VARIABLE(nHoles);
    AUX_VARIABLE(chi2f);
    AUX_VARIABLE(ndf);
    AUX_VARIABLE(nOutliers);
    AUX_VARIABLE(nSharedHits);    
  }
}
