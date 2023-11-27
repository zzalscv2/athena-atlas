# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Logging import logging

from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetLRTCfg

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, InEventRecoCA


logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

trkPairOutName = "HLT_TrigDV_VSITrkPair"
vtxOutName = "HLT_TrigDV_VSIVertex"
vtxCountName = "HLT_TrigDV_VtxCount"

def DVRecoFragment(flags):

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    fscfg = getInDetTrigConfig("fullScan")
    lrtcfg = getInDetTrigConfig( 'DVtxLRT' )


    selAcc = SelectionCA("DVRecoSequence1")
    
    from TrigEDMConfig.TriggerEDMRun3 import recordable
    inputMakerAlg = CompFactory.EventViewCreatorAlgorithm(
        "IMDVRoILRT",
        mergeUsingFeature = False,
        RoITool = CompFactory.ViewCreatorDVROITool(
            'ViewCreatorDVRoI',
            RoisWriteHandleKey  = recordable( lrtcfg.roi ),
            RoIEtaWidth = lrtcfg.etaHalfWidth,
            RoIPhiWidth = lrtcfg.phiHalfWidth,
        ),
        Views = "DVRoIViews",
        InViewRoIs = "InViewRoIs",
        RequireParentView = False,
        ViewFallThrough = True,
        ViewNodeName = selAcc.name+'InView',
    )
    
    reco = InViewRecoCA('DVRecoStep',viewMaker=inputMakerAlg)

    flagsWithTrk = getFlagsForActiveConfig(flags, lrtcfg.name, log)

    lrt_algs = trigInDetLRTCfg(
        flagsWithTrk,
        fscfg.trkTracks_FTF(),
        inputMakerAlg.InViewRoIs,
        in_view=True,
        extra_view_inputs=(
            ( 'xAOD::TrackParticleContainer' , fscfg.tracks_FTF() ),
            ( 'xAOD::VertexContainer' ,        fscfg.vertex ),
        )
    )

    from TrigEDMConfig.TriggerEDMRun3 import recordable
    from TrigVrtSecInclusive.TrigVrtSecInclusiveConfig import TrigVrtSecInclusiveCfg
    vertexingAlgs = TrigVrtSecInclusiveCfg( flags, "TrigVrtSecInclusive_TrigDV",
                                            FirstPassTracksName = fscfg.tracks_FTF(),
                                            SecondPassTracksName = lrtcfg.tracks_FTF(),
                                            PrimaryVertexInputName = fscfg.vertex,
                                            VxCandidatesOutputName = recordable(vtxOutName),
                                            TrkPairOutputName = recordable(trkPairOutName) )

    recoAlgSequence = seqAND("DVRecoSeq")
    acc = ComponentAccumulator()

    acc.addSequence(recoAlgSequence)
    
    acc.merge(lrt_algs)
    acc.merge(vertexingAlgs)
    
    reco.mergeReco(acc)
    selAcc.mergeReco(reco)
    return selAcc




def DVRecoSequence(flags):
    from TrigStreamerHypo.TrigStreamerHypoConfig import StreamerHypoToolGenerator

    selAcc = DVRecoFragment(flags)

    HypoAlg = CompFactory.TrigStreamerHypoAlg("TrigDVRecoDummyStream")
    selAcc.addHypoAlgo(HypoAlg)

    log.debug("Building the Step dictinary for TrigDV reco")
    return MenuSequenceCA(flags,
                        selAcc,
                        HypoToolGen = StreamerHypoToolGenerator
                        )




def DVTriggerEDSequence(flags):
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import TrigVSIHypoToolFromDict
    from TrigLongLivedParticlesHypo.TrigVrtSecInclusiveHypoConfig import createTrigVSIHypoAlgCfg

    selAcc = SelectionCA("TrigDVEDEmptyStep")

    from TrigEDMConfig.TriggerEDMRun3 import recordable
    theHypoAlg = createTrigVSIHypoAlgCfg(flags, "TrigDVHypoAlg",
                                         verticesKey = recordable(vtxOutName),
                                         vtxCountKey = recordable(vtxCountName))


    #run at the event level
    inputMakerAlg = CompFactory.InputMakerForRoI( "IM_TrigDV_ED" )
    inputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()

    reco = InEventRecoCA('DVEDStep',inputMaker=inputMakerAlg)

    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(theHypoAlg)

    log.info("Building the Step dictinary for DisVtxTrigger!")
    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = TrigVSIHypoToolFromDict,
                          )
