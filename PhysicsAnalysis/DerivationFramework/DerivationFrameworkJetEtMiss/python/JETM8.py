# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM8.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Main algorithm config
def JETM8SkimmingToolCfg(ConfigFlags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    jetSelection = '(count( AntiKt10LCTopoJets.pt > 150.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1)'
    jetSelection += '||(count( AntiKt10UFOCSSKJets.pt > 150.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >=1)'

    JETM8OfflineSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool( name = "JETM8OfflineSkimmingTool",
                                                                                       expression = jetSelection)

    acc.addPublicTool(JETM8OfflineSkimmingTool, primary = True)

    return(acc)


# Main algorithm config
def JETM8KernelCfg(ConfigFlags, name='JETM8Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM8"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("JETM8CommonKernel", AugmentationTools = []))

    # Skimming
    skimmingTool = acc.getPrimaryAndMerge(JETM8SkimmingToolCfg(ConfigFlags))

    # Thinning tools...
    JETM8AKt10CCThinningTool = CompFactory.DerivationFramework.JetCaloClusterThinning(name                  = "JETM8AKt10CCThinningTool",
                                                                                      StreamName            = kwargs['StreamName'],
                                                                                      SGKey                 = "AntiKt10LCTopoJets",
                                                                                      SelectionString       = "(AntiKt10LCTopoJets.pt > 150*GeV && abs(AntiKt10LCTopoJets.eta) < 2.8)",
                                                                                      TopoClCollectionSGKey = "CaloCalTopoClusters",
                                                                                      AdditionalClustersKey = ["LCOriginTopoClusters"])

    acc.addPublicTool(JETM8AKt10CCThinningTool)

    JETM8CSSKUFOTPThinningTool = CompFactory.DerivationFramework.UFOTrackParticleThinning(name                   = "JETM8CSSKUFOTPThinningTool",
                                                                                          StreamName             = kwargs['StreamName'],
                                                                                          JetKey                 = "AntiKt10UFOCSSKJets",
                                                                                          UFOKey                 = "UFOCSSK",
                                                                                          InDetTrackParticlesKey = "InDetTrackParticles",
                                                                                          ThinTrackingContainer  = False,
                                                                                          PFOCollectionSGKey     = "Global",
                                                                                          AdditionalPFOKey       = ["CHSG","CSSKG"])

    acc.addPublicTool(JETM8CSSKUFOTPThinningTool)


    # Finally the kernel itself
    thinningTools = [JETM8AKt10CCThinningTool, JETM8CSSKUFOTPThinningTool]

    if ConfigFlags.Input.isMC:
        JETM8TruthJetInputThin = CompFactory.DerivationFramework.ViewContainerThinning( name = "JETM8ViewContThinning",
                                                                                        StreamName = kwargs['StreamName'],
                                                                                        TruthParticleKey = "TruthParticles",
                                                                                        TruthParticleViewKey = "JetInputTruthParticles")

        acc.addPublicTool(JETM8TruthJetInputThin)
        thinningTools.append(JETM8TruthJetInputThin)

    # Finally the kernel itself 
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools = [skimmingTool]))

    
    # Extra jet content:
    acc.merge(JETM8ExtraContentCfg(ConfigFlags))

    return acc


def JETM8ExtraContentCfg(ConfigFlags):

    acc = ComponentAccumulator()

    #=======================================
    # More detailed truth information
    #=======================================

    if ConfigFlags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg, AddTruthCollectionNavigationDecorationsCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg())
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(TruthCollections=["TruthTopQuarkWithDecayParticles","TruthBosonsWithDecayParticles"],prefix='Top'))

    return acc


