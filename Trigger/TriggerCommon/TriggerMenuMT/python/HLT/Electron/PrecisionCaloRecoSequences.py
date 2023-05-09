#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging    import logging
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys      import getTrigEgammaKeys
from TriggerMenuMT.HLT.Egamma.TrigEgammaFactoriesCfg import TrigEgammaRecCfg, TrigEgammaSuperClusterBuilderCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


log = logging.getLogger(__name__)

# Dummy flag arg needed so that each reco sequence is held separately
# in the RecoFragmentsPool -- only the RoIs are used to distinguish
# different sequences. New convention is just to pass "None" for flags
# taken from Jet/JetRecoSequences.py
def precisionCaloRecoSequence(flags, RoIs, name = None, ion=False, variant=''):
    acc = ComponentAccumulator()

    TrigEgammaKeys = getTrigEgammaKeys(variant, ion = ion)

    log.debug('flags = %s',flags)
    log.debug('RoIs = %s',RoIs)

    from TrigCaloRec.TrigCaloRecConfig import egammaTopoClusteringCfg, egammaTopoClusteringCfg_LRT, hltCaloTopoClusteringHICfg

    if ion:
        topoCluster = hltCaloTopoClusteringHICfg(flags,
                                                 CellsName = "CaloCells",
                                                 roisKey=RoIs)
    else:
        if variant:
            topoCluster = egammaTopoClusteringCfg_LRT(flags, RoIs)

        else:
            topoCluster = egammaTopoClusteringCfg(flags, RoIs)
 
    acc.merge(topoCluster)
    tag = 'HI' if ion is True else '' 
    
    copier = CompFactory.egammaTopoClusterCopier('eTrigEgammaTopoClusterCopier'+  tag + RoIs,
                                                 InputTopoCollection=TrigEgammaKeys.precisionTopoClusterContainer,
                                                 OutputTopoCollection= TrigEgammaKeys.precisionCaloTopoCollection,
                                                 OutputTopoCollectionShallow='tmp_'+TrigEgammaKeys.precisionCaloTopoCollection)
    acc.addEventAlgo(copier)

    trigEgammaRec = TrigEgammaRecCfg(flags, name = 'eTrigEgammaRec'+tag + RoIs +variant)
        
    acc.merge(trigEgammaRec)

    trigEgammaSuperClusterBuilder = TrigEgammaSuperClusterBuilderCfg(flags,
                                                                     'eTrigEgammaSuperClusterBuilder' + tag + RoIs,
                                                                     'electron', 
                                                                     TrigEgammaKeys.precisionElectronCaloClusterContainer,
                                                                     TrigEgammaKeys.precisionEgammaSuperClusterRecCollection)
    acc.merge(trigEgammaSuperClusterBuilder)

    return acc
