# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsTrkFindingTool.ActsTrkFindingToolConfig import ActsTrkFindingToolCfg

# ACTS only algorithm
def ActsTrkFindingCfg(flags, name: str = "ActsTrkFindingAlg", **kwargs):
    acc = ComponentAccumulator()

    # tools
    if "TrackFindingTool" not in kwargs:
        kwargs["TrackFindingTool"] = acc.popToolsAndMerge(ActsTrkFindingToolCfg(flags))

    kwargs.setdefault("PixelClusterContainerKey", "ITkPixelClusters")
    kwargs.setdefault("StripClusterContainerKey", "ITkStripClusters")
    kwargs.setdefault("PixelDetectorElements", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("StripDetectorElements", "ITkStripDetectorElementCollection")
    kwargs.setdefault("InputEstimatedTrackParameters", "ITkPixelEstimatedTrackParams")
    kwargs.setdefault("TracksLocation", "SiSPSeededActsTracks")

    ## The following is a placeholder for when we implement ActsTrkFindingLiveMonitoringCfg.
    # if flags.Acts.doMonitoring:
    #     from ActsTrkAnalysis.ActsTrkLiveMonitoringConfig import ActsTrkFindingLiveMonitoringCfg
    #     kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkFindingLiveMonitoringCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc
