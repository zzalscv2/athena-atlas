# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSystematicsTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetTrackTruthOriginToolCfg(flags, name="InDetTrackTruthOriginTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("isFullPileUpTruth", flags.Digitization.PileUp \
                      and flags.Digitization.DigiSteeringConf in ['StandardPileUpToolsAlg', \
                                                                  'StandardInTimeOnlyTruthPileUpToolsAlg', \
                                                                  'StandardInTimeOnlyGeantinoTruthPileUpToolsAlg'])
    acc.setPrivateTools(CompFactory.InDet.InDetTrackTruthOriginTool(name, **kwargs))
    return acc

def InDetTrackTruthFilterToolCfg(flags, name="InDetTrackTruthFilterTool", **kwargs):
    acc = ComponentAccumulator()

    if "trackOriginTool" not in kwargs:
        kwargs.setdefault("trackOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.InDetTrackTruthFilterTool(name, **kwargs))
    return acc

def JetTrackFilterToolCfg(flags, name="JetTrackFilterTool", **kwargs):
    acc = ComponentAccumulator()

    if "trackOriginTool" not in kwargs:
        kwargs.setdefault("trackOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.JetTrackFilterTool(name, **kwargs))
    return acc

