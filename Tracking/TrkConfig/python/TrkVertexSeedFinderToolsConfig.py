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

def CrossDistancesSeedFinderCfg(flags, name="CrossDistancesSeedFinder", **kwargs):
    acc = ComponentAccumulator()

    if "TrkDistanceFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderUtilsConfig import (
            SeedNewtonTrkDistanceFinderCfg)
        kwargs.setdefault("TrkDistanceFinder", acc.popToolsAndMerge(
            SeedNewtonTrkDistanceFinderCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.CrossDistancesSeedFinder(name, **kwargs))
    return acc


def IndexedCrossDistancesSeedFinderCfg(flags, name='IndexedCrossDistancesSeedFinder', **kwargs):

  acc = ComponentAccumulator()
  if "Mode3dFinder" not in kwargs:
    from TrkConfig.TrkVertexSeedFinderUtilsConfig import Mode3dFromFsmw1dFinderCfg
    kwargs.setdefault("Mode3dFinder", acc.popToolsAndMerge(Mode3dFromFsmw1dFinderCfg(flags)))

  if "TrkDistanceFinder" not in kwargs:
    from TrkConfig.TrkVertexSeedFinderUtilsConfig import SeedNewtonTrkDistanceFinderCfg
    kwargs.setdefault("TrkDistanceFinder", acc.popToolsAndMerge(SeedNewtonTrkDistanceFinderCfg(flags)))

  kwargs.setdefault("trackdistcutoff", 0.01)
  kwargs.setdefault("maximumTracksNoCut", 30)
  kwargs.setdefault("maximumDistanceCut", 7.5)

  acc.setPrivateTools(CompFactory.Trk.IndexedCrossDistancesSeedFinder(name, **kwargs))
  return acc
