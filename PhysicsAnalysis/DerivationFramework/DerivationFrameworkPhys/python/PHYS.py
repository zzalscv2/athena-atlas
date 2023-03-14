# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_PHYS.py
# This defines DAOD_PHYS, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag PHYS in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Main algorithm config
def PHYSKernelCfg(ConfigFlags, name='PHYSKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PHYS"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule 
    thinningToolsArgs = {
        'TrackParticleThinningToolName'       : "PHYSTrackParticleThinningTool",
        'MuonTPThinningToolName'              : "PHYSMuonTPThinningTool",
        'TauJetThinningToolName'              : "PHYSTauJetThinningTool",
        'TauJets_MuonRMThinningToolName'      : "PHYSTauJets_MuonRMThinningTool",
        'DiTauTPThinningToolName'             : "PHYSDiTauTPThinningTool",
        'DiTauLowPtThinningToolName'          : "PHYSDiTauLowPtThinningTool",
        'DiTauLowPtTPThinningToolName'        : "PHYSDiTauLowPtTPThinningTool",
    } 
    # Configure the thinning tools
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(ConfigFlags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools))       
    return acc


def PHYSCfg(ConfigFlags):
    stream_name = 'StreamDAOD_PHYS'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    PHYSTriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Common augmentations
    acc.merge(PHYSKernelCfg(ConfigFlags, name="PHYSKernel", StreamName = stream_name, TriggerListsHelper = PHYSTriggerListsHelper))
    
    ## Higgs augmentations    
    from DerivationFrameworkHiggs.HiggsPhysContent import  setupHiggsAugmentationAlgs, setupHiggsSlimmingVariables
    acc.merge(setupHiggsAugmentationAlgs(ConfigFlags, stream_name = stream_name))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    PHYSSlimmingHelper = SlimmingHelper("PHYSSlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)
    PHYSSlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
                                           "BTagging_AntiKt4EMPFlow",
                                           "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                           "MET_Baseline_AntiKt4EMTopo",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "TauJets",
                                           "TauJets_MuonRM",
                                           "DiTauJets",
                                           "DiTauJetsLowPt",
                                           "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                          ]
    
    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]   

    PHYSSlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    PHYSSlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]
 
    # Truth extra content
    if ConfigFlags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(PHYSSlimmingHelper)
        PHYSSlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        PHYSSlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink"]

    setupHiggsSlimmingVariables(ConfigFlags, PHYSSlimmingHelper)
   
   
    # Trigger content
    PHYSSlimmingHelper.IncludeTriggerNavigation = False
    PHYSSlimmingHelper.IncludeJetTriggerContent = False
    PHYSSlimmingHelper.IncludeMuonTriggerContent = False
    PHYSSlimmingHelper.IncludeEGammaTriggerContent = False
    PHYSSlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    PHYSSlimmingHelper.IncludeTauTriggerContent = False
    PHYSSlimmingHelper.IncludeEtMissTriggerContent = False
    PHYSSlimmingHelper.IncludeBJetTriggerContent = False
    PHYSSlimmingHelper.IncludeBPhysTriggerContent = False
    PHYSSlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = PHYSTriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = PHYSTriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if ConfigFlags.Trigger.EDMVersion == 3 or (ConfigFlags.Trigger.EDMVersion == 2 and ConfigFlags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(PHYSSlimmingHelper)
        ##################################################### 
        ## NOTE: This block is temporary, during validation of the doEDMVersionConversion flag.
        ## This adds a LOT of containers to the output! In order to help validate the conversion.
        ## It should be removed once doEDMVersionConversion goes into production use.
        if ConfigFlags.Trigger.doEDMVersionConversion:   
            from DerivationFrameworkTrigger.TrigSlimmingHelper import addTrigEDMSetToOutput
            from AthenaCommon.Logging import logging
            msg = logging.getLogger('PHYSCfg')
            msg.warn('doEDMVersionConversion is still in validation, WRITING FULL TRIGGER EDM TO THE DAOD!')
            addTrigEDMSetToOutput(ConfigFlags, PHYSSlimmingHelper, "AODFULL")
            PHYSSlimmingHelper.AppendToDictionary.update({'HLTNav_R2ToR3Summary':'xAOD::TrigCompositeContainer'})
            PHYSSlimmingHelper.AllVariables += ['HLTNav_R2ToR3Summary']
        ##
        #####################################################

    # Output stream    
    PHYSItemList = PHYSSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_PHYS", ItemList=PHYSItemList, AcceptAlgs=["PHYSKernel"]))

    return acc

