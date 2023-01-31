# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.PixelFastDigitizationTool"              , "PixelFastDigitizationTool")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.PixelFastDigitizationToolHS"            , "PixelFastDigitizationToolHS")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.PixelFastDigitizationToolPU"            , "PixelFastDigitizationToolPU")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.PixelFastDigitizationToolSplitNoMergePU", "PixelFastDigitizationToolSplitNoMergePU")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.getFastPixelRange"                      , "FastPixelRange")

addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.FastClusterMakerTool"                   , "FastClusterMakerTool")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.SCT_FastDigitizationTool"               , "SCT_FastDigitizationTool")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.SCT_FastDigitizationToolHS"             , "SCT_FastDigitizationToolHS")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.SCT_FastDigitizationToolPU"             , "SCT_FastDigitizationToolPU")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.SCT_FastDigitizationToolSplitNoMergePU" , "SCT_FastDigitizationToolSplitNoMergePU")
addTool("FastSiDigitization.FastSiDigitizationConfigLegacy.getFastSCTRange"                        , "FastSCTRange" )
