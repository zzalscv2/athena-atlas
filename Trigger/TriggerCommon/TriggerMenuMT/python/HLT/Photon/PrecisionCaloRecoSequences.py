#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging    import logging
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys      import getTrigEgammaKeys
from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import TrigEgammaRecCfg, TrigEgammaSuperClusterBuilderCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

log = logging.getLogger(__name__)


def precisionCaloRecoSequence(flags, RoIs, name = None, ion=False):

    acc = ComponentAccumulator()

    TrigEgammaKeys = getTrigEgammaKeys(ion = ion)

    log.debug('flags = %s',flags)
    log.debug('RoIs = %s',RoIs)

    from TrigCaloRec.TrigCaloRecConfig import hltCaloTopoClusteringCfg, hltCaloTopoClusteringHICfg
    
    if ion:
        topoCluster = hltCaloTopoClusteringHICfg(flags,
                                                 namePrefix='',
                                                 CellsName = "CaloCells",
                                                 roisKey=RoIs)
    else:
        topoCluster = hltCaloTopoClusteringCfg(flags,
                                               namePrefix='',
                                               nameSuffix='RoI',
                                               CellsName = "CaloCells",
                                               roisKey=RoIs) 
    acc.merge(topoCluster)
    tag = 'HI' if ion is True else '' 
    
    copier = CompFactory.egammaTopoClusterCopier('gTrigEgammaTopoClusterCopier'+  tag + RoIs,
                                                 InputTopoCollection=TrigEgammaKeys.precisionTopoClusterContainer,
                                                 OutputTopoCollection= TrigEgammaKeys.precisionCaloTopoCollection,
                                                 OutputTopoCollectionShallow='tmp_'+TrigEgammaKeys.precisionCaloTopoCollection)
    acc.addEventAlgo(copier)

    trigEgammaRec = TrigEgammaRecCfg(flags, name = 'gTrigEgammaRec'+tag + RoIs)
        
    acc.merge(trigEgammaRec)

    trigEgammaSuperClusterBuilder = TrigEgammaSuperClusterBuilderCfg(flags,
                                                                     'gTrigEgammaSuperClusterBuilder' + tag + RoIs,
                                                                     'photon', 
                                                                     TrigEgammaKeys.precisionPhotonCaloClusterContainer,
                                                                     TrigEgammaKeys.precisionPhotonSuperClusterCollection)
    acc.merge(trigEgammaSuperClusterBuilder)

    return acc



