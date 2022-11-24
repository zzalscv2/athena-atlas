# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkTruthCreatorTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetTruthMatchToolCfg(flags, name='InDetTruthMatchTool', **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("WeightPixel", 10.)
    kwargs.setdefault("WeightSCT", 5.)
    kwargs.setdefault("WeightTRT", 1.)

    acc.setPrivateTools(CompFactory.Trk.TruthMatchRatio(name, **kwargs))
    return acc

def ITkTruthMatchToolCfg(flags, name='ITkTruthMatchTool', **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("WeightPixel", 10.)
    kwargs.setdefault("WeightSCT", 5.)

    acc.setPrivateTools(CompFactory.Trk.TruthMatchRatio(name, **kwargs))
    return acc

def TruthToTrackToolCfg(flags, name="TruthToTrack", **kwargs):
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.TruthToTrack(name, **kwargs))
    return acc

