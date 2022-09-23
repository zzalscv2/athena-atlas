
#include "GaudiKernel/DeclareFactoryEntries.h"

#include "DiTauRec/DiTauBuilder.h"
#include "DiTauRec/DiTauToolBase.h"
#include "DiTauRec/SeedJetBuilder.h"
#include "DiTauRec/SubjetBuilder.h"
#include "DiTauRec/ElMuFinder.h"
#include "DiTauRec/VertexFinder.h"
#include "DiTauRec/DiTauTrackFinder.h"
#include "DiTauRec/CellFinder.h"
#include "DiTauRec/IDVarCalculator.h"

#include "DiTauRec/MuHadProcessorTool.h"
#include "DiTauRec/MuHadAxisSetter.h"
#include "DiTauRec/MuHadTrackFinder.h"
#include "DiTauRec/MuHadVertexVariables.h"
#include "DiTauRec/MuHadClusterSubStructVariables.h"
#include "DiTauRec/MuHadJetRNNEvaluator.h"
#include "DiTauRec/MuHadIDVarCalculator.h"
#include "DiTauRec/MuHadElectronVetoVariables.h"

DECLARE_ALGORITHM_FACTORY( DiTauBuilder )
DECLARE_TOOL_FACTORY( SeedJetBuilder )
DECLARE_TOOL_FACTORY( SubjetBuilder )
DECLARE_TOOL_FACTORY( ElMuFinder )
DECLARE_TOOL_FACTORY( VertexFinder )
DECLARE_TOOL_FACTORY( DiTauTrackFinder )
DECLARE_TOOL_FACTORY( CellFinder )
DECLARE_TOOL_FACTORY( IDVarCalculator )

DECLARE_TOOL_FACTORY( MuHadProcessorTool )
DECLARE_TOOL_FACTORY( MuHadAxisSetter )
DECLARE_TOOL_FACTORY( MuHadTrackFinder )
DECLARE_TOOL_FACTORY( MuHadVertexVariables )
DECLARE_TOOL_FACTORY( MuHadClusterSubStructVariables )
DECLARE_TOOL_FACTORY( MuHadJetRNNEvaluator )
DECLARE_TOOL_FACTORY( MuHadIDVarCalculator )
DECLARE_TOOL_FACTORY( MuHadElectronVetoVariables )

DECLARE_FACTORY_ENTRIES( DiTauRec ) 
{
  DECLARE_ALGORITHM( DiTauBuilder );
  DECLARE_TOOL( SeedJetBuilder );
  DECLARE_TOOL( SubjetBuilder );
  DECLARE_TOOL( ElMuFinder );
  DECLARE_TOOL( VertexFinder );
  DECLARE_TOOL( DiTauTrackFinder );
  DECLARE_TOOL( CellFinder );
  DECLARE_TOOL( IDVarCalculator );

  DECLARE_TOOL( MuHadProcessorTool )
  DECLARE_TOOL( MuHadAxisSetter )
  DECLARE_TOOL( MuHadTrackFinder )
  DECLARE_TOOL( MuHadVertexVariables )
  DECLARE_TOOL( MuHadClusterSubStructVariables ) 
  DECLARE_TOOL( MuHadJetRNNEvaluator )
  DECLARE_TOOL( MuHadIDVarCalculator )
  DECLARE_TOOL( MuHadElectronVetoVariables )

}
