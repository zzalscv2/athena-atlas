# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPrepRawDataFormation package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format

def ITkInDetToXAODClusterConversionCfg(flags, name="ITkInDetToXAODClusterConversion", **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.InDet.InDetToXAODClusterConversion(name, **kwargs))
    return acc

def ITkXAODToInDetClusterConversionCfg(flags, name="ITkXAODToInDetClusterConversion", **kwargs):
    acc = ComponentAccumulator()

    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )

    acc.addEventAlgo(CompFactory.InDet.XAODToInDetClusterConversion(name, **kwargs))
    return acc

def PixelClusterizationCfg(flags, name = "InDetPixelClusterization", **kwargs):
    acc = ComponentAccumulator()

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import MergedPixelsToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            MergedPixelsToolCfg(flags)))

    if "gangedAmbiguitiesFinder" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import PixelGangedAmbiguitiesFinderCfg
        kwargs.setdefault("gangedAmbiguitiesFinder", acc.popToolsAndMerge(
            PixelGangedAmbiguitiesFinderCfg(flags)))

    if not flags.Overlay.doTrackOverlay:
        kwargs.setdefault("DataObjectName", "PixelRDOs")
    else:
        #for track overlay, only run tracking on the HS RDOs
        kwargs.setdefault("DataObjectName", flags.Overlay.SigPrefix + "PixelRDOs")
    kwargs.setdefault("ClustersName", "PixelClusters")

    acc.addEventAlgo(CompFactory.InDet.PixelClusterization(name, **kwargs))
    return acc

def PixelClusterizationPUCfg(flags, name="InDetPixelClusterizationPU", **kwargs):
    kwargs.setdefault("DataObjectName", "Pixel_PU_RDOs")
    kwargs.setdefault("ClustersName", "PixelPUClusters")
    kwargs.setdefault("AmbiguitiesMap", "PixelClusterAmbiguitiesMapPU")
    return PixelClusterizationCfg(flags, name, **kwargs)

def TrigPixelClusterizationCfg(flags, name="InDetPixelClusterization", **kwargs):
    acc = ComponentAccumulator()
   
    if "RegSelTool" not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
        kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(
            regSelTool_Pixel_Cfg(flags)))

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import TrigMergedPixelsToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            TrigMergedPixelsToolCfg(flags)))

    if "gangedAmbiguitiesFinder" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import PixelGangedAmbiguitiesFinderCfg
        kwargs.setdefault("gangedAmbiguitiesFinder", acc.popToolsAndMerge(
            PixelGangedAmbiguitiesFinderCfg(flags)))

    kwargs.setdefault("AmbiguitiesMap", "TrigPixelClusterAmbiguitiesMap")
    kwargs.setdefault("ClustersName", "PixelTrigClusters")
    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("ClusterContainerCacheKey", "PixelTrigClustersCache")

    acc.addEventAlgo(CompFactory.InDet.PixelClusterization(name, **kwargs))
    return acc

def ITkPixelClusterizationCfg(flags, name = "ITkPixelClusterization", **kwargs):
    acc = ComponentAccumulator()

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkMergedPixelsToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            ITkMergedPixelsToolCfg(flags)))

    if "gangedAmbiguitiesFinder" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkPixelGangedAmbiguitiesFinderCfg
        kwargs.setdefault("gangedAmbiguitiesFinder", acc.popToolsAndMerge(ITkPixelGangedAmbiguitiesFinderCfg(flags)))

    kwargs.setdefault("DataObjectName", "ITkPixelRDOs")
    kwargs.setdefault("ClustersName", "ITkPixelClusters")
    kwargs.setdefault("AmbiguitiesMap", "ITkPixelClusterAmbiguitiesMap")

    acc.addEventAlgo(CompFactory.InDet.PixelClusterization(name, **kwargs))
    return acc

def ITkTrigPixelClusterizationCfg(flags, name = "ITkTrigPixelClusterization", roisKey="", signature="", **kwargs):
    acc = ComponentAccumulator()
    from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
    acc.merge(ITkPixelClusterizationCfg(flags,
                                        name="ITkPixelClusterization_"+signature,
                                        isRoI_Seeded=True,
                                        RoIs=roisKey,
                                        ClustersName = "ITkTrigPixelClusters",
                                        ClusterContainerCacheKey="PixelTrigClustersCache",
                                        RegSelTool= acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags))))
    return acc

def SCTClusterizationCfg(flags, name="InDetSCT_Clusterization", **kwargs):
    acc = ComponentAccumulator()

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(
            SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False)))

    if "SCTDetElStatus" not in kwargs :
        from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_DetectorElementStatusAlgWithoutFlaggedCfg
        acc.merge( SCT_DetectorElementStatusAlgWithoutFlaggedCfg(flags) )
        kwargs.setdefault("SCTDetElStatus", "SCTDetectorElementStatusWithoutFlagged" )

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import SCT_ClusteringToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            SCT_ClusteringToolCfg(flags)))

    if not flags.Overlay.doTrackOverlay:
        kwargs.setdefault("DataObjectName", 'SCT_RDOs')
    else:
        #for track overlay, only run tracking on the HS RDOs
        kwargs.setdefault("DataObjectName", flags.Overlay.SigPrefix + "SCT_RDOs")
    kwargs.setdefault("ClustersName", 'SCT_Clusters')

    acc.addEventAlgo(CompFactory.InDet.SCT_Clusterization(name, **kwargs))
    return acc

