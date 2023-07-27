#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# menu components
from AthenaCommon.CFElements import seqAND
from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from TrigEDMConfig.TriggerEDMRun3 import recordable

from ..Config.MenuComponents import MenuSequence,algorithmCAToGlobalWrapper
# ====================================================================================================
#    Get MenuSequences
# ====================================================================================================


# ====================================================================================================
#    step 1: This is Jet code. Not here!
# ====================================================================================================

# ====================================================================================================
#    step 2: Second stage of fast tracking, Precision tracking, and flavour tagging
# ====================================================================================================

# todo: pass in more information, i.e. jet collection name
def getBJetSequence(flags, jc_name=None):
    if not jc_name:
        raise ValueError("jet collection name is empty - pass the full HLT jet collection name to getBJetSequence().")

    config=getInDetTrigConfig('fullScan')
    prmVtxKey = config.vertex

    bjetconfig    = getInDetTrigConfig('bjet')
    outputRoIName = bjetconfig.roi

    jc_key = f'{jc_name}_'
    # Output container names as defined in TriggerEDMRun3
    BTagName = recordable(f'{jc_key}BTagging')

    from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
    from DecisionHandling.DecisionHandlingConf import ViewCreatorCentredOnJetWithPVConstraintROITool
    InputMakerAlg = EventViewCreatorAlgorithm(
        f"IMBJet_{jc_name}_step2",
        mergeUsingFeature = True,
        RoITool = ViewCreatorCentredOnJetWithPVConstraintROITool(
            RoisWriteHandleKey  = recordable( outputRoIName ),
            VertexReadHandleKey = prmVtxKey,
            PrmVtxLink  = prmVtxKey.replace( "HLT_","" ),
            RoIEtaWidth = bjetconfig.etaHalfWidth,
            RoIPhiWidth = bjetconfig.phiHalfWidth,
            RoIZWidth   = bjetconfig.zedHalfWidth,
        ),
        Views = f"BTagViews_{jc_name}",
        InViewRoIs = "InViewRoIs",
        RequireParentView = False,
        ViewFallThrough = True,
        InViewJets = recordable( f'{jc_key}bJets' ),
        # BJet specific
        PlaceJetInView = True
    )

    # Second stage of Fast Tracking and Precision Tracking
    from TriggerMenuMT.HLT.Bjet.BjetTrackingConfiguration import getSecondStageBjetTracking
    secondStageAlgs, PTTrackParticles = getSecondStageBjetTracking(
        flags,
        inputRoI=InputMakerAlg.InViewRoIs,
        inputVertex=prmVtxKey,
        inputJets=InputMakerAlg.InViewJets
    )

    from TriggerMenuMT.HLT.Bjet.BjetFlavourTaggingConfiguration import getFlavourTagging
    flavourTaggingAlgs = algorithmCAToGlobalWrapper(
        getFlavourTagging,
        flags,
        inputJets=str(InputMakerAlg.InViewJets),
        inputVertex=prmVtxKey,
        inputTracks=PTTrackParticles[0],
        BTagName=BTagName,
        inputMuons=None
    )
    bJetBtagSequence = seqAND( f"bJetBtagSequence_{jc_name}", secondStageAlgs + flavourTaggingAlgs )


    InputMakerAlg.ViewNodeName = f"bJetBtagSequence_{jc_name}"

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = algorithmCAToGlobalWrapper(ROBPrefetchingAlgCfg_Si, flags, nameSuffix=InputMakerAlg.name())[0]

    # Sequence
    BjetAthSequence = seqAND( f"BjetAthSequence_{jc_name}_step2",[InputMakerAlg,robPrefetchAlg,bJetBtagSequence] )

    from TrigBjetHypo.TrigBjetHypoConf import TrigBjetBtagHypoAlg
    from TrigBjetHypo.TrigBjetMonitoringConfig import TrigBjetOnlineMonitoring
    hypo = TrigBjetBtagHypoAlg(
        f"TrigBjetBtagHypoAlg_{jc_name}",
        # keys
        BTaggedJetKey = InputMakerAlg.InViewJets,
        BTaggingKey = BTagName,
        TracksKey = PTTrackParticles[0],
        PrmVtxKey = InputMakerAlg.RoITool.VertexReadHandleKey,
        # links for navigation
        BTaggingLink = BTagName.replace( "HLT_","" ),
        PrmVtxLink = InputMakerAlg.RoITool.PrmVtxLink,
        MonTool = TrigBjetOnlineMonitoring(flags)
    )

    from TrigBjetHypo.TrigBjetBtagHypoTool import TrigBjetBtagHypoToolFromDict
    return MenuSequence( flags,
                         Sequence    = BjetAthSequence,
                         Maker       = InputMakerAlg,
                         Hypo        = hypo,
                         HypoToolGen = TrigBjetBtagHypoToolFromDict)
