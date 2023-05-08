# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValActsCfg(flags, 
                   name: str = 'PhysValActs') -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.PhysValTool(name=name))
    return acc
