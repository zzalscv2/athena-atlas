# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsTrkFindingTool.ActsTrkFindingToolConfig import ActsTrkFindingToolCfg
from ActsGeometry.ActsGeometryConfig import ActsATLASConverterToolCfg

# ACTS only algorithm
def ActsTrkFindingCfg(flags, name: str = "ActsTrkFindingAlg", **kwargs):
    acc = ComponentAccumulator()

    # tools
    # check if options have already the seed tool defined
    # and make sure it is not a None
    trackFindingTool = None
    if "TrackFindingTool" not in kwargs:
        trackFindingTool = acc.popToolsAndMerge(ActsTrkFindingToolCfg(flags))

    ActsATLASConverterTool = None
    if "ATLASConverterTool" not in kwargs:
        ActsATLASConverterTool = acc.popToolsAndMerge(ActsATLASConverterToolCfg(flags))

    kwargs.setdefault("PixelClusterContainerKey", "ITkPixelClusters")
    kwargs.setdefault("StripClusterContainerKey", "ITkStripClusters")
    kwargs.setdefault("PixelDetectorElements", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("StripDetectorElements", "ITkStripDetectorElementCollection")
    kwargs.setdefault("InputEstimatedTrackParameters", "ITkPixelEstimatedTrackParams")
    kwargs.setdefault("TracksLocation", "SiSPSeededActsTracks")
    kwargs.setdefault("TrackFindingTool", trackFindingTool)
    kwargs.setdefault("ATLASConverterTool", ActsATLASConverterTool)

    ## The following is a placeholder for when we implement ActsTrkFindingLiveMonitoringCfg.
    # if flags.Acts.doMonitoring:
    #     from ActsTrkAnalysis.ActsTrkLiveMonitoringConfig import ActsTrkFindingLiveMonitoringCfg
    #     kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkFindingLiveMonitoringCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc
