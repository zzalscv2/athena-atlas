# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkTrackCollectionMerger package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrackCollectionMergerAlgCfg(flags, name="InDetTrackCollectionMerger",
                                InputCombinedTracks=None,
                                OutputCombinedTracks="",
                                **kwargs):
    result = ComponentAccumulator()
    doTrackOverlay = getattr(flags.TrackOverlay, "ActiveConfig.doTrackOverlay", None) or flags.Overlay.doTrackOverlay
    prefix = 'Sig_' if doTrackOverlay else ''

    kwargs.setdefault("TracksLocation", InputCombinedTracks)
    kwargs.setdefault("OutputTracksLocation", OutputCombinedTracks)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import InDetPRDtoTrackMapToolGangedPixelsCfg
        kwargs.setdefault("AssociationTool", result.popToolsAndMerge(InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("DoTrackOverlay",doTrackOverlay)

    result.addEventAlgo(CompFactory.Trk.TrackCollectionMerger(prefix+name, **kwargs))
    return result


def ITkTrackCollectionMergerAlgCfg(flags, name="ITkTrackCollectionMerger",
                                   InputCombinedTracks=None,
                                   OutputCombinedTracks="CombinedITkTracks",
                                   **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("TracksLocation", InputCombinedTracks)
    kwargs.setdefault("OutputTracksLocation", OutputCombinedTracks)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import ITkPRDtoTrackMapToolGangedPixelsCfg
        kwargs.setdefault("AssociationTool", result.popToolsAndMerge(ITkPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("AssociationMapName", "ITkPRDToTrackMapCombinedITkTracks")

    result.addEventAlgo(CompFactory.Trk.TrackCollectionMerger(name, **kwargs))
    return result
