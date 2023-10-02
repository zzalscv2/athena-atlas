#include "PFlowUtils/WeightPFOTool.h"

#if !defined(XAOD_ANALYSIS)
#include "../PFlowCellCPDataDecoratorAlgorithm.h"
#include "../PFlowCalibPFODecoratorAlgorithm.h"
#endif

DECLARE_COMPONENT( CP::WeightPFOTool )

#if !defined(XAOD_ANALYSIS)
DECLARE_COMPONENT( PFlowCellCPDataDecoratorAlgorithm )
DECLARE_COMPONENT( PFlowCalibPFODecoratorAlgorithm )
#endif
