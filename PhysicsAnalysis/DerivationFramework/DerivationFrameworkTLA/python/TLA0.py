# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# Slimmed DAOD_PHYS.py for Run 3 trigger-object level analyses (TLAs)
# It contains the variables and objects needed for  Run 3 TLA searches.
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Skimming config
def TLA0SkimmingCfg(ConfigFlags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    
    from DerivationFrameworkTLA.TLATriggerList import SupportSingleJetTriggers, SupportMultiJetTriggers, SupportPhotonTriggers, PrimaryISRTLATriggers, SupportTLATriggers, PrimarySingleJetTLATriggers, PrimaryMultiJetTLATriggers
    
    tlaFullTriggerList =   SupportSingleJetTriggers +  SupportMultiJetTriggers + SupportPhotonTriggers + PrimaryISRTLATriggers + SupportTLATriggers + PrimarySingleJetTLATriggers + PrimaryMultiJetTLATriggers

    if not ConfigFlags.Input.isMC:
        TLA0TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( 
                        name          = "TLA0TriggerSkimmingTool1",
                        TriggerListOR = tlaFullTriggerList
        )
        acc.addPublicTool(TLA0TriggerSkimmingTool, primary=True)
    
    return acc


# Main thinning config and common augmentations
def TLA0KernelCfg(ConfigFlags, name='TLA0Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TLA0"""
    acc = ComponentAccumulator()

    # Skimming
    skimmingTool = None
    if not ConfigFlags.Input.isMC:
        skimmingTool = acc.getPrimaryAndMerge(TLA0SkimmingCfg(ConfigFlags))

    # Common augmentations
    from DerivationFrameworkTLA.TLACommonConfig import TLACommonAugmentationsCfg
    acc.merge(TLACommonAugmentationsCfg(ConfigFlags, prefix="TLA0_", TriggerListsHelper = kwargs['TriggerListsHelper']))
    
    from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
    DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
        ConfigFlags,
        name           = "DFCommonTrackSelectionLoose",
        ContainerName  = "InDetTrackParticles",
        DecorationName = "DFTLA0Loose"))
    DFCommonTrackSelection.TrackSelectionTool.CutLevel = "Loose" 

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("TLA0CommonKernel", AugmentationTools = [DFCommonTrackSelection]))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetTrackParticleThinningCfg

    # Include inner detector tracks associated with muons
    TLA0MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "TLA0MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    TLA0ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "TLA0ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    TLA0_thinning_expression = "InDetTrackParticles.DFTLA0Loose && ( abs(InDetTrackParticles.d0) < 5.0*mm ) && ( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"

    TLA0Akt4JetTPThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "TLA0Akt4JetTPThinningTool",
        StreamName              = kwargs['StreamName'],
        JetKey                  = "AntiKt4EMTopoJets",
        SelectionString         = "AntiKt4EMTopoJets.pt > 18*GeV",
        TrackSelectionString    = TLA0_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    TLA0Akt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        ConfigFlags,
        name                         = "TLA0Akt4PFlowJetTPThinningTool",
        StreamName                   = kwargs['StreamName'],
        JetKey                       = "AntiKt4EMPFlowJets",
        SelectionString              = "AntiKt4EMPFlowJets.pt > 18*GeV",
        TrackSelectionString         = TLA0_thinning_expression,
        InDetTrackParticlesKey       = "InDetTrackParticles"))

    # Finally the kernel itself
    thinningTools = [TLA0MuonTPThinningTool,
                     TLA0ElectronTPThinningTool,
                     TLA0Akt4JetTPThinningTool,
                     TLA0Akt4PFlowJetTPThinningTool]
    
    # create the derivation kernel
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(
        name, 
        ThinningTools = thinningTools,
        SkimmingTools = [skimmingTool] if skimmingTool is not None else []
    ))


    return acc

