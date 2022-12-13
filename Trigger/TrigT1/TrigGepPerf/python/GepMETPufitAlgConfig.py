# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepMETPufitAlgCfg(
        flags,
        name,
        caloClustersKey,
        outputMETPufitKey,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()

    alg = CompFactory.GepMETPufitAlg(name,
                                     caloClustersKey=caloClustersKey,
                                     outputMETPufitKey=outputMETPufitKey)
    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        
    cfg.addEventAlgo(alg)

    return cfg
    
                     
