# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# TRIG8.py
# This defines DAOD_TRIG8, a DAOD format for Run 3.
# It contains the variables and objects needed ID Trigger performance
# such as online and offline tracks, RoIs, and offline objects.  
# Only events passing idperf, and similar, chains are kept.
# It requires the flag TRIG8 in Derivation_tf.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

TRIG8MergedElectronContainer = "StdWithLRTElectrons"
TRIG8MergedMuonContainer = "StdWithLRTMuons"
TRIG8MergedTrackCollection = "InDetWithLRTTrackParticles"

# Main algorithm config
def TRIG8KernelCfg(ConfigFlags, name='TRIG8Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TRIG8"""
    acc = ComponentAccumulator()

    # Augmentations

    # LRT track merge
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleMergerCfg
    TRIG8TrackParticleMergerTool = acc.getPrimaryAndMerge(TrackParticleMergerCfg(
        ConfigFlags,
        name                        = "TRIG8TrackParticleMergerTool",
        TrackParticleLocation       = ["InDetTrackParticles", "InDetLargeD0TrackParticles"],
        OutputTrackParticleLocation = TRIG8MergedTrackCollection,
        CreateViewColllection       = True))

    LRTMergeAug = CompFactory.DerivationFramework.CommonAugmentation("InDetLRTMerge", AugmentationTools = [TRIG8TrackParticleMergerTool])
    acc.addEventAlgo(LRTMergeAug)

    # LRT muons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
    acc.merge(LRTMuonMergerAlg( ConfigFlags,
                                PromptMuonLocation    = "Muons",
                                LRTMuonLocation       = "MuonsLRT",
                                OutputMuonLocation    = TRIG8MergedMuonContainer,
                                CreateViewCollection  = True))

    # LRT electrons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronMergerAlg
    acc.merge(LRTElectronMergerAlg( ConfigFlags,
                                    PromptElectronLocation = "Electrons",
                                    LRTElectronLocation    = "LRTElectrons",
                                    OutputCollectionName   = TRIG8MergedElectronContainer,
                                    isDAOD                 = False,
                                    CreateViewCollection   = True))


    augmentationTools = [ ]

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # LRT Egamma
    from DerivationFrameworkEGamma.EGammaLRTConfig import EGammaLRTCfg
    acc.merge(EGammaLRTCfg(ConfigFlags))

    from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronLHSelectorsCfg
    acc.merge(LRTElectronLHSelectorsCfg(ConfigFlags))

    # LRT Muons
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    acc.merge(MuonsCommonCfg(ConfigFlags,
                             suff="LRT"))


    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg

    # Inner detector group recommendations for indet tracks in analysis
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations

    TRIG8PhotonsThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(
        ConfigFlags,
        name            = "TRIG8PhotonsThinningTool",
        StreamName      = kwargs['StreamName'],
        ContainerName   = "Photons",
        SelectionString = "Photons.pt >= 1000000."))

    TRIG8TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        ConfigFlags,
        name                    = "TRIG8TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = "InDetTrackParticles.pt > 1*GeV",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    TRIG8LRTTrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        ConfigFlags,
        name                    = "TRIG8LRTTrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = "InDetLargeD0TrackParticles.pt > 1*GeV",
        InDetTrackParticlesKey  = "InDetLargeD0TrackParticles"))

    # Finally the kernel itself
    thinningTools = [TRIG8PhotonsThinningTool,
                     TRIG8TrackParticleThinningTool,
                     TRIG8LRTTrackParticleThinningTool]

    # Skimming
    skimmingTools = []

    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod

    allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
    TriggerAPI.setConfigFlags(ConfigFlags)
    trig_all = TriggerAPI.getAllHLT(allperiods)
    
    # Pieces of trigger names to keep
    idtrig_keys = ['idperf', 'boffperf', 'ivarperf', 'idtp'] 
    # Triggers to veto
    idtrig_veto = ['HLT_e26_lhtight_ivarloose_2j20_0eta290_020jvt_boffperf_pf_ftf_L1EM22VHI']
    # Add specific triggers
    additional_triggers = [
        "HLT_mu20_msonly",
        "HLT_j180_2dispjet_2p_L1J100"
    ]    
    idtrig_keys += additional_triggers

    triggers = [t for t in trig_all for k in idtrig_keys if k in t]
    for veto in idtrig_veto:
        try:
            triggers.remove(veto)
        except ValueError:
            print(f"Warning, {veto} already removed from trigger list.")

    #remove duplicates
    triggers = sorted(list(set(triggers)))
    print('TRIG8 list of triggers used for skimming:')
    for trig in triggers: print(trig)

    TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool
    TRIG8TriggerSkimmingTool = TriggerSkimmingTool(name = "TRIG8TriggerPreSkimmingTool", 
                                                   TriggerListAND = [],
                                                   TriggerListOR  = triggers)
    acc.addPublicTool(TRIG8TriggerSkimmingTool)

    skimmingTools.append(TRIG8TriggerSkimmingTool)

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools = skimmingTools,
                                      ThinningTools = thinningTools,
                                      AugmentationTools = augmentationTools))

    return acc


