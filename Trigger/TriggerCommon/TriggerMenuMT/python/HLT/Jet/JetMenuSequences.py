#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

from enum import Enum
from TriggerMenuMT.HLT.Config.MenuComponents import RecoFragmentsPool, MenuSequence
from AthenaCommon.CFElements import seqAND, parOR
from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
from AthenaConfiguration.ComponentFactory import CompFactory
from ..CommonSequences.FullScanDefs import  trkFSRoI
from TrigEDMConfig.TriggerEDMRun3 import recordable
from .JetRecoCommon import jetRecoDictToString
from .JetRecoSequences import jetClusterSequence, jetCaloRecoSequences, jetTrackingRecoSequences, jetHICaloRecoSequences
from .JetTrackingConfig import JetRoITrackingSequence

# Hypo tool generators
from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetHypoToolFromDict
from .JetPresel import caloPreselJetHypoToolFromDict

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorJetSuperROITool

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

###############################################################################################
### --- Input Makers ----

# For step 1, starting from the basic calo reco and topoclustering
# Used for calo-only chains and preselection for tracking
def getCaloInputMaker():
    from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
    InputMakerAlg = conf2toConfigurable(clusterFSInputMaker())
    return InputMakerAlg

# For later steps, where calo reco should not be run
# The same instance of an algorithm cannot be run in different steps
# Used for chains that use tracking
def getTrackingInputMaker(trkopt):
    if trkopt=="ftf":
        InputMakerAlg = conf2toConfigurable(CompFactory.InputMakerForRoI(
            "IM_Jet_TrackingStep",
            mergeUsingFeature = False,
            RoITool = conf2toConfigurable(CompFactory.ViewCreatorInitialROITool()),
            RoIs = trkFSRoI))
    elif trkopt=="roiftf":
        IDTrigConfig = getInDetTrigConfig( 'jetSuper' )
        InputMakerAlg = EventViewCreatorAlgorithm(
            "IMJetRoIFTF",
            mergeUsingFeature = False,
            RoITool = ViewCreatorJetSuperROITool(
                'ViewCreatorJetSuperRoI',
                RoisWriteHandleKey  = recordable( IDTrigConfig.roi ),
                RoIEtaWidth = IDTrigConfig.etaHalfWidth,
                RoIPhiWidth = IDTrigConfig.phiHalfWidth,
            ),
            Views = "JetSuperRoIViews",
            InViewRoIs = "InViewRoIs",
            RequireParentView = False,
            ViewFallThrough = True)
    else:
        raise RuntimeError("Unrecognised trkopt '%s' provided, choices are ['ftf','roiftf']",trkopt)
    return InputMakerAlg

###############################################################################################
### --- Menu Sequence helpers ---

# Functions defining the MenuSequence that will be placed into ChainSteps
# Generate a menu sequence given a set of jet sequences to schedule.
# The hypo may be set up as a preselection hypo, in which case it will
# record a single DecisionObject, instead of one per jet.
# A hypo may alternatively be configured to passThrough, such that
# the hypo will not retrieve any jets and simply pass.

class JetHypoAlgType(Enum):
    STANDARD = 0
    PRESEL = 1
    PASSTHROUGH = 2

def makeMenuSequence(jetSeq,IMAlg,jetsIn,jetDefString,hypoType=JetHypoAlgType.STANDARD):
    def trigStreamerHypoTool(chain_dict):
        return conf2toConfigurable(CompFactory.TrigStreamerHypoTool(chain_dict["chainName"]))

    hyponame = "TrigJetHypoAlg_"+jetDefString
    trigHypoToolGen = trigJetHypoToolFromDict
    if hypoType==JetHypoAlgType.PASSTHROUGH:
        hyponame = "TrigStreamerHypoAlg_caloReco"
        hypo = conf2toConfigurable(CompFactory.TrigStreamerHypoAlg(hyponame))
        trigHypoToolGen = trigStreamerHypoTool
    elif hypoType==JetHypoAlgType.PRESEL:
        hyponame += "_presel"
        hypo = conf2toConfigurable(CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn, DoPresel=True))
        trigHypoToolGen = caloPreselJetHypoToolFromDict
    else:
        hypo = conf2toConfigurable(CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn))

    log.debug("Generating jet menu sequence for hypo %s",hyponame)

    return  MenuSequence(   Sequence    = jetSeq,
                            Maker       = IMAlg,
                            Hypo        = hypo,
                            HypoToolGen = trigHypoToolGen )


