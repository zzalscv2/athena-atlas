
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/DeclareFactoryEntries.h"

#include "PMGTools/PMGCrossSectionTool.h"
#include "PMGTools/PMGDecayProductsSelectionTool.h"
#include "PMGTools/PMGSherpa22VJetsWeightTool.h"
#include "PMGTools/PMGSherpaVjetsSysTool.h"
#include "PMGTools/PMGTruthWeightTool.h"
#include "PMGTools/PMGHFProductionFractionTool.h"

using namespace PMGTools;

DECLARE_TOOL_FACTORY( PMGCrossSectionTool )
DECLARE_TOOL_FACTORY( PMGDecayProductsSelectionTool )
DECLARE_TOOL_FACTORY( PMGSherpa22VJetsWeightTool )
DECLARE_TOOL_FACTORY( PMGSherpaVjetsSysTool )
DECLARE_TOOL_FACTORY( PMGTruthWeightTool )
DECLARE_TOOL_FACTORY( PMGHFProductionFractionTool )

DECLARE_FACTORY_ENTRIES( PMGTools )
{
  DECLARE_TOOL( PMGCrossSectionTool );
  DECLARE_TOOL( PMGDecayProductsSelectionTool )
  DECLARE_TOOL( PMGSherpa22VJetsWeightTool );
  DECLARE_TOOL( PMGSherpaVjetsSysTool );
  DECLARE_TOOL( PMGTruthWeightTool );
  DECLARE_TOOL( PMGHFProductionFractionTool );
}
