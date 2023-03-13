# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiCombinatorialTrackFinderTool_xk package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SiDetElementBoundaryLinksCondAlg_xk_Pixel_Cfg(flags, name = "InDetSiDetElementBoundaryLinksPixelCondAlg", **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc = PixelReadoutGeometryCfg(flags) # To produce PixelDetectorElementCollection

    kwargs.setdefault("ReadKey", "PixelDetectorElementCollection")
    kwargs.setdefault("WriteKey", "PixelDetElementBoundaryLinks_xk")

    acc.addCondAlgo(CompFactory.InDet.SiDetElementBoundaryLinksCondAlg_xk(name, **kwargs))
    return acc

def SiDetElementBoundaryLinksCondAlg_xk_SCT_Cfg(flags, name = "InDetSiDetElementBoundaryLinksSCTCondAlg", **kwargs):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags) # To produce SCT_DetectorElementCollection

    kwargs.setdefault("ReadKey", "SCT_DetectorElementCollection")
    kwargs.setdefault("WriteKey", "SCT_DetElementBoundaryLinks_xk")

    acc.addCondAlgo(CompFactory.InDet.SiDetElementBoundaryLinksCondAlg_xk(name, **kwargs))
    return acc

def SiDetElementBoundaryLinksCondAlg_xk_ITkPixel_Cfg(flags, name = "ITkSiDetElementBoundaryLinksPixelCondAlg", **kwargs):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc = ITkPixelReadoutGeometryCfg(flags) # To produce ITkPixelDetectorElementCollection

    kwargs.setdefault("ReadKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("WriteKey", "ITkPixelDetElementBoundaryLinks_xk")
    kwargs.setdefault("ITkGeometry", True)

    acc.addCondAlgo(CompFactory.InDet.SiDetElementBoundaryLinksCondAlg_xk(name, **kwargs))
    return acc

def SiDetElementBoundaryLinksCondAlg_xk_ITkStrip_Cfg(flags, name = "ITkSiDetElementBoundaryLinksStripCondAlg", **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc = ITkStripReadoutGeometryCfg(flags) # To produce ITkStripDetectorElementCollection

    kwargs.setdefault("ReadKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("WriteKey", "ITkStripDetElementBoundaryLinks_xk")
    kwargs.setdefault("ITkGeometry", True)

    acc.addCondAlgo(CompFactory.InDet.SiDetElementBoundaryLinksCondAlg_xk(name, **kwargs))
    return acc

def SiCombinatorialTrackFinder_xkCfg(flags, name="InDetSiComTrackFinder", **kwargs) :
    acc = ComponentAccumulator()

    # For SiDetElementBoundaryLinks_xk ReadCondHandle
    if flags.InDet.Tracking.ActiveConfig.usePixel:
        acc.merge(SiDetElementBoundaryLinksCondAlg_xk_Pixel_Cfg(flags))

    if flags.InDet.Tracking.ActiveConfig.useSCT:
        acc.merge(SiDetElementBoundaryLinksCondAlg_xk_SCT_Cfg(flags))

    kwargs.setdefault("usePixel", flags.InDet.Tracking.ActiveConfig.usePixel)
    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActiveConfig.useSCT)
    kwargs.setdefault("PixelClusterContainer", "PixelClusters")
    kwargs.setdefault("SCT_ClusterContainer", "SCT_Clusters")

    #
    # --- Local track finding using sdCaloSeededSSSpace point seed
    #
    from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorDigitalCfg
    RotCreator = acc.popToolsAndMerge(InDetRotCreatorDigitalCfg(flags))
    acc.addPublicTool(RotCreator)
    kwargs.setdefault("RIOonTrackTool", RotCreator)

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    InDetPatternPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags, name="InDetPatternPropagator"))
    acc.addPublicTool(InDetPatternPropagator)
    kwargs.setdefault("PropagatorTool", InDetPatternPropagator)

    from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdator_xkCfg
    InDetPatternUpdator = acc.popToolsAndMerge(KalmanUpdator_xkCfg(flags, name="InDetPatternUpdator"))
    acc.addPublicTool(InDetPatternUpdator)
    kwargs.setdefault("UpdatorTool", InDetPatternUpdator)

    from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
    kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags)))

    if flags.InDet.Tracking.ActiveConfig.usePixel:
        if "PixelSummaryTool" not in kwargs:
           from PixelConditionsTools.PixelConditionsSummaryConfig import PixelConditionsSummaryCfg
           kwargs.setdefault("PixelSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))
        if "PixelDetElStatus" not in kwargs :
            from PixelConditionsAlgorithms.PixelConditionsConfig import PixelDetectorElementStatusAlgCfg
            acc.merge( PixelDetectorElementStatusAlgCfg(flags) )
            kwargs.setdefault("PixelDetElStatus", "PixelDetectorElementStatus")
    else:
        kwargs.setdefault("PixelSummaryTool", "")

    if flags.InDet.Tracking.ActiveConfig.useSCT:
        if "SctSummaryTool" not in kwargs:
           from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
           kwargs.setdefault("SctSummaryTool", acc.popToolsAndMerge(SCT_ConditionsSummaryToolCfg(flags)))
        if "SCTDetElStatus" not in kwargs :
            from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig  import SCT_DetectorElementStatusAlgCfg
            acc.merge( SCT_DetectorElementStatusAlgCfg(flags) )
            kwargs.setdefault("SCTDetElStatus", "SCTDetectorElementStatus" )
    else:
        kwargs.setdefault("SctSummaryTool", "")

    track_finder = CompFactory.InDet.SiCombinatorialTrackFinder_xk(name = name+flags.InDet.Tracking.ActiveConfig.extension, **kwargs)
    acc.setPrivateTools(track_finder)
    return acc

def SiCombinatorialTrackFinder_xk_Trig_Cfg( flags, name="InDetTrigSiComTrackFinder", **kwargs ):
  """
  based  on: InnerDetector/InDetExample/InDetTrigRecExample/python/InDetTrigConfigRecLoadTools.py
  """
  acc = ComponentAccumulator()

  # For SiDetElementBoundaryLinks_xk ReadCondHandle
  if flags.InDet.Tracking.ActiveConfig.usePixel:
      acc.merge(SiDetElementBoundaryLinksCondAlg_xk_Pixel_Cfg(flags))

  if flags.InDet.Tracking.ActiveConfig.useSCT:
      acc.merge(SiDetElementBoundaryLinksCondAlg_xk_SCT_Cfg(flags))

  kwargs.setdefault("usePixel", flags.InDet.Tracking.ActiveConfig.usePixel)
  kwargs.setdefault("useSCT", flags.InDet.Tracking.ActiveConfig.useSCT)
  kwargs.setdefault("PixelClusterContainer", 'PixelTrigClusters')
  kwargs.setdefault("SCT_ClusterContainer", 'SCT_TrigClusters')
  kwargs.setdefault("PixelDetElStatus", "")
  kwargs.setdefault("SCTDetElStatus", "" )

  from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
  propagatorTool = acc.popToolsAndMerge( RungeKuttaPropagatorCfg( flags, name="InDetTrigPatternPropagator" ) )
  acc.addPublicTool(propagatorTool)
  kwargs.setdefault("PropagatorTool", propagatorTool)

  from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdator_xkCfg
  patternUpdatorTool = acc.popToolsAndMerge( KalmanUpdator_xkCfg(flags, name="InDetTrigPatternUpdator") )
  acc.addPublicTool(patternUpdatorTool)
  kwargs.setdefault("UpdatorTool", patternUpdatorTool)

  from TrkConfig.TrkRIO_OnTrackCreatorConfig import TrigRotCreatorCfg
  rioOnTrackTool = acc.popToolsAndMerge( TrigRotCreatorCfg( flags ) )
  acc.addPublicTool(rioOnTrackTool)
  kwargs.setdefault("RIOonTrackTool", rioOnTrackTool)

  from PixelConditionsTools.PixelConditionsSummaryConfig import PixelConditionsSummaryCfg
  kwargs.setdefault("PixelSummaryTool", acc.popToolsAndMerge(
      PixelConditionsSummaryCfg(flags)))

  from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
  kwargs.setdefault("SctSummaryTool", acc.popToolsAndMerge(
      SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False, withTdaqTool=False)))

  from InDetConfig.InDetBoundaryCheckToolConfig import InDetTrigBoundaryCheckToolCfg
  kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(
      InDetTrigBoundaryCheckToolCfg(flags)))

  acc.setPrivateTools(CompFactory.InDet.SiCombinatorialTrackFinder_xk(name, **kwargs))
  return acc

