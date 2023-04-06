#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from enum import Enum
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory

from .JetRecoCommon import jetRecoDictToString
from ..CommonSequences.FullScanDefs import  trkFSRoI, em_clusters, lc_clusters
from ..CommonSequences.CaloConfig import CaloClusterCfg
from ..Config.MenuComponents import parOR
from TrigEDMConfig.TriggerEDMRun3 import recordable

# Hypo tool generators
from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetHypoToolFromDict
from .JetPresel import caloPreselJetHypoToolFromDict, roiPreselJetHypoToolFromDict

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

###############################################################################################
### --- Input Makers ----

# For step 1, starting from the basic calo reco and topoclustering
# Used for calo-only chains and preselection for tracking
def getCaloInputMaker():
    from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
    InputMakerAlg = clusterFSInputMaker()
    return InputMakerAlg

# For later steps, where calo reco should not be run
# The same instance of an algorithm cannot be run in different steps
# Used for chains that use tracking
def getTrackingInputMaker(trkopt):
    if trkopt=="ftf":

        IDTrigConfig = getInDetTrigConfig( 'jet' )

        log.debug( "jet FS tracking: useDynamicRoiZWidth: %s", IDTrigConfig.useDynamicRoiZWidth )
        
        roiUpdater = None
        if IDTrigConfig.useDynamicRoiZWidth:
            roiUpdater = CompFactory.RoiUpdaterTool( useBeamSpot=True )

            log.info( roiUpdater )

            InputMakerAlg = CompFactory.InputMakerForRoI( "IM_Jet_TrackingStep",
                                                          mergeUsingFeature = False,
                                                          RoITool = CompFactory.ViewCreatorFSROITool( name="RoiTool_FS", 
                                                                                                      RoiUpdater=roiUpdater,
                                                                                                      RoisWriteHandleKey=recordable( IDTrigConfig.roi ) ),
                                                          RoIs = trkFSRoI )
        else: 
            InputMakerAlg = CompFactory.InputMakerForRoI( "IM_Jet_TrackingStep",
                                                          mergeUsingFeature = False,
                                                          RoITool = CompFactory.ViewCreatorInitialROITool(),
                                                          RoIs = trkFSRoI)



    elif trkopt=="roiftf":
        IDTrigConfig = getInDetTrigConfig( 'jetSuper' )
        InputMakerAlg = CompFactory.EventViewCreatorAlgorithm(
            "IMJetRoIFTF",
            mergeUsingFeature = False,
            RoITool = CompFactory.ViewCreatorJetSuperROITool(
                'ViewCreatorJetSuperRoI',
                RoisWriteHandleKey  = recordable( IDTrigConfig.roi ),
                RoIEtaWidth = IDTrigConfig.etaHalfWidth,
                RoIPhiWidth = IDTrigConfig.phiHalfWidth,
                RoIZWidth   = IDTrigConfig.zedHalfWidth,
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
    CALOPRESEL = 1
    ROIPRESEL = 2
    PASSTHROUGH = 3

def jetSelectionCfg(flags, reco, jetsIn, hypoType):
    selname = reco.name.replace('RecoSequence','MenuSequence')
    if hypoType==JetHypoAlgType.PASSTHROUGH:
        hyponame = "TrigStreamerHypoAlg_passthrough"
        selname += "_passthrough"
        hypo = CompFactory.TrigStreamerHypoAlg(hyponame)
    else:
        assert jetsIn is not None
        if hypoType==JetHypoAlgType.CALOPRESEL:
            hyponame = f"TrigJetHypoAlg_{jetsIn}_calopresel"
            selname += "_calopresel"
            hypo = CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn, DoPresel=True)
        elif hypoType==JetHypoAlgType.ROIPRESEL:
            hyponame = f"TrigJetHypoAlg_{jetsIn}_roipresel"
            selname += "_roipresel"
            hypo = CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn, DoPresel=True)
        else:
            hyponame = f"TrigJetHypoAlg_{jetsIn}"
            hypo = CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn)

    log.debug("Generating jet SelectionCA for hypo %s",hyponame)

    selAcc = SelectionCA(selname)
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(hypo)

    return selAcc

