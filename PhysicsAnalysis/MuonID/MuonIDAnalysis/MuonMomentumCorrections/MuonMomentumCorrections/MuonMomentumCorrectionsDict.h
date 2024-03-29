/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMOMENTUMCORRECTIONS_MUONMOMENTUMCORRECTIONSDICT_H
#define MUONMOMENTUMCORRECTIONS_MUONMOMENTUMCORRECTIONSDICT_H

#if defined(__GCCXML__) and not defined(EIGEN_DONT_VECTORIZE)
#define EIGEN_DONT_VECTORIZE
#endif  // __GCCXML__

#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"



#include "MuonMomentumCorrections/MuonCalibTool.h"
#include "MuonMomentumCorrections/IMuonCalibIntTool.h"
#include "MuonMomentumCorrections/MuonCalibIntSagittaTool.h"
#include "MuonMomentumCorrections/MuonCalibIntScaleSmearTool.h"
#include "MuonMomentumCorrections/MuonCalibIntHighpTSmearTool.h"

#endif  // not MUONMOMENTUMCORRECTIONS_MUONMOMENTUMCORRECTIONSDICT_H
