#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Config.MenuComponents import RecoFragmentsPool, MenuSequence, algorithmCAToGlobalWrapper
from AthenaCommon.CFElements import seqAND, parOR
from .FullScanDefs import caloFSRoI

class CaloMenuDefs(object):
      """Static Class to collect all string manipulations in Calo sequences """
      from TrigEDMConfig.TriggerEDMRun3 import recordable
      L2CaloClusters= recordable("HLT_FastCaloEMClusters")


#
# central or forward fast calo sequence 
#
def fastCaloSequence(flags, name="fastCaloSequence"):
    """ Creates Fast Calo reco sequence"""
    
    from TrigT2CaloCommon.CaloDef import fastCaloEVCreator
    from TrigT2CaloCommon.CaloDef import fastCaloRecoSequence
    (fastCaloViewsMaker, InViewRoIs) = fastCaloEVCreator()
    # reco sequence always build the rings
    (fastCaloInViewSequence, sequenceOut) = fastCaloRecoSequence(flags, InViewRoIs)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Calo, flags, nameSuffix=fastCaloViewsMaker.name())[0]

     # connect EVC and reco
    fastCaloSequence = seqAND(name, [fastCaloViewsMaker, robPrefetchAlg, fastCaloInViewSequence ])
    return (fastCaloSequence, fastCaloViewsMaker, sequenceOut)



def fastCaloMenuSequence(flags, name, doRinger=True, is_probe_leg=False):
    """ Creates Egamma Fast Calo  MENU sequence
    The Hypo name changes depending on name, so for different implementations (Electron, Gamma,....)
    """
    (sequence, fastCaloViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(fastCaloSequence, flags=flags)
    # check if use Ringer and are electron because there aren't ringer for photons yet:

    # hypo
    if doRinger:
      from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import createTrigEgammaFastCaloHypoAlg
    else:
      from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import createTrigEgammaFastCaloHypoAlg_noringer as createTrigEgammaFastCaloHypoAlg

    theFastCaloHypo = createTrigEgammaFastCaloHypoAlg(flags, name+"EgammaFastCaloHypo", sequenceOut)
    CaloMenuDefs.L2CaloClusters = sequenceOut

    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import TrigEgammaFastCaloHypoToolFromDict
    return MenuSequence( flags,
                         Sequence    = sequence,
                         Maker       = fastCaloViewsMaker,
                         Hypo        = theFastCaloHypo,
                         HypoToolGen = TrigEgammaFastCaloHypoToolFromDict,
                         IsProbe     = is_probe_leg )



def cellRecoSequence(flags, name="HLTCaloCellMakerFS", RoIs=caloFSRoI, outputName="CaloCellsFS"):
    """ Produce the full scan cell collection """
    if not RoIs:
        from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
        RoIs = mapThresholdToL1RoICollection("FSNOSEED")
    from TrigCaloRec.TrigCaloRecConfig import HLTCaloCellMaker
    alg = HLTCaloCellMaker(flags, name, roisKey = RoIs, CellsName=outputName, monitorCells=False)
    return parOR(name+"RecoSequence", [alg]), str(alg.CellsName)

def caloClusterRecoSequence(
        flags, name="HLTCaloClusterMakerFS", RoIs=caloFSRoI,
        outputName="HLT_TopoCaloClustersFS"):
    """ Create the EM-level fullscan clusters """
    cell_sequence, cells_name = RecoFragmentsPool.retrieve(cellRecoSequence, flags=flags, RoIs=RoIs)
    from TrigCaloRec.TrigCaloRecConfig import hltTopoClusterMakerCfg
    alg = algorithmCAToGlobalWrapper(hltTopoClusterMakerCfg, flags, name,
                                     doLC=False,
                                     clustersKey=outputName,
                                     cellsKey=cells_name)[0]
    return parOR(name+"RecoSequence", [cell_sequence, alg]), str(alg.CaloClusters)

def LCCaloClusterRecoSequence(
        flags, name="HLTCaloClusterCalibratorLCFS", RoIs=caloFSRoI,
        outputName="HLT_TopoCaloClustersLCFS"):
    """ Create the LC calibrated fullscan clusters

    The clusters will be created as a shallow copy of the EM level clusters
    """
    em_sequence, em_clusters = RecoFragmentsPool.retrieve(caloClusterRecoSequence, flags=flags, RoIs=RoIs)
    from TrigCaloRec.TrigCaloRecConfig import hltCaloTopoClusterCalibratorCfg
    alg = algorithmCAToGlobalWrapper(hltCaloTopoClusterCalibratorCfg,
                                     flags, name,
                                     clustersin = em_clusters,
                                     clustersout = outputName,
                                     OutputCellLinks = outputName+"_cellLinks")[0]

    return parOR(name+"RecoSequence", [em_sequence, alg]), str(alg.OutputClusters)

def caloTowerHIRecoSequence(
        flags, name="HLTHICaloTowerMakerFS", RoIs=caloFSRoI,
        outputName="HLT_HICaloTowerFS"):
    """ Create the EM-level fullscan clusters for heavy-ion"""
    cell_sequence, cells_name = RecoFragmentsPool.retrieve(cellRecoSequence, flags=flags, RoIs=RoIs)
    from TrigCaloRec.TrigCaloRecConfig import hltHICaloTowerMakerCfg
    alg = algorithmCAToGlobalWrapper(hltHICaloTowerMakerCfg,
                                     flags, name,
                                     towersKey = outputName,
                                     cellsKey = cells_name,
                                     RoIs=RoIs)[0]
    return parOR(name+"RecoSequence", [cell_sequence, alg]), str(alg.CaloTowers), str(cells_name)
