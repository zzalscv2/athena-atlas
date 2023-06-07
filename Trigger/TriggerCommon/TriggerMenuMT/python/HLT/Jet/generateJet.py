# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, ChainStep, Chain, InEventRecoCA, SelectionCA
from AthenaConfiguration.ComponentFactory import CompFactory
import pprint
from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg, HICaloTowerCfg
from AthenaCommon.Logging import logging
from ..CommonSequences.FullScanDefs import trkFSRoI, em_clusters, caloFSRoI
from .JetRecoCommon import jetRecoDictToString
log = logging.getLogger(__name__)

def generateChains( flags, chainDict ):
    from .JetRecoCommon import extractRecoDict, jetChainParts

    jetRecoDict = extractRecoDict(jetChainParts(chainDict["chainParts"]))
    jetDefStr = jetRecoDictToString(jetRecoDict)

    stepName = f"MainStep_jet_{jetDefStr}"

    acc = SelectionCA(stepName)
#    acc.addSequence(stepView)

    from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
    InEventReco = InEventRecoCA(f"Jet_{jetDefStr}Reco",inputMaker=clusterFSInputMaker())

    # HI jet reconstruction sequence is starting
    if jetRecoDict["ionopt"] == "noion":
        clustersname = em_clusters

        InEventReco.mergeReco(jetmetTopoClusteringCfg(flags,RoIs ='')) 
                                                       
        if jetRecoDict["trkopt"] != "notrk":
            from .JetTrackingConfig import JetFSTrackingCfg
            trk_acc, trkcolls = JetFSTrackingCfg(flags, jetRecoDict["trkopt"], trkFSRoI)
            InEventReco.mergeReco(trk_acc)
        else:
            trkcolls = None

        from .JetRecoSequencesConfig import JetRecoCfg
        jet_acc, jetsOut, jetDef = JetRecoCfg(flags, clustersKey=clustersname, trkcolls=trkcolls, **jetRecoDict)
        InEventReco.mergeReco(jet_acc)
    else:
        clustersname = caloFSRoI 
        InEventReco.mergeReco(HICaloTowerCfg(flags))

        from .JetHIConfig import JetHICfg
        jet_acc, jetsOut, jetDef = JetHICfg(flags, clustersKey=clustersname, **jetRecoDict)
        InEventReco.mergeReco(jet_acc)
        
    acc.mergeReco(InEventReco)
    #hypo
    from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetHypoToolFromDict
    hypo = CompFactory.TrigJetHypoAlg(f"TrigJetHypoAlg_{jetDefStr}", Jets=jetDef.fullname())
    acc.addHypoAlgo(hypo)

    jetSequence = MenuSequenceCA(flags, acc,
                                 HypoToolGen = trigJetHypoToolFromDict)

    jetStep = ChainStep(name=stepName, Sequences=[jetSequence], chainDicts=[chainDict])
    
    l1Thresholds=[]
    for part in chainDict['chainParts']:
        l1Thresholds.append(part['L1threshold'])

    log.debug('dictionary is: %s\n', pprint.pformat(chainDict))

    acc.printConfig()

    chain = Chain( chainDict['chainName'], L1Thresholds=l1Thresholds, ChainSteps=[jetStep] )

    return chain
