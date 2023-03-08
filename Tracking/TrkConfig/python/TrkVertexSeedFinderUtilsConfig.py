# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexSeedFinderUtils package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SeedNewtonTrkDistanceFinderCfg(flags, name='SeedNewtonTrkDistanceFinder', **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.Trk.SeedNewtonTrkDistanceFinder(name, **kwargs))
    return acc


def Mode3dFromFsmw1dFinderCfg(flags, name='Mode3dFromFsmw1dFinder', **kwargs):

  acc = ComponentAccumulator()
  
  kwargs["MinimalDistanceFromZtoXY"] = 0.25

  acc.setPrivateTools(CompFactory.Trk.Mode3dFromFsmw1dFinder(name, **kwargs))
  return acc
