#include "GaudiKernel/DeclareFactoryEntries.h"

#include "LArRecUtils/LArADC2MeVTool.h"
#include "LArRecUtils/LArAutoCorrNoiseTool.h"
#include "LArRecUtils/LArAutoCorrTotalTool.h"
#include "LArRecUtils/LArCellFakeProbElectronics.h"
//#include "LArRecUtils/LArCellFakeProbHV.h"
#include "LArRecUtils/LArHVCorrTool.h"
//#include "LArRecUtils/LArHVGeometryTool.h"
#include "LArRecUtils/LArOFCTool.h"
#include "LArRecUtils/LArOFPeakRecoTool.h"
#include "LArRecUtils/LArParabolaPeakRecoTool.h"
#include "LArRecUtils/LArShapePeakRecoTool.h"
#include "LArRecUtils/LArTowerBuilderTool.h"
#include "LArRecUtils/LArFCalTowerBuilderTool.h"
#include "LArRecUtils/LArFEBConfigReader.h"
#include "LArRecUtils/LArFlatConditionSvc.h"
#include "../LArFCalTowerBuilderToolTestAlg.h"
#include "../LArHVScaleRetriever.h"
#include "../LArFlatConditionsAlg.h"
#include "../LArOnOffMappingAlg.h"
#include "../LArSymConditionsAlg.h"
#include "../LArMCSymCondAlg.h"

DECLARE_TOOL_FACTORY( LArADC2MeVTool )
DECLARE_TOOL_FACTORY( LArAutoCorrNoiseTool )
DECLARE_TOOL_FACTORY( LArAutoCorrTotalTool )
DECLARE_TOOL_FACTORY( LArCellFakeProbElectronics )
//DECLARE_TOOL_FACTORY( LArCellFakeProbHV )
DECLARE_TOOL_FACTORY( LArHVCorrTool )
//DECLARE_TOOL_FACTORY( LArHVGeometryTool )
DECLARE_TOOL_FACTORY( LArOFCTool )
DECLARE_TOOL_FACTORY( LArOFPeakRecoTool )
DECLARE_TOOL_FACTORY( LArParabolaPeakRecoTool )
DECLARE_TOOL_FACTORY( LArShapePeakRecoTool )
DECLARE_TOOL_FACTORY( LArTowerBuilderTool )
DECLARE_TOOL_FACTORY( LArFCalTowerBuilderTool )
DECLARE_TOOL_FACTORY( LArFEBConfigReader )
DECLARE_SERVICE_FACTORY( LArFlatConditionSvc)
DECLARE_ALGORITHM_FACTORY( LArFCalTowerBuilderToolTestAlg )
DECLARE_TOOL_FACTORY( LArHVScaleRetriever )
DECLARE_ALGORITHM_FACTORY( LArCondAlgAutoCorrSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgDAC2uAFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlgDAC2uASC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgHVScaleCorrFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlgHVScaleCorrSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgMinBiasSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgMphysOverMcalFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlgMphysOverMcalSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgNoiseSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgOFCFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlgPedestalFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlgPedestalSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgRampFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlgRampSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgShapeFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlgShapeSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlgfSamplSC)
DECLARE_ALGORITHM_FACTORY( LArCondAlguA2MeVFlat)
DECLARE_ALGORITHM_FACTORY( LArCondAlguA2MeVSC)
DECLARE_ALGORITHM_FACTORY( LArOnOffMappingAlg)
DECLARE_ALGORITHM_FACTORY( LArMCSymCondAlg)
DECLARE_ALGORITHM_FACTORY( LArRampSymCondAlg )



/*
DECLARE_FACTORY_ENTRIES(LArRecUtils) {
	DECLARE_TOOL( LArADC2MeVTool )
	DECLARE_TOOL( LArAutoCorrNoiseTool )
	DECLARE_TOOL( LArAutoCorrTotalTool )
	DECLARE_TOOL( LArCellFakeProbElectronics )
	  //	DECLARE_TOOL( LArCellFakeProbHV )
	DECLARE_TOOL( LArHVCorrTool )
	  //	DECLARE_TOOL( LArHVGeometryTool )
	DECLARE_TOOL( LArOFCTool )
	DECLARE_TOOL( LArOFPeakRecoTool )
	DECLARE_TOOL( LArParabolaPeakRecoTool )
	DECLARE_TOOL( LArShapePeakRecoTool )
        DECLARE_TOOL( LArTowerBuilderTool )
        DECLARE_TOOL( LArFCalTowerBuilderTool )
	DECLARE_TOOL(LArFEBConfigReader)
	DECLARE_SERVICE( LArFlatConditionSvc )
	DECLARE_ALGORITHM( LArFCalTowerBuilderToolTestAlg )
  DECLARE_TOOL( LArHVScaleRetriever )

	  DECLARE_ALGORITHM( LArFlatCondAlgHVScale)
	  DECLARE_ALGORITHM( LArFlatCondAlgPedestal)
	  
}
*/


//needed for LArCellCorrection, which is the base class of LArG3Escale; 
//LArG3Escale is declared as a factory in LArCellRec