def makeMenuSequenceCA(flags, reco, jetsIn=None, hypoType=JetHypoAlgType.STANDARD):
    def trigStreamerHypoTool(chain_dict):
        return CompFactory.TrigStreamerHypoTool(chain_dict["chainName"])

    trigHypoToolGen = {
        JetHypoAlgType.STANDARD:    trigJetHypoToolFromDict,
        JetHypoAlgType.PASSTHROUGH: trigStreamerHypoTool,
        JetHypoAlgType.CALOPRESEL:  caloPreselJetHypoToolFromDict,
        JetHypoAlgType.ROIPRESEL:   roiPreselJetHypoToolFromDict,
    }[hypoType]

    selAcc = jetSelectionCfg(flags, reco, jetsIn, hypoType)

    return MenuSequenceCA(flags, selAcc, HypoToolGen=trigHypoToolGen)


###############################################################################################
### --- Menu Sequence getters ---

# For the preselection step before running tracking (step 1)
# We set RoIs='' (recognised as seedless) instead of caloFSRoI (output of caloInputMater()) to
# cut data dependency to InputMaker and allow full scan CaloCell+Clustering to be
# shared with EGamma (ATR-24722)
def jetCaloPreselMenuSequence(flags, **jetRecoDict):
    jetDefString = jetRecoDictToString(jetRecoDict)
    reco = InEventRecoCA(f"jetSeqCaloPresel_{jetDefString}_RecoSequence", inputMaker=getCaloInputMaker())

    doLCCalib = jetRecoDict['clusterCalib']=='lcw'
    reco.mergeReco( CaloClusterCfg(flags, doLCCalib=doLCCalib) )
    clustersKey = lc_clusters if doLCCalib else em_clusters

    from .JetRecoSequencesConfig import JetRecoCfg
    jetreco, jetsOut, jetDef = JetRecoCfg(flags, clustersKey=clustersKey, **jetRecoDict)
    reco.mergeReco(jetreco)
    log.debug("Generating jet preselection menu sequence for reco %s",jetDef.fullname())

    return makeMenuSequenceCA(flags, reco, jetsIn=jetDef.fullname(),
                            hypoType=JetHypoAlgType.CALOPRESEL), jetDef, clustersKey

# A null preselection, which will only run the cluster making (step 1)
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
def jetCaloRecoMenuSequence(flags, clusterCalib):
    reco = InEventRecoCA(f"jetSeqCaloReco_{clusterCalib}_RecoSequence", inputMaker=getCaloInputMaker())

    doLCCalib = clusterCalib=='lcw'
    reco.mergeReco( CaloClusterCfg(flags, doLCCalib) )
    clustersKey = lc_clusters if doLCCalib else em_clusters

    return makeMenuSequenceCA(flags, reco, hypoType=JetHypoAlgType.PASSTHROUGH), clustersKey

# A full hypo selecting only on calo jets (step 1)
# Passing isPerf = True disables the hypo
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
def jetCaloHypoMenuSequence(flags, isPerf, **jetRecoDict):
    jetDefString = jetRecoDictToString(jetRecoDict)
    reco = InEventRecoCA(f"jetSeqCaloHypo_{jetDefString}_RecoSequence", inputMaker=getCaloInputMaker())

    doLCCalib = jetRecoDict['clusterCalib']=='lcw'
    reco.mergeReco( CaloClusterCfg(flags, doLCCalib) )
    clustersKey = lc_clusters if doLCCalib else em_clusters

    if jetRecoDict["trkopt"] != "notrk":
        from .JetTrackingConfig import JetFSTrackingCfg
        trk_acc, trkcolls = JetFSTrackingCfg(flags, jetRecoDict["trkopt"], trkFSRoI)
        reco.mergeReco(trk_acc)

    from .JetRecoSequencesConfig import JetRecoCfg
    jetreco, jetsOut, jetDef = JetRecoCfg(flags, clustersKey=clustersKey, **jetRecoDict)
    reco.mergeReco(jetreco)
    log.debug("Generating jet calo hypo menu sequence for reco %s",jetDef.fullname())

    hypoType = JetHypoAlgType.STANDARD
    if isPerf: hypoType = JetHypoAlgType.PASSTHROUGH
    return makeMenuSequenceCA(flags, reco, jetsIn=jetDef.fullname(), hypoType=hypoType) ,jetDef

# A full hypo selecting only on heavy ion calo jets (step 1)
# Passing isPerf = True disables the hypo
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
def jetHICaloHypoMenuSequence(flags, isPerf, **jetRecoDict):
    jetDefString = jetRecoDictToString(jetRecoDict)
    reco = InEventRecoCA(f"jetSeqHICaloHypo_{jetDefString}_RecoSequence", inputMaker=getCaloInputMaker())

    from ..CommonSequences.CaloConfig import HICaloTowerCfg
    reco.mergeReco( HICaloTowerCfg(flags) )

    from .JetHIConfig import JetHICfg
    jetreco, jetsOut, jetDef = JetHICfg(flags, clustersKey=em_clusters, **jetRecoDict)
    reco.mergeReco(jetreco)
    log.debug("Generating jet HI calo hypo menu sequence for reco %s",jetDef.fullname())

    hypoType = JetHypoAlgType.STANDARD
    if isPerf: hypoType = JetHypoAlgType.PASSTHROUGH
    return makeMenuSequenceCA(flags, reco, jetsIn=jetDef.fullname(), hypoType=hypoType), jetDef

