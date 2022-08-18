# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool, addAlgorithm

addTool("MDT_Digitization.MdtDigitizationConfigLegacy.MdtDigitizationTool"     , "MdtDigitizationTool")
addTool("MDT_Digitization.MdtDigitizationConfigLegacy.RT_Relation_DB_DigiTool" , "RT_Relation_DB_DigiTool")
addTool("MDT_Digitization.MdtDigitizationConfigLegacy.MDT_Response_DigiTool"   , "MDT_Response_DigiTool")
addTool("MDT_Digitization.MdtDigitizationConfigLegacy.getMdtRange"             , "MdtRange")
addTool("MDT_Digitization.MdtDigitizationConfigLegacy.Mdt_OverlayDigitizationTool", "Mdt_OverlayDigitizationTool")
addAlgorithm("MDT_Digitization.MdtDigitizationConfigLegacy.getMDT_OverlayDigitizer", "MDT_OverlayDigitizer")

