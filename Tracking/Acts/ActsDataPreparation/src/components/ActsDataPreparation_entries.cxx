/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/PixelClusterizationAlg.h"
#include "src/StripClusterizationAlg.h"
#include "src/PixelSpacePointFormationAlg.h"
#include "src/StripSpacePointFormationAlg.h"
#include "src/ActsPixelClusteringTool.h"
#include "src/ActsStripClusteringTool.h"
#include "src/PixelSpacePointFormationTool.h"
#include "src/ActsCoreStripSpacePointFormationTool.h"
#include "src/ActsTrkStripSpacePointFormationTool.h"

// Algs
DECLARE_COMPONENT(ActsTrk::PixelClusterizationAlg)
DECLARE_COMPONENT(ActsTrk::StripClusterizationAlg)
DECLARE_COMPONENT( ActsTrk::PixelSpacePointFormationAlg )
DECLARE_COMPONENT( ActsTrk::StripSpacePointFormationAlg )
// Tools
DECLARE_COMPONENT(ActsTrk::PixelClusteringTool)
DECLARE_COMPONENT(ActsTrk::StripClusteringTool)
DECLARE_COMPONENT( ActsTrk::PixelSpacePointFormationTool )
DECLARE_COMPONENT( ActsTrk::ActsCoreStripSpacePointFormationTool )
DECLARE_COMPONENT( ActsTrk::ActsTrkStripSpacePointFormationTool )
