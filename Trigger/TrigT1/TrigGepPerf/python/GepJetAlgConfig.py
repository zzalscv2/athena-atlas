# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GepJetAlgCfg(
        flags,
        name, 
        jetAlgName,
        caloClustersKey,
        outputJetsKey,
        OutputLevel=None):
    
    cfg = ComponentAccumulator()


    assert jetAlgName in ('ModAntikT', 'Cone')
    alg = CompFactory.GepJetAlg(name,
                                jetAlgName=jetAlgName,
                                caloClustersKey=caloClustersKey,
                                outputJetsKey=outputJetsKey)
    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        
    cfg.addEventAlgo(alg)

    return cfg
    
                     