def SCTClusterizationPUCfg(flags, name="InDetSCT_ClusterizationPU", **kwargs):
    kwargs.setdefault("DataObjectName", "SCT_PU_RDOs" )
    kwargs.setdefault("ClustersName", "SCT_PU_Clusters")
    return SCTClusterizationCfg(flags, name, **kwargs)

def TrigSCTClusterizationCfg(flags, name="InDetSCT_Clusterization", roisKey="", signature="", **kwargs):
    acc = ComponentAccumulator()

    if "RegSelTool" not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
        kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(
            regSelTool_SCT_Cfg(flags)))

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool",  acc.popToolsAndMerge(
            SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False, withTdaqTool=False)))

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import Trig_SCT_ClusteringToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            Trig_SCT_ClusteringToolCfg(flags)))

    kwargs.setdefault("DataObjectName", 'SCT_RDOs')
    kwargs.setdefault("ClustersName", 'SCT_TrigClusters')
    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("ClusterContainerCacheKey", "SCT_ClustersCache")
    kwargs.setdefault("FlaggedCondCacheKey", "")

    acc.addEventAlgo(CompFactory.InDet.SCT_Clusterization(name+signature, **kwargs))
    return acc

def ITkStripClusterizationCfg(flags, name="ITkStripClusterization", **kwargs):
    acc = ComponentAccumulator()

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(
            ITkStripConditionsSummaryToolCfg(flags)))

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITKStrip_SCT_ClusteringToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            ITKStrip_SCT_ClusteringToolCfg(flags)))

    kwargs.setdefault("DataObjectName", 'ITkStripRDOs')
    kwargs.setdefault("ClustersName", 'ITkStripClusters')
    kwargs.setdefault("SCT_FlaggedCondData", "ITkStripFlaggedCondData")

    acc.addEventAlgo( CompFactory.InDet.SCT_Clusterization(name, **kwargs))
    return acc

def ITkTrigStripClusterizationCfg(flags, name="ITkTrigStripClusterization", roisKey="", signature="", **kwargs):
    acc = ComponentAccumulator()
    from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
    acc.merge(ITkStripClusterizationCfg(flags,
                                        name="ITkStripClusterization_"+signature,
                                        isRoI_Seeded=True,
                                        RoIs=roisKey,
                                        ClustersName = "ITkTrigStripClusters",
                                        ClusterContainerCacheKey="SCT_ClustersCache",
                                        RegSelTool= acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags))))
    return acc

def InDetTRT_RIO_MakerCfg(flags, name = "InDetTRT_RIO_Maker", **kwargs):
    acc = ComponentAccumulator()

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_DriftCircleToolCfg(flags)))

    if not flags.Overlay.doTrackOverlay:
        kwargs.setdefault("TRTRDOLocation", 'TRT_RDOs')
    else:
        #for track overlay, only run tracking on the HS RDOs
        kwargs.setdefault("TRTRDOLocation", flags.Overlay.SigPrefix + 'TRT_RDOs')
    kwargs.setdefault("TRTRIOLocation", 'TRT_DriftCircles')

    acc.addEventAlgo(CompFactory.InDet.TRT_RIO_Maker(name, **kwargs))
    return acc

def InDetTRT_NoTime_RIO_MakerCfg(flags, name = "InDetTRT_NoTime_RIO_Maker", **kwargs):
    acc = ComponentAccumulator()

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_NoTime_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_NoTime_DriftCircleToolCfg(flags)))

    kwargs.setdefault("TRTRIOLocation", 'TRT_DriftCirclesUncalibrated')

    acc.merge(InDetTRT_RIO_MakerCfg(flags, name, **kwargs))
    return acc

def InDetTRT_Phase_RIO_MakerCfg(flags, name = "InDetTRT_Phase_RIO_Maker", **kwargs):
    acc = ComponentAccumulator()

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_Phase_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_Phase_DriftCircleToolCfg(flags)))

    acc.merge(InDetTRT_RIO_MakerCfg(flags, name, **kwargs))
    return acc

def InDetTRT_RIO_MakerPUCfg(flags, name = "InDetTRT_RIO_MakerPU", **kwargs):
    kwargs.setdefault("TRTRDOLocation", 'TRT_PU_RDOs')    
    kwargs.setdefault("TRTRIOLocation", 'TRT_PU_DriftCircles')
    return InDetTRT_RIO_MakerCfg(flags, name, **kwargs)

def TrigTRTRIOMakerCfg(flags, name="InDetTrigMTTRTDriftCircleMaker", **kwargs):
    acc = ComponentAccumulator()

    if "RegSelTool" not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
        kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(
            regSelTool_TRT_Cfg(flags)))

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_DriftCircleToolCfg(flags)))

    kwargs.setdefault("TRTRIOLocation", "TRT_TrigDriftCircles")
    kwargs.setdefault("TRTRDOLocation", "TRT_RDOs_TRIG" if flags.Input.Format is Format.BS else "TRT_RDOs")
    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("RoIs", flags.Tracking.ActiveConfig.roi)
    
    kwargs.setdefault("TRT_DriftCircleCache", "TRT_DriftCircleCache")
    acc.addEventAlgo(CompFactory.InDet.TRT_RIO_Maker(
        name+"_"+flags.Tracking.ActiveConfig.input_name, **kwargs))
    return acc

def AthenaTrkClusterizationCfg(flags):
    acc = ComponentAccumulator()
    #
    # -- Pixel Clusterization
    #
    if flags.Detector.EnableITkPixel:
        acc.merge(ITkPixelClusterizationCfg(flags))
    #
    # --- Strip Clusterization
    #
    if flags.Detector.EnableITkStrip:
        acc.merge(ITkStripClusterizationCfg(flags))

    return acc
