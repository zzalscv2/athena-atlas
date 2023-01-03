# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkTrackSlimmer package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrackSlimmerCfg(flags, name="TrackSlimmer", **kwargs):
    acc = ComponentAccumulator()

    if "TrackSlimmingTool" not in kwargs:
        from TrkConfig.TrkTrackSlimmingToolConfig import TrackSlimmingToolCfg
        TrackSlimmingTool = acc.popToolsAndMerge(TrackSlimmingToolCfg(flags))
        acc.addPublicTool(TrackSlimmingTool)
        kwargs.setdefault("TrackSlimmingTool", TrackSlimmingTool)

    acc.addEventAlgo(CompFactory.Trk.TrackSlimmer(name, **kwargs))
    return acc
