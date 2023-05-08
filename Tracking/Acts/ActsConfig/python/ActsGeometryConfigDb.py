# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.CfgGetter import addService, addTool

addService("ActsConfig.ActsGeometryConfigLegacy.getActsTrackingGeometrySvc",  "ActsTrackingGeometrySvc")
addTool("ActsConfig.ActsGeometryConfigLegacy.getActsTrackingGeometryTool", "ActsTrackingGeometryTool")
addTool("ActsConfig.ActsGeometryConfigLegacy.getActsExtrapolationTool", "ActsExtrapolationTool")
