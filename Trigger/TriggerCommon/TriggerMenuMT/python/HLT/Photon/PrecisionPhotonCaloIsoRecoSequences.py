#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)



def precisionPhotonCaloIsoRecoSequence(flags, RoIs, ion=False):
    """ With this function we will setup the sequence of Calo isolation to be executed after PrecisionPhoton in  TrigEgamma 

    """
    acc= ComponentAccumulator()

    log.debug('precisionPhotonCaloIsoRecoSequence(RoIs = %s)',RoIs)

    log.debug('retrieve(precisionPhotonCaloIsoRecoSequence,None,RoIs = %s)',RoIs)


    # Add CaloIsolationTool
    from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import TrigPhotonIsoBuilderCfg
    TrigPhotonIsoBuilder = TrigPhotonIsoBuilderCfg(flags,
                                                   ion =ion)

    acc.merge(TrigPhotonIsoBuilder)

    #online monitoring for topoEgammaBuilder
    from TriggerMenuMT.HLT.Photon.TrigPhotonFactoriesCfg import PrecisionPhotonCaloIsoMonitorCfg
    PrecisionPhotonCaloIsoRecoMonAlgo = PrecisionPhotonCaloIsoMonitorCfg(flags)
    acc.merge(PrecisionPhotonCaloIsoRecoMonAlgo)

    return acc





