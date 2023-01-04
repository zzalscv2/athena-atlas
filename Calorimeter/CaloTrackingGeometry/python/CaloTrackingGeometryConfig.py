# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of CaloTrackingGeometry package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def CaloDepthEntranceCfg(flags, name="CaloDepthTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CaloDepthTool(name, **kwargs))
    return acc

def CaloSurfaceBuilderEntranceCfg(flags, name="CaloSurfaceBuilderEntrance", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CaloDepthTool", acc.popToolsAndMerge(
        CaloDepthEntranceCfg(flags, name = "CaloDepthToolEntrance",
                             DepthChoice = "entrance")))
    acc.setPrivateTools( CompFactory.CaloSurfaceBuilder(name, **kwargs) )
    return acc

def CaloSurfaceBuilderMiddleCfg(flags, name="CaloSurfaceBuilderMiddle", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CaloDepthTool", acc.popToolsAndMerge(
        CaloDepthEntranceCfg(flags, name = "CaloDepthToolMiddle",
                             DepthChoice = "middle")))
    acc.setPrivateTools( CompFactory.CaloSurfaceBuilder(name, **kwargs) )
    return acc
