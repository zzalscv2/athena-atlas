# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_StandaloneTrackFinder package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_StandaloneTrackFinderCfg(flags, name ='InDetTRT_StandaloneTrackFinder', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MinNumDriftCircles",
                      flags.Tracking.ActiveConfig.minTRTonly)
    kwargs.setdefault("MinPt", flags.Tracking.ActiveConfig.minTRTonlyPt)
    kwargs.setdefault("MaterialEffects", 0)
    kwargs.setdefault("OldTransitionLogic",
                      flags.Tracking.ActiveConfig.useTRTonlyOldLogic)
    kwargs.setdefault("OutputTracksLocation", "TRTStandaloneTracks")

    if "TRT_SegToTrackTool" not in kwargs:
        from InDetConfig.TRT_SegmentsToTrackToolConfig import (
            TRT_Standalone_SegmentToTrackToolCfg)
        kwargs.setdefault("TRT_SegToTrackTool", acc.popToolsAndMerge(
            TRT_Standalone_SegmentToTrackToolCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TRT_StandaloneTrackFinder(name, **kwargs))
    return acc

def TRT_TrackSegment_TrackFinderCfg(flags, name ='InDetTRT_TrackSegment_TrackFinder',
                                    **kwargs):
    acc = ComponentAccumulator()

    if "TRT_SegToTrackTool" not in kwargs:
        from InDetConfig.TRT_SegmentsToTrackToolConfig import (
            TRT_TrackSegment_SegmentToTrackToolCfg)
        kwargs.setdefault("TRT_SegToTrackTool", acc.popToolsAndMerge(
            TRT_TrackSegment_SegmentToTrackToolCfg(flags)))

    kwargs.setdefault("OutputTracksLocation", "StandaloneTRTTracks")

    acc.merge(TRT_StandaloneTrackFinderCfg(flags, name, **kwargs))
    return acc
