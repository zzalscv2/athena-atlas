#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)



def precisionPhotonRecoSequence(flags, RoIs, name = None, ion=False):
    """ With this function we will setup the sequence of offline EgammaAlgorithms so to make a photon for TrigEgamma 

    Sequence of algorithms is the following:
      - egammaRecBuilder/TrigEgammaRecPhoton creates egammaObjects out of clusters and tracks. Here, at HLT photons we will only use clusters. 
      - photonSuperClusterBuilder algorithm will create superclusters out of the toposlusters and tracks in egammaRec under the photon hypothesis
          https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/python/egammaSuperClusterBuilder.py#L26 
      - TopoEgammBuilder will create photons and electrons out of trakcs and SuperClusters. Here at HLT photons the aim is to ignore electrons and not use tracks at all.
          https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/src/topoEgammaBuilder.cxx
    """

    log.debug('precisionPhotonRecoSequence(RoIs = %s)',RoIs)

    acc= ComponentAccumulator()
    
    # Retrieve the factories now
    from TriggerMenuMT.HLT.Photon.TrigPhotonFactoriesCfg import TrigTopoEgammaPhotonCfg, TrigTopoEgammaPhotonCfg_HI

    log.debug('retrieve(precisionPhotonRecoSequence,None,RoIs = %s)',RoIs)
    
    if ion:
        TrigTopoEgammaPhoton = TrigTopoEgammaPhotonCfg_HI(flags)
    else:
        TrigTopoEgammaPhoton = TrigTopoEgammaPhotonCfg(flags)

    acc.merge(TrigTopoEgammaPhoton)

    #online monitoring for topoEgammaBuilder
    from TriggerMenuMT.HLT.Photon.TrigPhotonFactoriesCfg import PrecisionPhotonTopoMonitorCfg
    PrecisionPhotonTopoRecoMonAlgo = PrecisionPhotonTopoMonitorCfg(flags,ion=ion)
    acc.merge(PrecisionPhotonTopoRecoMonAlgo)

    #online monitoring for TrigPhotonSuperClusterBuilder
    from TriggerMenuMT.HLT.Photon.TrigPhotonFactoriesCfg import PrecisionPhotonSuperClusterMonitorCfg
    PrecisionPhotonSuperClusterMonAlgo = PrecisionPhotonSuperClusterMonitorCfg(flags, ion=ion)
    acc.merge(PrecisionPhotonSuperClusterMonAlgo)

    return acc

