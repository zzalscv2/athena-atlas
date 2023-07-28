# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import (seqAND, parOR)
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
from AthenaCommon.Logging import logging

from ..Config.MenuComponents import RecoFragmentsPool, algorithmCAToGlobalWrapper, extractAlgorithmsAndAppendCA
from TrigEDMConfig.TriggerEDMRun3 import recordable
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorCentredOnIParticleROITool
from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetLRTCfg

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

def DJPromptStep(flags):
    from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJHypoPromptToolFromDict
    from TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoConf import DisplacedJetPromptHypoAlg

    hypo_alg = DisplacedJetPromptHypoAlg("DJTrigPromptHypoAlg")

    #get the jet tracking config to get the track collection name
    fscfg = getInDetTrigConfig("fullScan")

    hypo_alg.min_trk_pt = 1.0
    hypo_alg.stdTracksKey = fscfg.tracks_FTF()
    hypo_alg.jetContainerKey = recordable("HLT_AntiKt4EMTopoJets_subjesIS")
    hypo_alg.vtxKey = fscfg.vertex_jet
    hypo_alg.countsKey = "DispJetTrigger_Counts"

    #run at the event level
    from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
    from AthenaConfiguration.ComponentFactory import CompFactory
    im_alg = conf2toConfigurable(CompFactory.InputMakerForRoI( "IM_DJTRIG_Prompt" ))
    im_alg.RoITool = conf2toConfigurable(CompFactory.ViewCreatorInitialROITool())

    return MenuSequence( flags,
                         Sequence    = seqAND("DJTrigPromptEmptyStep",[im_alg]),
                         Maker       = im_alg,
                         Hypo        = hypo_alg,
                         HypoToolGen = TrigDJHypoPromptToolFromDict,
                     )

def DJDispFragment(flags):
    lrtcfg = getInDetTrigConfig( 'DJetLRT' )
    roiTool = ViewCreatorCentredOnIParticleROITool('ViewCreatorDJRoI', RoisWriteHandleKey = recordable(lrtcfg.roi), RoIEtaWidth = lrtcfg.etaHalfWidth, RoIPhiWidth = lrtcfg.phiHalfWidth, RoIZedWidth=lrtcfg.zedHalfWidth, UseZedPosition=False)
    InputMakerAlg = EventViewCreatorAlgorithm("IMDJRoIFTF",mergeUsingFeature = True, RoITool = roiTool,Views = "DJRoIViews",InViewRoIs = "InViewRoIs",RequireParentView = False,ViewFallThrough = True)
    fscfg = getInDetTrigConfig("fullScan")

    flagsWithTrk = getFlagsForActiveConfig(flags, lrtcfg.name, log)

    from AthenaCommon.Configurable import ConfigurableCABehavior
    with ConfigurableCABehavior():
        lrt_ca = trigInDetLRTCfg(
            flagsWithTrk,
            fscfg.trkTracks_FTF(),
            InputMakerAlg.InViewRoIs,
            in_view=True
        )
    lrt_algs = extractAlgorithmsAndAppendCA(lrt_ca)

    reco_seq = parOR("UncTrkrecoSeqDJTrigDispRecoSeq", lrt_algs)

    InputMakerAlg.ViewNodeName = reco_seq.name()

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=InputMakerAlg.name())[0]

    return (seqAND("UncTrkrecoSeqDJTrigDisp", [InputMakerAlg, robPrefetchAlg, reco_seq]), InputMakerAlg)

def DJDispStep(flags):
    from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJHypoDispToolFromDict
    from TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoConf import (DisplacedJetDispHypoAlg)

    hypo_alg = DisplacedJetDispHypoAlg("DJTrigDispHypoAlg")

    lrtcfg = getInDetTrigConfig( 'DJetLRT' )
    fscfg = getInDetTrigConfig("fullScan")

    hypo_alg.lrtTracksKey = lrtcfg.tracks_FTF()
    hypo_alg.vtxKey = fscfg.vertex_jet

    ( alg_seq ,im_alg) = RecoFragmentsPool.retrieve(DJDispFragment,flags)

    return MenuSequence(flags,
                        Sequence    = alg_seq,
                        Maker       = im_alg,
                        Hypo        = hypo_alg,
                        HypoToolGen = TrigDJHypoDispToolFromDict,
                        )
