#ifndef XAOD_ANALYSIS
#include "HIEventUtils/ExtractCaloGeoConstants.h"
#endif
#include "HIEventUtils/HIEventShapeSummaryTool.h"
#include "HIEventUtils/HIVertexSelectionTool.h"
#include "HIEventUtils/HIEventSelectionTool.h"
#include "HIEventUtils/HITowerWeightTool.h"
#include "HIEventUtils/HIEventShapeMapTool.h"
#include "HIEventUtils/ZdcRecTool.h"

#ifndef XAOD_ANALYSIS
DECLARE_COMPONENT( ExtractCaloGeoConstants )
#endif
DECLARE_COMPONENT( HIEventShapeSummaryTool )
DECLARE_COMPONENT( HITowerWeightTool )
DECLARE_COMPONENT( HIEventShapeMapTool )
DECLARE_COMPONENT( HI::HIVertexSelectionTool )
DECLARE_COMPONENT( HI::HIEventSelectionTool )
DECLARE_COMPONENT( ZDC::ZdcRecTool )
