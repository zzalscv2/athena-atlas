/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"

#include "TrkTruthTPCnv/DetailedTrackTruthCollectionCnv_p2.h"
#include "TrkTruthTPCnv/DetailedTrackTruthCollection_p2.h"
#include "TrkTruthData/DetailedTrackTruthCollection.h"

DECLARE_TPCNV_FACTORY (DetailedTrackTruthCollectionCnv_p2,
                       DetailedTrackTruthCollection,
                       Trk::DetailedTrackTruthCollection_p2,
                       Athena::TPCnvVers::Current)
