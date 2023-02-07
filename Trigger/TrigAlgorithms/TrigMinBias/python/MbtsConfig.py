# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TrigMinBias.TrigMinBiasConf import MbtsFex
from TrigMinBias.TrigMinBiasMonitoring import MbtsFexMonitoring


def MbtsFexCfg(flags, name="MbtsFex", **kwargs):
    alg = MbtsFex(name, **kwargs)
    alg.MonTool =  MbtsFexMonitoring(flags)
    from TrigT2CaloCommon.TrigCaloDataAccessConfig import CaloDataAccessSvcDependencies
    alg.ExtraInputs = CaloDataAccessSvcDependencies

    # Tell SGInputLoader to get TileTBID from the DetectorStore
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()
    if hasattr(topSequence,"SGInputLoader"):
        topSequence.SGInputLoader.Load += [('TileTBID','DetectorStore+TileTBID')]

    return alg


