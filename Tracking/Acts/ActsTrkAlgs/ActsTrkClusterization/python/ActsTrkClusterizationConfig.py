# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from ActsTrkClusterizationTools.ActsTrkClusterizationToolsConfig import ActsTrkITkPixelClusteringToolCfg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTrkITkPixelClusterizationAlgCfg(flags, name='ActsTrkITkPixelClusterizationAlgorithm', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDOContainerKey", "ITkPixelRDOs")
    kwargs.setdefault("PixelClustersKey", "ITkPixelClusters")
    kwargs.setdefault("PixelClusteringTool", acc.popToolsAndMerge(ActsTrkITkPixelClusteringToolCfg(flags)))
    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterizationAlgorithm(name, **kwargs))
    return acc
