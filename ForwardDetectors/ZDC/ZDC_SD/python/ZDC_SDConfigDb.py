# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool
addTool("ZDC_SD.ZDC_SDConfigLegacy.getZDC_FiberSD"  , "ZDC_FiberSD"   )
addTool("ZDC_SD.ZDC_SDConfigLegacy.getZDC_G4CalibSD", "ZDC_G4CalibSD" )
addTool("ZDC_SD.ZDC_SDConfigLegacy.getZDC_StripSD"       , "ZDC_StripSD" )
addTool("ZDC_SD.ZDC_SDConfigLegacy.getZDC_PixelSD"       , "ZDC_PixelSD" )