def TRIG8Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TRIG8TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Common augmentations
    acc.merge(TRIG8KernelCfg(ConfigFlags, name="TRIG8Kernel", StreamName = 'StreamDAOD_TRIG8', TriggerListsHelper = TRIG8TriggerListsHelper))


    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    TRIG8SlimmingHelper = SlimmingHelper("TRIG8SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections)

    TRIG8SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "LRTElectrons",
                                            "Photons",
                                            "Muons",
                                            "MuonsLRT",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "InDetLargeD0TrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt4EMPFlowJets",
                                            "BTagging_AntiKt4EMTopo",
                                            "BTagging_AntiKt4EMPFlow",
                                            "TauJets"]

    TRIG8SlimmingHelper.AllVariables =  ["HLT_IDTrack_Electron_FTF", 
                                        "HLT_IDTrack_ElecLRT_FTF", 
                                        "HLT_IDTrack_Electron_IDTrig", 
                                        "HLT_IDTrack_ElecLRT_IDTrig", 
                                        "HLT_IDTrack_Electron_GSF",
                                        "HLT_IDTrack_Electron_LRTGSF",
                                        "HLT_IDTrack_Muon_FTF", 
                                        "HLT_IDTrack_Muon_IDTrig", 
                                        "HLT_IDTrack_MuonLRT_IDTrig", 
                                        "HLT_IDTrack_MuonIso_FTF", 
                                        "HLT_IDTrack_MuonIso_IDTrig", 
                                        "HLT_IDTrack_MuonLRT_FTF", 
                                        "HLT_IDTrack_Bmumux_FTF", 
                                        "HLT_IDTrack_Bmumux_IDTrig", 
                                        "HLT_IDTrack_TauCore_FTF", 
                                        "HLT_IDTrack_TauLRT_FTF", 
                                        "HLT_IDTrack_TauIso_FTF", 
                                        "HLT_IDTrack_Tau_IDTrig", 
                                        "HLT_IDTrack_TauLRT_IDTrig", 
                                        "HLT_IDTrack_FS_FTF", 
                                        "HLT_IDTrack_FSLRT_FTF", 
                                        "HLT_IDTrack_FSLRT_IDTrig", 
                                        "HLT_IDTrack_DVLRT_FTF", 
                                        "HLT_IDTrack_BeamSpot_FTF", 
                                        "HLT_IDTrack_JetSuper_FTF", 
                                        "HLT_IDTrack_Bjet_FTF", 
                                        "HLT_IDTrack_Bjet_IDTrig", 
                                        "HLT_IDTrack_MinBias_IDTrig", 
                                        "HLT_IDTrack_Cosmic_FTF", 
                                        "HLT_IDTrack_Cosmic_IDTrig", 
                                        "HLT_IDTrack_DJLRT_FTF",
                                        "BTagging_AntiKt4EMPFlowSecVtx",
                                        "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingSecVtx",
                                        "HLT_IDVertex_FS",
                                        "HLT_IDVertex_JetSuper",
                                        "HLT_IDVertex_Tau"]


    TRIG8SlimmingHelper.StaticContent = [
                        "TrigRoiDescriptorCollection#HLT_Roi_Bjet",
                        "TrigRoiDescriptorCollection#HLT_Roi_DJ",
                        "TrigRoiDescriptorCollection#HLT_Roi_FS",
                        "TrigRoiDescriptorCollection#HLT_Roi_FastElectron",
                        "TrigRoiDescriptorCollection#HLT_Roi_FastElectron_LRT",
                        "TrigRoiDescriptorCollection#HLT_Roi_FastPhoton",
                        "TrigRoiDescriptorCollection#HLT_Roi_IDCalibPEB",
                        "TrigRoiDescriptorCollection#HLT_Roi_JetSuper",
                        "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon",
                        "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuonForEF",
                        "TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon_LRT",
                        "TrigRoiDescriptorCollection#HLT_Roi_LArPEBHLT",
                        "TrigRoiDescriptorCollection#HLT_Roi_MuonIso",
                        "TrigRoiDescriptorCollection#HLT_Roi_Tau",
                        "TrigRoiDescriptorCollection#HLT_Roi_TauCore",
                        "TrigRoiDescriptorCollection#HLT_Roi_TauIso",
                        "TrigRoiDescriptorCollection#HLT_Roi_TauIsoBDT",
                        "TrigRoiDescriptorCollection#HLT_Roi_TauLRT",
                        "TrigInDetTrackTruthMap#TrigInDetTrackTruthMap"
                        ]


    TRIG8SlimmingHelper.ExtraVariables += [ 
                        "Electrons.Tight.Medium.Loose.LHTight.LHMedium.LHLoose",
                        "LRTElectrons.Tight.Medium.Loose.LHTight.LHMedium.LHLoose",
                        "egammaClusters.phi_sampl.eta0.phi0",
                        "LRTegammaClusters.phi_sampl.eta0.phi0",
                        "TruthPrimaryVertices.t.x.y.z",
                        "PrimaryVertices.t.x.y.z.numberDoF.chiSquared.covariance.trackParticleLinks",
                        "InDetTrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern.trackFitter.patternRecoInfo.numberDoF.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit",
                        "InDetLargeD0TrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern.trackFitter.patternRecoInfo.numberDoF.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit",
                        "GSFTrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.trackFitter.patternRecoInfo.hitPattern.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit",
                        "LRTGSFTrackParticles.d0.z0.vz.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.trackFitter.patternRecoInfo.hitPattern.numberOfTRTHits.numberOfTRTOutliers.numberOfBLayerHits.expectBLayerHit.numberOfPixelDeadSensors.numberOfSCTDeadSensors.numberOfTRTHighThresholdHits.expectInnermostPixelLayerHit",
                        "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                        "TauJets.dRmax.etOverPtLeadTrk"]


    # Truth containers
    if ConfigFlags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(TRIG8SlimmingHelper)
        TRIG8SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        TRIG8SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "LRTElectrons.TruthLink",
                                              "Muons.TruthLink",
                                              "MuonsLRT.TruthLink",
                                              "Photons.TruthLink"]



    # Trigger content
    TRIG8SlimmingHelper.IncludeTriggerNavigation = True
    TRIG8SlimmingHelper.IncludeAdditionalTriggerContent = True
    TRIG8SlimmingHelper.IncludeJetTriggerContent = False
    TRIG8SlimmingHelper.IncludeMuonTriggerContent = False
    TRIG8SlimmingHelper.IncludeEGammaTriggerContent = False
    TRIG8SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    TRIG8SlimmingHelper.IncludeTauTriggerContent = False
    TRIG8SlimmingHelper.IncludeEtMissTriggerContent = False
    TRIG8SlimmingHelper.IncludeBJetTriggerContent = False
    TRIG8SlimmingHelper.IncludeBPhysTriggerContent = False
    TRIG8SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkLLP.LLPToolsConfig import LLP1TriggerMatchingToolRun2Cfg
        acc.merge(LLP1TriggerMatchingToolRun2Cfg(ConfigFlags,
                                              name = "LRTTriggerMatchingTool",
                                              OutputContainerPrefix = "LRTTrigMatch_",
                                              TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesNoTau,
                                              InputElectrons=TRIG8MergedElectronContainer,
                                              InputMuons=TRIG8MergedMuonContainer
                                              ))
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TRIG8SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TRIG8SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesNoTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TRIG8SlimmingHelper,
                                         OutputContainerPrefix = "LRTTrigMatch_",
                                         TriggerList = TRIG8TriggerListsHelper.Run2TriggerNamesNoTau,
                                         InputElectrons=TRIG8MergedElectronContainer,
                                         InputMuons=TRIG8MergedMuonContainer
                                         )
    # Run 3
    elif ConfigFlags.Trigger.EDMVersion == 3:
        from DerivationFrameworkLLP.LLPToolsConfig import LLP1TriggerMatchingToolRun2Cfg
        acc.merge(LLP1TriggerMatchingToolRun2Cfg(ConfigFlags,
                                              name = "LRTTriggerMatchingTool",
                                              OutputContainerPrefix = "LRTTrigMatch_",
                                              TriggerList = TRIG8TriggerListsHelper.Run3TriggerNamesNoTau,
                                              InputElectrons=TRIG8MergedElectronContainer,
                                              InputMuons=TRIG8MergedMuonContainer
                                              ))
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TRIG8SlimmingHelper)

    # Output stream
    TRIG8ItemList = TRIG8SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_TRIG8", ItemList=TRIG8ItemList, AcceptAlgs=["TRIG8Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_TRIG8", AcceptAlgs=["TRIG8Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