###############################################################################################
### --- Menu Sequence getters ---

# For the preselection step before running tracking (step 1)
# We set RoIs='' (recognised as seedless) instead of caloFSRoI (output of caloInputMater()) to
# cut data dependency to InputMaker and allow full scan CaloCell+Clustering to be
# shared with EGamma (ATR-24722)
def jetCaloPreselMenuSequence(configFlags, **jetRecoDict):
    InputMakerAlg = getCaloInputMaker()
    jetRecoSequences, jetsIn, jetDef, clustersKey = RecoFragmentsPool.retrieve(
        jetCaloRecoSequences,
        configFlags, RoIs='', **jetRecoDict)

    jetDefString = jetRecoDictToString(jetRecoDict)
    jetAthRecoSeq = parOR(f"jetSeqCaloPresel_{jetDefString}_RecoSequence", jetRecoSequences)
    log.debug("Generating jet preselection menu sequence for reco %s",jetDefString)
    jetAthMenuSeq = seqAND(f"jetSeqCaloPresel_{jetDefString}_MenuSequence",[InputMakerAlg,jetAthRecoSeq])

    return makeMenuSequence(jetAthMenuSeq,InputMakerAlg,jetsIn,jetDefString,
                            hypoType=JetHypoAlgType.PRESEL), jetDef, clustersKey

# A null preselection, which will only run the cluster making (step 1)
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
def jetCaloRecoMenuSequence(configFlags, clusterCalib):
    InputMakerAlg = getCaloInputMaker()
    jetsIn = ""
    # get calo reco sequence: topoClusterSequence is a parOR of cell and topocluster reco algorithms.
    topoClusterSequence, clustersKey = RecoFragmentsPool.retrieve(
        jetClusterSequence, configFlags, RoIs='', clusterCalib=clusterCalib)

    jetAthMenuSeq = seqAND(f"jetSeqCaloRecoPassThrough_{clusterCalib}_MenuSequence",[InputMakerAlg,topoClusterSequence])

    log.debug("Generating jet calocluster reco sequence")
    return makeMenuSequence(jetAthMenuSeq,InputMakerAlg,jetsIn,"caloreco",
                            hypoType=JetHypoAlgType.PASSTHROUGH), clustersKey

# A full hypo selecting only on calo jets (step 1)
# Passing isPerf = True disables the hypo
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
def jetCaloHypoMenuSequence(configFlags, isPerf, **jetRecoDict):
    InputMakerAlg = getCaloInputMaker()
    jetRecoSequences, jetsIn, jetDef, clustersKey = RecoFragmentsPool.retrieve(
        jetCaloRecoSequences,
        configFlags, RoIs='', **jetRecoDict)
    jetDefString = jetRecoDictToString(jetRecoDict)
    jetAthRecoSeq = parOR(f"jetSeqCaloHypo_{jetDefString}_RecoSequence", jetRecoSequences)
    log.debug("Generating jet calo hypo menu sequence for reco %s",jetDefString)
    jetAthMenuSeq = seqAND(f"jetSeqCaloHypo_{jetDefString}_MenuSequence",[InputMakerAlg,jetAthRecoSeq])

    hypoType = JetHypoAlgType.STANDARD
    if isPerf: hypoType = JetHypoAlgType.PASSTHROUGH
    return makeMenuSequence(jetAthMenuSeq,InputMakerAlg,jetsIn,jetDefString,hypoType) ,jetDef

