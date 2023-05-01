# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MdtReadoutGeomToolCfg(flags, name="MdtReadoutGeomTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonGMR4.MdtReadoutGeomTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonDetectorToolCfg(flags, name="MuonDetectorToolR4", **kwargs):
    result = ComponentAccumulator()
    sub_detTools = []
    if flags.Detector.GeometryMDT:
        sub_detTools.append(result.popToolsAndMerge(MdtReadoutGeomToolCfg(flags)))
    kwargs.setdefault("ReadoutEleBuilders", sub_detTools)
    the_tool = CompFactory.MuonGMR4.MuonDetectorTool(name = name, **kwargs)
    result.setPrivateTools(the_tool)
    return result


