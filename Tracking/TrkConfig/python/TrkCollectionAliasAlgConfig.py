# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkCollectionAliasAlg package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CopyAlgForAmbiCfg(flags, name="CopyAlgForAmbi", **kwargs) :
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.Trk.TrkCollectionAliasAlg(name, **kwargs))
    return acc
