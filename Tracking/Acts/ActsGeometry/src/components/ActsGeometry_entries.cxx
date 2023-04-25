/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "EventPrimitives/EventPrimitives.h"
// needed here to get the ATLAS eigen plugins in before the ACTS eigen plugins
#include "../ActsAlignmentCondAlg.h"
#include "../ActsDetAlignCondAlg.h"
#include "ActsGeometry/ActsCaloTrackingVolumeBuilder.h"
#include "ActsGeometry/ActsExtrapolationAlg.h"
#include "ActsGeometry/ActsExtrapolationTool.h"
#include "ActsGeometry/ActsMaterialJsonWriterTool.h"
#include "ActsGeometry/ActsMaterialMapping.h"
#include "ActsGeometry/ActsMaterialStepConverterTool.h"
#include "ActsGeometry/ActsMaterialTrackWriterSvc.h"
#include "ActsGeometry/ActsObjWriterTool.h"
#include "ActsGeometry/ActsPropStepRootWriterSvc.h"
#include "ActsGeometry/ActsSurfaceMappingTool.h"
#include "ActsGeometry/ActsTrackingGeometrySvc.h"
#include "ActsGeometry/ActsTrackingGeometryTool.h"
#include "ActsGeometry/ActsVolumeMappingTool.h"
#include "ActsGeometry/ActsWriteTrackingGeometry.h"
#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"

DECLARE_COMPONENT(ActsExtrapolationAlg)
DECLARE_COMPONENT(ActsWriteTrackingGeometry)
DECLARE_COMPONENT(ActsWriteTrackingGeometryTransforms)
DECLARE_COMPONENT(ActsTrackingGeometrySvc)
DECLARE_COMPONENT(ActsExtrapolationTool)

DECLARE_COMPONENT(ActsMaterialMapping)
DECLARE_COMPONENT(ActsSurfaceMappingTool)
DECLARE_COMPONENT(ActsVolumeMappingTool)
DECLARE_COMPONENT(ActsObjWriterTool)
// DECLARE_COMPONENT( ActsExCellWriterSvc )
DECLARE_COMPONENT(ActsMaterialTrackWriterSvc)
DECLARE_COMPONENT(ActsMaterialStepConverterTool)
DECLARE_COMPONENT(ActsMaterialJsonWriterTool)

DECLARE_COMPONENT(ActsTrackingGeometryTool)

DECLARE_COMPONENT(ActsPropStepRootWriterSvc)
DECLARE_COMPONENT(ActsAlignmentCondAlg)
DECLARE_COMPONENT(ActsDetAlignCondAlg)
DECLARE_COMPONENT(ActsCaloTrackingVolumeBuilder)
