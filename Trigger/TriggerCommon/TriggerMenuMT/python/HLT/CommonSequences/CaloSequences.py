#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Config.MenuComponents import RecoFragmentsPool, algorithmCAToGlobalWrapper
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from AthenaCommon.CFElements import parOR
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from .FullScanDefs import caloFSRoI
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

class CaloMenuDefs(object):
      """Static Class to collect all string manipulations in Calo sequences """
      from TrigEDMConfig.TriggerEDMRun3 import recordable
      L2CaloClusters= recordable("HLT_FastCaloEMClusters")


#
# central or forward fast calo sequence 
#

@AccumulatorCache
def fastCaloMenuSequenceCfg(flags, name, doRinger=True, is_probe_leg=False):
    """ Creates Egamma Fast Calo  MENU sequence
    The Hypo name changes depending on name, so for different implementations (Electron, Gamma,....)
    """

    from TrigT2CaloCommon.CaloDef import fastCaloVDVCfg
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    from TrigT2CaloCommon.CaloDef import fastCaloRecoSequenceCfg
    nameselAcc = "fastCaloSequence"
    output = "HLT_FastCaloEMClusters"
    selAcc = SelectionCA(nameselAcc,isProbe=is_probe_leg)
    InViewRoIs="EMCaloRoIs"
    reco = InViewRecoCA("EMCalo",InViewRoIs=InViewRoIs,isProbe=is_probe_leg)
    reco.mergeReco(fastCaloVDVCfg(InViewRoIs=InViewRoIs))
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix=InViewRoIs+'_probe' if is_probe_leg else InViewRoIs)
    reco.mergeReco(fastCaloRecoSequenceCfg(flags, inputEDM=InViewRoIs,ClustersName=output))
    selAcc.mergeReco(reco, robPrefetchCA=robPrefetchAlg)

    # hypo # The Alg will ALWAYS configure photons and electrons for ringer
    # The tool is what will use that or not
    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import createTrigEgammaFastCaloHypoAlg

    theFastCaloHypo = createTrigEgammaFastCaloHypoAlg(flags, name, sequenceOut=output)
    selAcc.addHypoAlgo(theFastCaloHypo)

    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import TrigEgammaFastCaloHypoToolFromDict
    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigEgammaFastCaloHypoToolFromDict,isProbe=is_probe_leg)
    
def fastCaloMenuSequence(flags, name, doRinger=True, is_probe_leg=False):
     if isComponentAccumulatorCfg():
        return fastCaloMenuSequenceCfg(flags,name=name,doRinger=doRinger,is_probe_leg=is_probe_leg)
     else: 
        return menuSequenceCAToGlobalWrapper(fastCaloMenuSequenceCfg,flags,name=name,doRinger=doRinger,is_probe_leg=is_probe_leg)

def cellRecoSequence(flags, name="HLTCaloCellMakerFS", RoIs=caloFSRoI, outputName="CaloCellsFS", monitorCells = False):
    from TrigCaloRec.TrigCaloRecConfig import hltCaloCellMakerCfg
    alg = algorithmCAToGlobalWrapper(hltCaloCellMakerCfg, flags=flags, name=name, roisKey=RoIs, CellsName=outputName, monitorCells=monitorCells)
    return alg,outputName

def caloClusterRecoSequence(
        flags, name="HLTCaloClusterMakerFS", RoIs=caloFSRoI,
        outputName="HLT_TopoCaloClustersFS"):
        from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg
    
        alg=algorithmCAToGlobalWrapper(jetmetTopoClusteringCfg,
                                       flags	   =flags,
                                       RoIs        =RoIs
                                       )        
        return alg, outputName

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
