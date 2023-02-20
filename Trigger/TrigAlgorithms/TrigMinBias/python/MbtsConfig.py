# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TrigMinBias.TrigMinBiasMonitoring import MbtsFexMonitoring
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def MbtsFexCfg(flags, name="MbtsFex", MbtsBitsKey=None):
    """Configures MBTS Fex with monitoring"""
    acc = ComponentAccumulator()
    from TrigT2CaloCommon.TrigCaloDataAccessConfig import CaloDataAccessSvcDependencies

    alg = CompFactory.MbtsFex(name,
        MbtsBitsKey=MbtsBitsKey,
        MonTool = MbtsFexMonitoring(flags), 
        ExtraInputs = CaloDataAccessSvcDependencies)
    acc.addEventAlgo(alg, primary=True)
    return acc
    
def MbtsSGInputCfg(flags):
    """Configures SG Input needed for MBTS Fex"""
    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    return SGInputLoaderCfg(flags, [('TileTBID','DetectorStore+TileTBID' )])