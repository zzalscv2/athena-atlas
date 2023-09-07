# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MdtSensitiveDetectorToolCfg(flags, name = "MdtSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xMdtSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::MDTDriftGas"])
    the_tool = CompFactory.MuonG4R4.MdtSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def RpcSensitiveDetectorToolCfg(flags, name = "RpcSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRpcSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::RpcGasGap"])
    the_tool = CompFactory.MuonG4R4.RpcSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def SetupSensitiveDetectorsCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableMDT:
        tools += [result.popToolsAndMerge(MdtSensitiveDetectorToolCfg(flags))]
    
    if flags.Detector.EnableRPC:
        tools += [result.popToolsAndMerge(RpcSensitiveDetectorToolCfg(flags))]

    result.setPrivateTools(tools)
    return result