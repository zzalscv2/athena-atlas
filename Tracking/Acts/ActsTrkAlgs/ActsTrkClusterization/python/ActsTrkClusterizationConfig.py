# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from ActsTrkClusterizationTools.ActsTrkClusterizationToolsConfig import ActsTrkITkPixelClusteringToolCfg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

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

