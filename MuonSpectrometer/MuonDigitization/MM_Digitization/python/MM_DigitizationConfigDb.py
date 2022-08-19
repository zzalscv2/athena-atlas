# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool,addAlgorithm
addTool("MM_Digitization.MM_DigitizationConfigLegacy.MM_DigitizationTool","MM_DigitizationTool")
addTool("MM_Digitization.MM_DigitizationConfigLegacy.getMMRange", "MMRange")
addTool("MM_Digitization.MM_DigitizationConfigLegacy.MM_OverlayDigitizationTool", "MM_OverlayDigitizationTool")
addAlgorithm("MM_Digitization.MM_DigitizationConfigLegacy.getMM_OverlayDigitizer", "MM_OverlayDigitizer")