# A full hypo selecting only on heavy ion calo jets (step 1)
# Passing isPerf = True disables the hypo
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
def jetHICaloHypoMenuSequence(configFlags, isPerf, **jetRecoDict):
    InputMakerAlg = getCaloInputMaker()
    jetRecoSequences, jetsIn, jetDef, clustersKey = RecoFragmentsPool.retrieve(
        jetHICaloRecoSequences,
        configFlags, RoIs='', **jetRecoDict)

    strtemp = "HI_{recoAlg}_{jetCalib}"
    jetDefString = strtemp.format(**jetRecoDict)
    jetAthRecoSeq = parOR(f"jetSeqHICaloHypo_{jetDefString}_RecoSequence", jetRecoSequences)
    log.debug("Generating jet HI calo hypo menu sequence for reco %s",jetDefString)
    jetAthMenuSeq = seqAND(f"jetSeqHICaloHypo_{jetDefString}_MenuSequence",[InputMakerAlg,jetAthRecoSeq])

    hypoType = JetHypoAlgType.STANDARD
    if isPerf: hypoType = JetHypoAlgType.PASSTHROUGH
    return makeMenuSequence(jetAthMenuSeq,InputMakerAlg,jetsIn,jetDefString,hypoType), jetDef

# A full hypo selecting on jets with FS track reco (step 2)
# To combine either with a presel or a passthrough sequence
# As this does not run topoclustering, the cluster collection
# name needs to be passed in
def jetFSTrackingHypoMenuSequence(configFlags, clustersKey, isPerf, **jetRecoDict):
    InputMakerAlg = getTrackingInputMaker(jetRecoDict['trkopt'])
    jetRecoSequences, jetsIn, jetDef = RecoFragmentsPool.retrieve(
        jetTrackingRecoSequences,
        configFlags, RoIs=trkFSRoI, clustersKey=clustersKey, **jetRecoDict)

    jetDefString = jetRecoDictToString(jetRecoDict)
    jetAthRecoSeq = parOR(f"jetFSTrackingHypo_{jetDefString}_RecoSequence", jetRecoSequences)
    log.debug("Generating jet tracking hypo menu sequence for reco %s",jetDefString)
    jetAthMenuSeq = seqAND(f"jetFSTrackingHypo_{jetDefString}_MenuSequence",[InputMakerAlg,jetAthRecoSeq])

    hypoType = JetHypoAlgType.STANDARD
    if isPerf: hypoType = JetHypoAlgType.PASSTHROUGH
    return makeMenuSequence(jetAthMenuSeq,InputMakerAlg,jetsIn,jetDefString,hypoType), jetDef

# A full hypo selecting on jets with RoI track reco (step 2)
# Needs to be preceded by a presel sequence, and be provided
# with the input jets from which to define RoIs
# Presel jets to be reused, which makes ghost association impossible
# Substitute DR association decorator

def jetRoITrackingHypoMenuSequence(configFlags, jetsIn, **jetRecoDict):
    InputMakerAlg = getTrackingInputMaker(jetRecoDict['trkopt'])

    # Get the track reconstruction sequence: jetTrkSeq is parOR of all needed track + f-tag reco algorithms
    jetTrkSeq = RecoFragmentsPool.retrieve(
        JetRoITrackingSequence, configFlags, jetsIn=jetsIn,trkopt=jetRecoDict["trkopt"], RoIs=InputMakerAlg.InViewRoIs)

    InputMakerAlg.ViewNodeName = jetTrkSeq.name()

    jetDefString = jetRecoDictToString(jetRecoDict)

    log.debug("Generating jet tracking hypo menu sequence for reco %s",jetDefString)

    jetAthMenuSeq = seqAND(f"jetRoITrackingHypo_{jetDefString}_MenuSequence",
                       [InputMakerAlg]+[jetTrkSeq])


    # Needs track-to-jet association here, maybe with dR decorator
    hypoType = JetHypoAlgType.STANDARD
    return makeMenuSequence(jetAthMenuSeq,InputMakerAlg,jetsIn,jetDefString,hypoType)