# Main setup of the config & format
def TLA0Cfg(ConfigFlags):
    stream_name = 'StreamDAOD_TLA0'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TLA0TriggerListsHelper = TriggerListsHelper(ConfigFlags)


    # Common augmentations and TLA0 thinning & skimming
    acc.merge(TLA0KernelCfg(ConfigFlags, name="TLA0Kernel", StreamName = stream_name, TriggerListsHelper = TLA0TriggerListsHelper))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    TLA0SlimmingHelper = SlimmingHelper("TLA0SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)

    TLA0SlimmingHelper.SmartCollections = [
                        "EventInfo",
                        "Electrons",
                        "Photons",
                        "PrimaryVertices",
                        "Muons",
                        "AntiKt4EMTopoJets",
                        "AntiKt4EMPFlowJets",
                        "BTagging_AntiKt4EMPFlow",
    ]
    
    # Extra content
    if ConfigFlags.Input.isMC:
        TLA0SlimmingHelper.ExtraVariables += [
            "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
            
            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
            
            "TruthPrimaryVertices.t.x.y.z",
                                                        
            "EventInfo.hardScatterVertexLink.timeStampNSOffset",
        ]
    else:
        TLA0SlimmingHelper.ExtraVariables += [
            "AntiKt4EMTopoJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
            
            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                                                    
            "EventInfo.hardScatterVertexLink.timeStampNSOffset", 
        ]

    TLA0SlimmingHelper.AllVariables = [
        # store event shape variables to get full objects (also included by jet CP content)
        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape", # EMTopo and EMPFlow event shapes
        "Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape", # newer event shapes for testing (e.g. if offline jet calibration changes)
        # store muon segments in case they are needed for offline jet calibrations
        "MuonSegments",
    ]

    # add eEM RoIs
    # based on L1CALOCore.py implementation
    L1Calo_eEM_postfix = "" # empty unless otherwise set
    # append to slimming helper dictionaties so that the code knows the container type
    TLA0SlimmingHelper.AppendToDictionary.update(
        {"L1_eEMRoI"+L1Calo_eEM_postfix :  "xAOD::eFexEMRoIContainer",
         "L1_eEMRoI"+L1Calo_eEM_postfix+"Aux" : "xAOD::eFexEMRoIAuxContainer"})
    # add the RoIs to the derivation   
    TLA0SlimmingHelper.AllVariables += ["L1_eEMRoI"+L1Calo_eEM_postfix]

    
    # Truth extra content
    if ConfigFlags.Input.isMC:
        # from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        from DerivationFrameworkTLA.TLACommonConfig import addTLATruth3ContentToSlimmerTool
        addTLATruth3ContentToSlimmerTool(TLA0SlimmingHelper)
        TLA0SlimmingHelper.AllVariables += [
            'TruthHFWithDecayParticles',
            'TruthHFWithDecayVertices',
            'TruthCharm',
            'TruthPileupParticles',
            'InTimeAntiKt4TruthJets',
            'OutOfTimeAntiKt4TruthJets',
        ]
        TLA0SlimmingHelper.ExtraVariables += [
                                    "Electrons.TruthLink",
                                    "Photons.TruthLink"
                                ]
        # truth jet collections for calibrations and performance studies
        # replicates jet collection configuration in JETM1 (with the exception of AntiKt4TruthDressedWZJets which doesn't exist there)
        TLA0SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]
        TLA0SlimmingHelper.AllVariables += ["AntiKt4TruthJets", "AntiKt4TruthDressedWZJets"]
   
    # Trigger content
    # only save B-jet trigger content and trigger navigation when running on MC
    TLA0SlimmingHelper.IncludeTriggerNavigation = True
    TLA0SlimmingHelper.IncludeJetTriggerContent = True
    TLA0SlimmingHelper.IncludeMuonTriggerContent = False
    TLA0SlimmingHelper.IncludeTrackingTriggerContent = True
    TLA0SlimmingHelper.IncludeEGammaTriggerContent = True
    TLA0SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    TLA0SlimmingHelper.IncludeTauTriggerContent = False
    TLA0SlimmingHelper.IncludeEtMissTriggerContent = False
    TLA0SlimmingHelper.IncludeBJetTriggerContent = True
    TLA0SlimmingHelper.IncludeBPhysTriggerContent = False
    TLA0SlimmingHelper.IncludeMinBiasTriggerContent = False
    TLA0SlimmingHelper.OverrideJetTriggerContentWithTLAContent = True

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TLA0SlimmingHelper, 
                                        OutputContainerPrefix = "TrigMatch_", 
                                        TriggerList = TLA0TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TLA0SlimmingHelper, 
                                        OutputContainerPrefix = "TrigMatch_",
                                        TriggerList = TLA0TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if ConfigFlags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TLA0SlimmingHelper)        

    # Output stream    
    TLA0ItemList = TLA0SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_TLA0", ItemList=TLA0ItemList, AcceptAlgs=["TLA0Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_TLA0", AcceptAlgs=["TLA0Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

