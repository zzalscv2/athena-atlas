# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def MdtSensitiveDetectorToolCfg(flags, name = "MdtSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xMdtSimHits"])
    the_tool = CompFactory.MuonG4R4.MDTSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def SetupSensitiveDetectorsCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Detector.EnableMDT:
        tools += [result.popToolsAndMerge(MdtSensitiveDetectorToolCfg(flags))]
    result.setPrivateTools(tools)
    
    return result