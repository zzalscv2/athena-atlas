# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM2.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM2KernelCfg(ConfigFlags, name='JETM2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM2"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    if ConfigFlags.Input.isMC:
        # thinning tools: 
        truthThinningTool = CompFactory.DerivationFramework.MenuTruthThinning(name               = "JETM2TruthThinning",
                                                                              StreamName         = kwargs['StreamName'],
                                                                              WriteAllStable     = True,
                                                                              WritePartons       = False,
                                                                              WriteHadrons       = False,
                                                                              WriteBHadrons      = True,
                                                                              WriteCHadrons      = False,
                                                                              WriteGeant         = False,
                                                                              WriteFirstN        = 10)

        acc.addPublicTool(truthThinningTool)

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = [truthThinningTool] if ConfigFlags.Input.isMC else []))

    # Extra jet content:
    acc.merge(JETM2ExtraContentCfg(ConfigFlags))

    return acc


def JETM2ExtraContentCfg(ConfigFlags):

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg, getInputAlgs, getConstitPJGAlg
    from JetRecConfig.JetConfigFlags import jetInternalFlags
    from JetRecConfig.JetInputConfig import buildEventShapeAlg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSKNoPtCut

    #=======================================
    # CHS R = 0.4 UFO jets
    #=======================================

    algs = getInputAlgs(cst.UFO, flags=ConfigFlags)
    for alg in algs:
        if isinstance(alg, ComponentAccumulator):
            acc.merge(alg)
        else:
            acc.addEventAlgo(alg)

    #=======================================
    # CSSK R = 0.4 UFO jets
    #=======================================
    jetList = [AntiKt4UFOCSSKNoPtCut]

    jetInternalFlags.isRecoJob = True

    for jd in jetList:
        acc.merge(JetRecCfg(ConfigFlags,jd))

    #=======================================
    # UFO CSSK event shape 
    #=======================================

    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK,'', suffix=None))
    acc.addEventAlgo(getConstitPJGAlg(cst.UFOCSSK, suffix='Neut'))
    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK,'', suffix='Neut'))

    #=======================================
    # More detailed truth information
    #=======================================

    if ConfigFlags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg, AddTruthCollectionNavigationDecorationsCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(ConfigFlags))
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(ConfigFlags, TruthCollections=["TruthTopQuarkWithDecayParticles","TruthBosonsWithDecayParticles"],prefix='Top'))


    return acc


def JETM2Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM2TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM2KernelCfg(ConfigFlags, name="JETM2Kernel", StreamName = 'StreamDAOD_JETM2', TriggerListsHelper = JETM2TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM2SlimmingHelper = SlimmingHelper("JETM2SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)

    JETM2SlimmingHelper.SmartCollections = ["EventInfo","InDetTrackParticles", "PrimaryVertices",
                                            "Electrons", "Photons", "Muons", "TauJets",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "AntiKt4EMTopoJets","AntiKt4EMPFlowJets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                            "BTagging_AntiKt4EMPFlow",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track",

    ]

    JETM2SlimmingHelper.AllVariables = ["CaloCalTopoClusters", "CaloCalFwdTopoTowers",
                                        "GlobalChargedParticleFlowObjects", "GlobalNeutralParticleFlowObjects",
                                        "CHSGChargedParticleFlowObjects","CHSGNeutralParticleFlowObjects",
                                        "CSSKGChargedParticleFlowObjects","CSSKGNeutralParticleFlowObjects",
                                        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape",
                                        "Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape"]

    JETM2SlimmingHelper.ExtraVariables = ["AntiKt4EMPFlowJets.GhostTower",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.SizeParameter",
                                          "UFOCSSK.pt.eta.phi.m.signalType.otherObjectLinks.chargedObjectLinks",
                                          "UFO.pt.eta.phi.m.signalType.otherObjectLinks.chargedObjectLinks",
                                          "InDetTrackParticles.particleHypothesis.vx.vy.vz.btagIp_d0Uncertainty.btagIp_z0SinThetaUncertainty.btagIp_z0SinTheta.btagIp_trackMomentum.btagIp_trackDisplacement.btagIp_invalidIp",
                                          "GSFTrackParticles.particleHypothesis.vx.vy.vz",
                                          "PrimaryVertices.x.y.z.covariance.trackWeights",
                                          "TauJets.clusterLinks",
                                          "Electrons.neutralGlobalFELinks.chargedGlobalFELinks",
                                          "Photons.neutralGlobalFELinks",
                                          "Muons.energyLossType.EnergyLoss.ParamEnergyLoss.MeasEnergyLoss.EnergyLossSigma.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.clusterLinks.FSR_CandidateEnergy.neutralGlobalFELinks.chargedGlobalFELinks",
                                          "MuonSegments.x.y.z.px.py.pz"]

    JETM2SlimmingHelper.AppendToDictionary.update({'CSSKGNeutralParticleFlowObjects': 'xAOD::FlowElementContainer',
                                                   'CSSKGNeutralParticleFlowObjectsAux': 'xAOD::ShallowAuxContainer',
                                                   'CSSKGChargedParticleFlowObjects': 'xAOD::FlowElementContainer',
                                                   'CSSKGChargedParticleFlowObjectsAux': 'xAOD::ShallowAuxContainer',
                                                   'UFO': 'xAOD::FlowElementContainer',
                                                   'UFOAux': 'xAOD::FlowElementAuxContainer',
                                                   'Kt4UFOCSSKEventShape': 'xAOD::EventShape',
                                                   'Kt4UFOCSSKEventShapeAux': 'xAOD::EventShapeAuxInfo',
                                                   'Kt4UFOCSSKNeutEventShape': 'xAOD::EventShape',
                                                   'Kt4UFOCSSKNeutEventShapeAux': 'xAOD::EventShapeAuxInfo'})

    from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
    addOriginCorrectedClustersToSlimmingTool(JETM2SlimmingHelper,writeLC=True,writeEM=True)

    # Truth containers
    if ConfigFlags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM2SlimmingHelper)

        JETM2SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})
        
        JETM2SlimmingHelper.AllVariables += ["TruthTopQuarkWithDecayParticles","TruthTopQuarkWithDecayVertices","TruthHFWithDecayParticles",
                                             "AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", "TruthParticles", "TruthVertices","TruthEvents"]
        JETM2SlimmingHelper.ExtraVariables += ["AntiKt10TruthSoftDropBeta100Zcut10Jets.SizeParameter"]
        JETM2SlimmingHelper.SmartCollections += ["AntiKt4TruthJets","AntiKt10TruthJets","AntiKt4TruthWZJets"]

    # Trigger content
    JETM2SlimmingHelper.IncludeTriggerNavigation = False
    JETM2SlimmingHelper.IncludeJetTriggerContent = False
    JETM2SlimmingHelper.IncludeMuonTriggerContent = False
    JETM2SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM2SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM2SlimmingHelper.IncludeTauTriggerContent = False
    JETM2SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM2SlimmingHelper.IncludeBJetTriggerContent = False
    JETM2SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM2SlimmingHelper.IncludeMinBiasTriggerContent = False

    jetOutputList = ["AntiKt4UFOCSSKNoPtCutJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(JETM2SlimmingHelper, jetOutputList, JETM2SlimmingHelper.SmartCollections)

    # Output stream
    JETM2ItemList = JETM2SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_JETM2", ItemList=JETM2ItemList, AcceptAlgs=["JETM2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_JETM2", AcceptAlgs=["JETM2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

