# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexSeedFinderTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrackDensitySeedFinderCfg(flags, name="TrackDensitySeedFinder", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.TrackDensitySeedFinder(name, **kwargs))
    return acc

def ZScanSeedFinderCfg(flags, name="ZScanSeedFinder", **kwargs):
    acc = ComponentAccumulator()

    if "IPEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            TrackToVertexIPEstimatorCfg)
        kwargs.setdefault("IPEstimator", acc.popToolsAndMerge(
            TrackToVertexIPEstimatorCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.ZScanSeedFinder(name, **kwargs))
    return acc