# A full hypo selecting on jets with FS track reco (step 2)
# To combine either with a presel or a passthrough sequence
# As this does not run topoclustering, the cluster collection
# name needs to be passed in
def jetFSTrackingHypoMenuSequence(flags, clustersKey, isPerf, **jetRecoDict):
    jetDefString = jetRecoDictToString(jetRecoDict)
    reco = InEventRecoCA(f"jetFSTrackingHypo_{jetDefString}_RecoSequence", inputMaker=getTrackingInputMaker(jetRecoDict['trkopt']))

    assert jetRecoDict["trkopt"] != "notrk"
    from .JetTrackingConfig import JetFSTrackingCfg
    trk_acc, trkcolls = JetFSTrackingCfg(flags, jetRecoDict["trkopt"], trkFSRoI)
    reco.mergeReco(trk_acc)

    from .JetRecoSequencesConfig import JetRecoCfg
    jetreco, jetsOut, jetDef = JetRecoCfg(flags, clustersKey=clustersKey, trkcolls=trkcolls, **jetRecoDict)
    reco.mergeReco(jetreco)
    log.debug("Generating jet tracking hypo menu sequence for reco %s",jetDef.fullname())

    hypoType = JetHypoAlgType.STANDARD
    if isPerf: hypoType = JetHypoAlgType.PASSTHROUGH
    return makeMenuSequenceCA(flags, reco, jetsIn=jetDef.fullname(), hypoType=hypoType), jetDef

# A full hypo selecting on jets with RoI track reco (step 2)
# Needs to be preceded by a presel sequence, and be provided
# with the input jets from which to define RoIs
# Presel jets to be reused, which makes ghost association impossible
# Substitute DR association decorator
def jetRoITrackJetTagHypoMenuSequence(flags, jetsIn, isPresel=True, **jetRecoDict):
    jetDefString = jetRecoDictToString(jetRecoDict)
    # Seems odd, but we have to combine event and view execution here
    # where InViewRecoCA will do all in view
    reco = InEventRecoCA(
        f"jetRoITrackJetTagHypo_{jetDefString}_RecoSequence",
        inputMaker=getTrackingInputMaker(jetRecoDict['trkopt'])
    )

    # Add to top-level serial sequence after IM
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    reco.mergeReco(ROBPrefetchingAlgCfg_Si(flags, nameSuffix=reco.inputMaker().name))

    # Add to top-level serial sequence to ensure it is ready for in-view reco
    from .JetRecoSequencesConfig import FastFtaggedJetCopyAlgCfg, JetRoITrackJetTagSequenceCfg, JetViewAlgCfg
    ftagjet_acc, ftaggedJetsIn = FastFtaggedJetCopyAlgCfg(flags,jetsIn=jetsIn,jetRecoDict=jetRecoDict)
    reco.mergeReco(ftagjet_acc)
    ftaggedJetsIn=recordable(ftaggedJetsIn)

    track_acc = JetRoITrackJetTagSequenceCfg(
        flags,
        jetsIn,
        jetRecoDict['trkopt'],
        RoIs=reco.inputMaker().InViewRoIs)
    # Explicitly add the sequence here that is to run in the super-RoI view
    seqname = f"JetRoITrackJetTag_{jetRecoDict['trkopt']}_RecoSequence"
    reco.addSequence(parOR(seqname))
    reco.merge(track_acc,seqname)
    reco.inputMaker().ViewNodeName = seqname

    # Run the JetViewAlg sequence to filter out low pT jets
    # Have to run it outside of JetRoITrackJetTagSequence (which runs in EventView), so that hypo recognises the filtered jets.
    jetview_Acc, filtered_jetsIn = JetViewAlgCfg(flags,jetsIn=ftaggedJetsIn,**jetRecoDict)
    reco.merge(jetview_Acc)

    # Needs track-to-jet association here, maybe with dR decorator
    hypoType = JetHypoAlgType.ROIPRESEL if isPresel else JetHypoAlgType.STANDARD
    return makeMenuSequenceCA(flags, reco, jetsIn=filtered_jetsIn, hypoType=hypoType)