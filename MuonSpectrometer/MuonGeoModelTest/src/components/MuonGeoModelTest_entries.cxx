/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "../GeoModelCscTest.h"
#include "../GeoModelMdtTest.h"
#include "../GeoModelRpcTest.h"
#include "../GeoModelTgcTest.h"
#include "../NSWGeoPlottingAlg.h"
#include "../sTgcPadPlottingAlg.h"

#include "MuonGeoModelTest/MuonGMCheck.h"
#include "MuonGeoModelTest/MuonGMTestOnPrd.h"
#include "MuonGeoModelTest/MuonHitRelocation.h"

DECLARE_COMPONENT(MuonGMCheck)
DECLARE_COMPONENT(MuonGMTestOnPrd)
DECLARE_COMPONENT(MuonHitRelocation)
DECLARE_COMPONENT(NSWGeoPlottingAlg)
DECLARE_COMPONENT(sTgcPadPlottingAlg)
DECLARE_COMPONENT(MuonGM::GeoModelCscTest)
DECLARE_COMPONENT(MuonGM::GeoModelMdtTest)
DECLARE_COMPONENT(MuonGM::GeoModelRpcTest)
DECLARE_COMPONENT(MuonGM::GeoModelTgcTest)
