# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool,addAlgorithm
addTool("sTGC_Digitization.sTGC_DigitizationConfigLegacy.sTgcDigitizationTool","sTgcDigitizationTool")
addTool("sTGC_Digitization.sTGC_DigitizationConfigLegacy.getSTGCRange","sTgcRange")
addTool("sTGC_Digitization.sTGC_DigitizationConfigLegacy.STGC_OverlayDigitizationTool", "STGC_OverlayDigitizationTool")
addAlgorithm("sTGC_Digitization.sTGC_DigitizationConfigLegacy.getSTGC_OverlayDigitizer", "STGC_OverlayDigitizer")
