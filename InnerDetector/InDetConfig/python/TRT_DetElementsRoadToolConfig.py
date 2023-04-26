# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_DetElementsRoadTool_xk package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_DetElementsRoadCondAlgCfg(flags, name="TRT_DetElementsRoadCondAlg_xk", **kwargs):
    acc = ComponentAccumulator()
    acc.addCondAlgo(
        CompFactory.InDet.TRT_DetElementsRoadCondAlg_xk(name, **kwargs))
    return acc

def TRT_DetElementsRoadMaker_xkCfg(flags, name='TRT_DetElementsRoadMaker', **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags)
    acc.merge(TRT_DetElementsRoadCondAlgCfg(flags)) # To produce the input TRT_DetElementsRoadData_xk CondHandle

    if "PropagatorTool" not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import (
            RungeKuttaPropagatorCfg)
        kwargs.setdefault("PropagatorTool", acc.popToolsAndMerge(
            RungeKuttaPropagatorCfg(flags)))

    acc.setPrivateTools(
        CompFactory.InDet.TRT_DetElementsRoadMaker_xk(name, **kwargs))
    return acc

def TRT_DetElementsRoadMaker_xk_TRTExtensionCfg(flags, name='TRT_DetElementsRoadMaker_TRTExtension', **kwargs):
    kwargs.setdefault("RoadWidth", 20.)
    return TRT_DetElementsRoadMaker_xkCfg(flags, name, **kwargs)

def Trig_TRT_DetElementsRoadMaker_xk_TRTExtensionCfg(flags, name='TRT_DetElementsRoadMaker_TRTExtension', **kwargs):
    #this should not be necessary aside from 2022 legacy config
    kwargs.setdefault("RoadWidth", 10.)                 #2023fix 
    return TRT_DetElementsRoadMaker_xkCfg(flags, name, **kwargs)