def ITkSiCombinatorialTrackFinder_xkCfg(flags, name="ITkSiComTrackFinder", **kwargs) :
    acc = ComponentAccumulator()

    # For SiDetElementBoundaryLinks_xk ReadCondHandle
    if flags.ITk.Tracking.ActiveConfig.useITkPixel:
        acc.merge(SiDetElementBoundaryLinksCondAlg_xk_ITkPixel_Cfg(flags))

    if flags.ITk.Tracking.ActiveConfig.useITkStrip:
        acc.merge(SiDetElementBoundaryLinksCondAlg_xk_ITkStrip_Cfg(flags))

    kwargs.setdefault("usePixel", flags.ITk.Tracking.ActiveConfig.useITkPixel)
    kwargs.setdefault("useSCT",   flags.ITk.Tracking.ActiveConfig.useITkStrip)
    kwargs.setdefault("PixelClusterContainer", 'ITkPixelClusters')
    kwargs.setdefault("SCT_ClusterContainer",   'ITkStripClusters')
    kwargs.setdefault("PixelDetElementBoundaryLinks_xk", "ITkPixelDetElementBoundaryLinks_xk")
    kwargs.setdefault("SCT_DetElementBoundaryLinks_xk",  "ITkStripDetElementBoundaryLinks_xk")
    kwargs.setdefault("ITkGeometry", True)
    kwargs.setdefault("doFastTracking", flags.ITk.Tracking.doFastTracking)

    #
    # --- Local track finding using sdCaloSeededSSSpace point seed
    #
    from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
    ITkRotCreator = acc.popToolsAndMerge(ITkRotCreatorCfg(flags, name="ITkRotCreator"+flags.ITk.Tracking.ActiveConfig.extension))
    acc.addPublicTool(ITkRotCreator)
    kwargs.setdefault("RIOonTrackTool", ITkRotCreator)

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    ITkPatternPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags, name="ITkPatternPropagator"))
    acc.addPublicTool(ITkPatternPropagator)
    kwargs.setdefault("PropagatorTool", ITkPatternPropagator)

    from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdator_xkCfg
    ITkPatternUpdator = acc.popToolsAndMerge(KalmanUpdator_xkCfg(flags, name="ITkPatternUpdator"))
    acc.addPublicTool(ITkPatternUpdator)
    kwargs.setdefault("UpdatorTool", ITkPatternUpdator)

    from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg
    kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(
        ITkBoundaryCheckToolCfg(flags)))

    if flags.Detector.EnableITkPixel:
        from PixelConditionsTools.ITkPixelConditionsSummaryConfig import ITkPixelConditionsSummaryCfg
        kwargs.setdefault("PixelSummaryTool", acc.popToolsAndMerge(
            ITkPixelConditionsSummaryCfg(flags)))
    else:
        kwargs.setdefault("PixelSummaryTool", None)

    if flags.Detector.EnableITkStrip:
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
        kwargs.setdefault("SctSummaryTool", acc.popToolsAndMerge(
            ITkStripConditionsSummaryToolCfg(flags)))
    else:
        kwargs.setdefault("SctSummaryTool", None)

    acc.setPrivateTools(CompFactory.InDet.SiCombinatorialTrackFinder_xk(name = name+flags.ITk.Tracking.ActiveConfig.extension, **kwargs))
    return acc
