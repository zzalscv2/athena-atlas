# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkTruthAlgs package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrackTruthSimilaritySelectorCfg(flags, name='Selector', **kwargs) :
    acc = ComponentAccumulator()

    if "TrackTruthSimilarityTool" not in kwargs:
        from TrkConfig.TrkTruthCreatorToolsConfig import InDetTruthMatchToolCfg
        kwargs.setdefault("TrackTruthSimilarityTool", acc.popToolsAndMerge(
            InDetTruthMatchToolCfg(flags)))

    acc.addEventAlgo(CompFactory.TrackTruthSimilaritySelector(kwargs["OutputName"]+name, **kwargs))
    return acc

def ITkTrackTruthSimilaritySelectorCfg(flags, name='Selector', **kwargs) :
    acc = ComponentAccumulator()

    if "TrackTruthSimilarityTool" not in kwargs:
        from TrkConfig.TrkTruthCreatorToolsConfig import ITkTruthMatchToolCfg
        kwargs.setdefault("TrackTruthSimilarityTool", acc.popToolsAndMerge(
            ITkTruthMatchToolCfg(flags)))

    acc.addEventAlgo(CompFactory.TrackTruthSimilaritySelector(kwargs["OutputName"]+name, **kwargs))
    return acc

def TrackTruthSelectorCfg(flags, name="Selector", tracks="", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("DetailedTrackTruthName", tracks+"DetailedTruth")
    kwargs.setdefault("OutputName", tracks+"Truth")
    acc.addEventAlgo(CompFactory.TrackTruthSelector(tracks+name, **kwargs))
    return acc

def TrackParticleTruthAlgCfg(flags, name="TruthAlg", tracks="", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackTruthName", tracks+"Truth")
    acc.addEventAlgo(CompFactory.TrackParticleTruthAlg(tracks+name, **kwargs))
    return acc
