# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ActsTrkClusterizationTools.ActsTrkClusterizationToolsConfig import ActsTrkITkPixelClusteringToolCfg, ActsTrkITkStripClusteringToolCfg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg

def ActsTrkITkPixelClusterizationAlgCfg(flags, name='ActsTrkITkPixelClusterizationAlg', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDOContainerKey", "ITkPixelRDOs")
    kwargs.setdefault("PixelClustersKey", "ITkPixelClusters")
    kwargs.setdefault("PixelClusteringTool", acc.popToolsAndMerge(ActsTrkITkPixelClusteringToolCfg(flags)))

    if flags.Acts.doMonitoring:
        from ActsTrkAnalysis.ActsTrkMonitoringConfig import ActsTrkITkPixelClusterizationMonitoringToolCfg
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
        from ActsTrkAnalysis.ActsTrkMonitoringConfig import ActsTrkITkStripClusterizationMonitoringToolCfg
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
        from ActsTrkAnalysis.ActsTrkAnalysisConfig import ActsTrkClusterAnalysisCfg
        acc.merge(ActsTrkClusterAnalysisCfg(flags))

    return acc
