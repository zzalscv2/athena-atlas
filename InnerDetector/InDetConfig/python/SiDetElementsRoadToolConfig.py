# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiDetElementsRoadTool_xk package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def SiDetElementsRoadMaker_xkCfg(flags, name="InDetSiRoadMaker", **kwargs) :
    acc = ComponentAccumulator()

    #
    # --- SCT and Pixel detector elements road builder
    #

    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc.merge(PixelReadoutGeometryCfg(flags)) # To produce PixelDetectorElementCollection
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags)) # To produce SCT_DetectorElementCollection

    # Create ReadCondHandle SiDetElementsLayerVectors_xk
    acc.addCondAlgo(CompFactory.InDet.SiDetElementsRoadCondAlg_xk(name = "SiDetElementsRoadCondAlg_xk"))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    InDetPatternPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags, name="InDetPatternPropagator"))
    acc.addPublicTool(InDetPatternPropagator)
    kwargs.setdefault("PropagatorTool", InDetPatternPropagator)

    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.usePixel )
    kwargs.setdefault("PixManagerLocation", 'Pixel')
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useSCT)
    kwargs.setdefault("SCTManagerLocation", 'SCT')
    kwargs.setdefault("RoadWidth", flags.Tracking.ActiveConfig.roadWidth)

    acc.setPrivateTools(CompFactory.InDet.SiDetElementsRoadMaker_xk(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc

def TrigSiDetElementsRoadMaker_xkCfg(flags, name="InDetTrigSiRoadMaker", **kwargs) :
    acc = ComponentAccumulator()
    
    if 'PropagatorTool' not in kwargs:
        from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
        InDetPatternPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags, 
                                                                              name="InDetTrigPatternPropagator"))
        acc.addPublicTool(InDetPatternPropagator)
        kwargs.setdefault("PropagatorTool", InDetPatternPropagator)

    acc.setPrivateTools(acc.popToolsAndMerge(SiDetElementsRoadMaker_xkCfg(flags, name, **kwargs)))
    return acc


def SiDetElementsRoadMaker_xk_TRT_Cfg(flags, name = 'InDetTRT_SeededSiRoad', **kwargs):
    #
    # Silicon det elements road maker tool
    #
    kwargs.setdefault("RoadWidth", 50. if flags.Beam.Type is BeamType.Cosmics else 35.)
    kwargs.setdefault("MaxStep", 20.)

    return SiDetElementsRoadMaker_xkCfg(flags, name, **kwargs)

def ITkSiDetElementsRoadMaker_xkCfg(flags, name="ITkSiRoadMaker", **kwargs) :
    acc = ComponentAccumulator()
    #
    # --- ITk Strip and Pixel detector elements road builder
    #

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags)) # To produce ITkPixelDetectorElementCollection
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags)) # To produce ITkStripDetectorElementCollection

    # Create ReadCondHandle SiDetElementsLayerVectors_xk
    acc.addCondAlgo(CompFactory.InDet.SiDetElementsRoadCondAlg_xk(name = "SiDetElementsRoadCondAlg_xk",
                                                                  PixelDetEleCollKey = "ITkPixelDetectorElementCollection",
                                                                  SCTDetEleCollKey = "ITkStripDetectorElementCollection"))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    ITkPatternPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags, name="ITkPatternPropagator"))
    acc.addPublicTool(ITkPatternPropagator)
    kwargs.setdefault("PropagatorTool", ITkPatternPropagator)

    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.useITkPixel )
    kwargs.setdefault("PixManagerLocation", 'ITkPixel')
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useITkStrip)
    kwargs.setdefault("SCTManagerLocation", 'ITkStrip')
    kwargs.setdefault("RoadWidth", flags.Tracking.ActiveConfig.roadWidth)

    acc.setPrivateTools(CompFactory.InDet.SiDetElementsRoadMaker_xk(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc

