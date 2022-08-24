# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkTruthAlgs package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrackTruthSimilaritySelectorCfg(flags, DetailedTruth, TracksTruth, name='Selector', **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("DetailedTrackTruthName", DetailedTruth)
    kwargs.setdefault("OutputName", TracksTruth)

    if "TrackTruthSimilarityTool" not in kwargs:
        from TrkConfig.TrkTruthCreatorToolsConfig import InDetTruthMatchToolCfg
        kwargs.setdefault("TrackTruthSimilarityTool", acc.popToolsAndMerge(
            InDetTruthMatchToolCfg(flags)))

    acc.addEventAlgo(CompFactory.TrackTruthSimilaritySelector(name = TracksTruth+name, **kwargs))
    return acc

def ITkTrackTruthSimilaritySelectorCfg(flags, DetailedTruth, TracksTruth, name='Selector', **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("DetailedTrackTruthName", DetailedTruth)
    kwargs.setdefault("OutputName", TracksTruth)

    if "TrackTruthSimilarityTool" not in kwargs:
        from TrkConfig.TrkTruthCreatorToolsConfig import ITkTruthMatchToolCfg
        kwargs.setdefault("TrackTruthSimilarityTool", acc.popToolsAndMerge(
            ITkTruthMatchToolCfg(flags)))

    acc.addEventAlgo(CompFactory.TrackTruthSimilaritySelector(name = TracksTruth+name, **kwargs))
    return acc
