# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool, addAlgorithm

addTool("RPC_Digitization.RPC_DigitizationConfigLegacy.RpcDigitizationTool" , "RpcDigitizationTool")
addTool("RPC_Digitization.RPC_DigitizationConfigLegacy.getRpcRange"         , "RpcRange")
addTool("RPC_Digitization.RPC_DigitizationConfigLegacy.Rpc_OverlayDigitizationTool" , "Rpc_OverlayDigitizationTool")
addAlgorithm("RPC_Digitization.RPC_DigitizationConfigLegacy.getRPC_OverlayDigitizer" , "RPC_OverlayDigitizer")