def JETM8Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM8TriggerListsHelper = TriggerListsHelper()

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM8KernelCfg(ConfigFlags, name="JETM8Kernel", StreamName = 'StreamDAOD_JETM8', TriggerListsHelper = JETM8TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM8SlimmingHelper = SlimmingHelper("JETM8SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections)

    JETM8SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons", "Photons", "Muons",
                                            "InDetTrackParticles", "PrimaryVertices",
                                            "AntiKt4TruthJets",
                                            "AntiKt4TruthWZJets",
                                            "AntiKt4EMPFlowJets",
                                            "AntiKt10TruthJets",
                                            "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
                                            "AntiKt10TruthSoftDropBeta100Zcut10Jets",
                                            "AntiKt10LCTopoJets",
                                            "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                            "AntiKt10UFOCSSKJets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                            "BTagging_AntiKt4EMPFlow",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                        ]

    JETM8SlimmingHelper.AllVariables = ["CaloCalTopoClusters", "CaloCalFwdTopoTowers",
                                        "UFOCSSK",
                                        "TruthParticles"]

    JETM8SlimmingHelper.ExtraVariables += ['AntiKt10LCTopoJets.SizeParameter.GhostTrack',
                                           'AntiKt10TruthJets.SizeParameter.Split12.Split23',
                                           'AntiKt10UFOCSSKJets.SizeParameter.GhostTrack',
                                           'AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets.SizeParameter.GhostTrack',
                                           'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.SizeParameter.GhostTrack',
                                           'AntiKt10TruthTrimmedPtFrac5SmallR20Jets.SizeParameter',
                                           'AntiKt10TruthSoftDropBeta100Zcut10Jets.SizeParameter',
                                       ]

    JETM8SlimmingHelper.ExtraVariables.append('GlobalChargedParticleFlowObjects.chargedObjectLinks')
    JETM8SlimmingHelper.ExtraVariables.append('GlobalNeutralParticleFlowObjects.chargedObjectLinks')
    JETM8SlimmingHelper.ExtraVariables.append('CSSKGChargedParticleFlowObjects.pt.eta.phi.m.matchedToPV.originalObjectLink')
    JETM8SlimmingHelper.ExtraVariables.append('CSSKGNeutralParticleFlowObjects.pt.eta.phi.m.originalObjectLink')

    # Truth containers
    if ConfigFlags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM8SlimmingHelper)

        JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticles'] = 'xAOD::TruthParticleContainer'
        JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
        JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVertices'] = 'xAOD::TruthVertexContainer'
        JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVerticesAux'] = 'xAOD::TruthVertexAuxContainer'
        JETM8SlimmingHelper.AppendToDictionary['TruthParticles'] = 'xAOD::TruthParticleContainer'
        JETM8SlimmingHelper.AppendToDictionary['TruthParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
        
        JETM8SlimmingHelper.AllVariables += ["TruthEvents", "TruthParticles", "TruthTopQuarkWithDecayParticles", "TruthTopQuarkWithDecayVertices","TruthHFWithDecayParticles"]

    # Low-level inputs: 

    from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
    addOriginCorrectedClustersToSlimmingTool(JETM8SlimmingHelper,writeLC=True,writeEM=True)

    JETM8SlimmingHelper.AppendToDictionary['UFOCSSK'] = 'xAOD::FlowElementContainer'
    JETM8SlimmingHelper.AppendToDictionary['UFOCSSKAux'] = 'xAOD::FlowElementAuxContainer'

    JETM8SlimmingHelper.AppendToDictionary["GlobalChargedParticleFlowObjects"]='xAOD::FlowElementContainer'
    JETM8SlimmingHelper.AppendToDictionary["GlobalChargedParticleFlowObjectsAux"]='xAOD::FlowElementAuxContainer'

    JETM8SlimmingHelper.AppendToDictionary["GlobalNeutralParticleFlowObjects"]='xAOD::FlowElementContainer'
    JETM8SlimmingHelper.AppendToDictionary["GlobalNeutralParticleFlowObjectsAux"]='xAOD::FlowElementAuxContainer'

    JETM8SlimmingHelper.AppendToDictionary["CSSKGChargedParticleFlowObjects"] = 'xAOD::FlowElementContainer'
    JETM8SlimmingHelper.AppendToDictionary["CSSKGChargedParticleFlowObjectsAux"] = 'xAOD::FlowElementAuxContainer'

    JETM8SlimmingHelper.AppendToDictionary["CSSKGNeutralParticleFlowObjects"] = 'xAOD::FlowElementContainer'
    JETM8SlimmingHelper.AppendToDictionary["CSSKGNeutralParticleFlowObjectsAux"] = 'xAOD::FlowElementAuxContainer'


    # Trigger content
    JETM8SlimmingHelper.IncludeTriggerNavigation = False
    JETM8SlimmingHelper.IncludeJetTriggerContent = False
    JETM8SlimmingHelper.IncludeMuonTriggerContent = False
    JETM8SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM8SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM8SlimmingHelper.IncludeTauTriggerContent = False
    JETM8SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM8SlimmingHelper.IncludeBJetTriggerContent = False
    JETM8SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM8SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM8SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM8TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM8SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM8TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if ConfigFlags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(JETM8SlimmingHelper)
        # Run 2 is added here temporarily to allow testing/comparison/debugging
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM8SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM8TriggerListsHelper.Run3TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = JETM8SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = JETM8TriggerListsHelper.Run3TriggerNamesNoTau)

    # Output stream    
    JETM8ItemList = JETM8SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_JETM8", ItemList=JETM8ItemList, AcceptAlgs=["JETM8Kernel"]))

    return acc

