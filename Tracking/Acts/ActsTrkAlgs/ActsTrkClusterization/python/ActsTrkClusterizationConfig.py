# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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
        from ActsTrkAnalysis.ActsTrkLiveMonitoringConfig import ActsTrkITkPixelClusterizationLiveMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkITkPixelClusterizationLiveMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterizationAlg(name, **kwargs))
    return acc

def ActsTrkITkStripClusterizationAlgCfg(flags, name='ActsTrkItkStripClusterizationAlg', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("StripRDOContainerKey", "ITkStripRDOs")
    kwargs.setdefault("StripClustersKey", "ITkStripClusters")
    kwargs.setdefault("StripDetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("StripClusteringTool", acc.popToolsAndMerge(ActsTrkITkStripClusteringToolCfg(flags)))
    kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))
    kwargs.setdefault("checkBadModules", True)
    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterizationAlg(name, **kwargs))

    if flags.Acts.doMonitoring:
        from ActsTrkAnalysis.ActsTrkLiveMonitoringConfig import ActsTrkITkStripClusterizationLiveMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkITkStripClusterizationLiveMonitoringToolCfg(flags)))

    return acc
