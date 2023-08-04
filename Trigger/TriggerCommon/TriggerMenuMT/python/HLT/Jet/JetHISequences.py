# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import parOR, seqAND
from ..Config.MenuComponents import MenuSequence, RecoFragmentsPool
from .JetMenuSequencesConfig import JetHypoAlgType, getCaloInputMaker
from .JetHIConfig import jetHIClusterSequence, jetHIRecoSequence
from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetHypoToolFromDict

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

# This sets up the reconstruction starting from calo towers for heavy ion events.
def jetHICaloRecoSequences( flags, RoIs, **jetRecoDict ):
    if jetRecoDict["ionopt"] == "noion":
        raise ValueError("Heavy-ion calorimeter jet reco called without an ion option!")

    # Get the tower reconstruction sequence 
    clusterSequence, clustersKey, towerKey = RecoFragmentsPool.retrieve(
        jetHIClusterSequence, flags, ionopt=jetRecoDict["ionopt"], RoIs=RoIs)

    # Get the jet reconstruction sequence including the jet definition and output collection
    jetRecoSeq, jetsOut, jetDef  = RecoFragmentsPool.retrieve(
        jetHIRecoSequence, flags, clustersKey=clustersKey, towerKey=towerKey, **jetRecoDict )

    return [clusterSequence,jetRecoSeq], jetsOut, jetDef, clustersKey


def makeMenuSequence(flags,jetSeq,IMAlg,jetsIn,jetDefString,hypoType=JetHypoAlgType.STANDARD):
    def trigStreamerHypoTool(chain_dict):
        return CompFactory.TrigStreamerHypoTool(chain_dict["chainName"])

    hyponame = "TrigJetHypoAlg_"+jetDefString
    trigHypoToolGen = trigJetHypoToolFromDict
    if hypoType is JetHypoAlgType.PASSTHROUGH:
        hyponame = "TrigStreamerHypoAlg_passthrough"
        hypo = CompFactory.TrigStreamerHypoAlg(hyponame)
        trigHypoToolGen = trigStreamerHypoTool
    else:
        hypo = CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn)

    log.debug("Generating jet menu sequence for hypo %s",hyponame)

    return  MenuSequence( flags,
                          Sequence    = jetSeq,
                          Maker       = IMAlg,
                          Hypo        = hypo,
                          HypoToolGen = trigHypoToolGen )


# Passing isPerf = True disables the hypo
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
def jetHICaloHypoMenuSequence(flags, isPerf, **jetRecoDict):
    InputMakerAlg = getCaloInputMaker()
    jetRecoSequences, jetsIn, jetDef, clustersKey = RecoFragmentsPool.retrieve(
        jetHICaloRecoSequences,
        flags, RoIs='', **jetRecoDict)

    strtemp = "HI_{recoAlg}_{jetCalib}"
    jetDefString = strtemp.format(**jetRecoDict)
    jetAthRecoSeq = parOR(f"jetSeqHICaloHypo_{jetDefString}_RecoSequence", jetRecoSequences)
    log.debug("Generating jet HI calo hypo menu sequence for reco %s",jetDefString)

    menuseq_suffix = ''
    hypoType = JetHypoAlgType.STANDARD
    if isPerf:
        hypoType = JetHypoAlgType.PASSTHROUGH
        menuseq_suffix = '_passthrough'
    jetAthMenuSeq = seqAND(f"jetSeqHICaloHypo_{jetDefString}_MenuSequence{menuseq_suffix}",[InputMakerAlg,jetAthRecoSeq])
    return makeMenuSequence(flags,jetAthMenuSeq,InputMakerAlg,jetsIn,jetDefString,hypoType), jetDef
