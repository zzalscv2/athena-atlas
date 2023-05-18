# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
from InDetConfig.SiClusterizationToolConfig import ITkClusterMakerToolCfg, ITkPixelRDOToolCfg
from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg

def ActsTrkITkPixelClusteringToolCfg(flags, name="ActsITkPixelClusteringTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(ITkPixelRDOToolCfg(flags)))
    kwargs.setdefault("ClusterMakerTool", acc.popToolsAndMerge(ITkClusterMakerToolCfg(flags)))
    kwargs.setdefault("AddCorners", True)
    kwargs.setdefault("ErrorStrategy", 1)
    kwargs.setdefault("PixelChargeCalibCondData", "ITkPixelChargeCalibCondData")
    kwargs.setdefault("PixelOfflineCalibData", "")
    acc.setPrivateTools(CompFactory.ActsTrk.PixelClusteringTool(name, **kwargs))
    return acc


def ActsTrkITkStripClusteringToolCfg(flags, name="ActsITkStripClusteringTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("StripConditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)))

    if flags.ITk.selectStripIntimeHits:
        from AthenaConfiguration.Enums import BeamType
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.ActsTrk.StripClusteringTool(name, **kwargs))
    return acc

def ActsTrkITkPixelClusterizationAlgCfg(flags, name='ActsTrkITkPixelClusterizationAlg', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDOContainerKey", "ITkPixelRDOs")
    kwargs.setdefault("PixelClustersKey", "ITkPixelClusters")
    kwargs.setdefault("PixelClusteringTool", acc.popToolsAndMerge(ActsTrkITkPixelClusteringToolCfg(flags)))

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkITkPixelClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkITkPixelClusterizationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterizationAlg(name, **kwargs))
    return acc

def ActsTrkITkStripClusterizationAlgCfg(flags, name='ActsTrkITkStripClusterizationAlg', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("StripRDOContainerKey", "ITkStripRDOs")
    kwargs.setdefault("StripClustersKey", "ITkStripClusters")
    kwargs.setdefault("StripDetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("StripClusteringTool", acc.popToolsAndMerge(ActsTrkITkStripClusteringToolCfg(flags)))
    kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))
    kwargs.setdefault("checkBadModules", True)

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkITkStripClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkITkStripClusterizationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterizationAlg(name, **kwargs))
    return acc

def ActsTrkClusterizationCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsTrkITkPixelClusterizationAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsTrkITkStripClusterizationAlgCfg(flags))

    if flags.Acts.doAnalysis:
        from ActsConfig.ActsTrkAnalysisConfig import ActsTrkClusterAnalysisCfg
        acc.merge(ActsTrkClusterAnalysisCfg(flags))

    return acc
