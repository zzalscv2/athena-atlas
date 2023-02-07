# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TBDetDescrLoaderCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("TBElementContainer", "TBElementCnt")
    kwargs.setdefault("TBDetDescrManager", "TBDetDescrMgr")
    # do not read anything from StoreGate
    kwargs.setdefault("ReadAction", 0)
    # and write TBDetDescrContainer to StoreGate every event
    kwargs.setdefault("WriteAction", 2)
    kwargs.setdefault("OutputLevel", 5)
    result.addEventAlgo(CompFactory.TBDetDescrLoader(**kwargs))
    return result
