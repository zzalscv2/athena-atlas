/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingAlg.h"
#include "src/AmbiguityResolutionAlg.h"
#include "src/ActsReFitterAlg.h"
#include "src/ActsCompareTrackAlg.h"

// Tools
#include "src/TrackStatePrinter.h"
#include "src/ActsKalmanFitter.h"
#include "src/ActsGaussianSumFitter.h"
#include "src/ProtoTrackCreationAndFitAlg.h"
#include "src/ProtoTrackReportingAlg.h"
#include "src/RandomProtoTrackCreator.h"
#include "src/TruthGuidedProtoTrackCreator.h"
// Algs
DECLARE_COMPONENT( ActsTrk::TrackFindingAlg )
DECLARE_COMPONENT( ActsTrk::ActsReFitterAlg )
DECLARE_COMPONENT( ActsTrk::ActsCompareTrackAlg )
DECLARE_COMPONENT( ActsTrk::AmbiguityResolutionAlg )
DECLARE_COMPONENT( ActsTrk::ProtoTrackCreationAndFitAlg )
DECLARE_COMPONENT( ActsTrk::ProtoTrackReportingAlg )

// Tools
DECLARE_COMPONENT( ActsTrk::TrackStatePrinter )
DECLARE_COMPONENT( ActsTrk::ActsKalmanFitter )
DECLARE_COMPONENT( ActsTrk::ActsGaussianSumFitter )
DECLARE_COMPONENT( ActsTrk::RandomProtoTrackCreator )
DECLARE_COMPONENT( ActsTrk::TruthGuidedProtoTrackCreator )
