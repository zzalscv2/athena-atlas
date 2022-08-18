# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool

addTool("CSC_Digitization.CSC_DigitizationConfigLegacy.getCscDigitizationTool" , "CscDigitizationTool")
addTool("CSC_Digitization.CSC_DigitizationConfigLegacy.getCscOverlayDigitizationTool" , "CscOverlayDigitizationTool")
addTool("CSC_Digitization.CSC_DigitizationConfigLegacy.getCscOverlayDigitBuilder" , "CscOverlayDigitBuilder")
addTool("CSC_Digitization.CSC_DigitizationConfigLegacy.getCscRange"         , "CscRange")
