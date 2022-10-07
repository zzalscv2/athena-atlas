/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "../IsoCloseByCaloDecorAlg.h"
#include "../IsoCloseByCorrectionTrkSelAlg.h"
#include "../TestIsolationAthenaAlg.h"
#include "../TestIsolationCloseByCorrAlg.h"
#include "IsolationSelection/IsolationCloseByCorrectionTool.h"
#include "IsolationSelection/IsolationLowPtPLVTool.h"
#include "IsolationSelection/IsolationSelectionTool.h"

DECLARE_COMPONENT(CP::IsolationSelectionTool)
DECLARE_COMPONENT(CP::TestIsolationAthenaAlg)
DECLARE_COMPONENT(CP::IsolationCloseByCorrectionTool)
DECLARE_COMPONENT(CP::TestIsolationCloseByCorrAlg)
DECLARE_COMPONENT(CP::IsoCloseByCorrectionTrkSelAlg)
DECLARE_COMPONENT(CP::IsoCloseByCaloDecorAlg)
DECLARE_COMPONENT(CP::IsolationLowPtPLVTool)
