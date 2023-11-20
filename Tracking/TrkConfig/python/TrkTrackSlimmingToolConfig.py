# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkTrackSlimmingTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrackSlimmingToolCfg(flags, name="TrackSlimmingTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("KeepParameters", True)
    kwargs.setdefault("KeepOutliers", True)
    acc.setPrivateTools(CompFactory.Trk.TrackSlimmingTool(name, **kwargs))
    return acc

def GSFTrackSlimmingToolCfg(flags, name="GSFBuildInDetTrackSlimmingTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("KeepParameters", False)
    kwargs.setdefault("KeepOutliers", True)
    acc.setPrivateTools(CompFactory.Trk.TrackSlimmingTool(name, **kwargs))
    return acc

