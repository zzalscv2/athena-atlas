# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool, addAlgorithm

addTool("TGC_Digitization.TGC_DigitizationConfigLegacy.TgcDigitizationTool" , "TgcDigitizationTool")
addTool("TGC_Digitization.TGC_DigitizationConfigLegacy.getTgcRange"         , "TgcRange")
addTool("TGC_Digitization.TGC_DigitizationConfigLegacy.Tgc_OverlayDigitizationTool" , "Tgc_OverlayDigitizationTool")
addAlgorithm("TGC_Digitization.TGC_DigitizationConfigLegacy.getTGC_OverlayDigitizer" , "TGC_OverlayDigitizer")
