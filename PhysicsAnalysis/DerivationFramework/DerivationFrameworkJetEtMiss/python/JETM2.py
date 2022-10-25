# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM2.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

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

    algs = getInputAlgs(cst.UFO, configFlags=ConfigFlags)
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
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg())
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(TruthCollections=["TruthTopQuarkWithDecayParticles","TruthBosonsWithDecayParticles"],prefix='Top'))


    return acc


def JETM2Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM2TriggerListsHelper = TriggerListsHelper()

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM2KernelCfg(ConfigFlags, name="JETM2Kernel", StreamName = 'StreamDAOD_JETM2', TriggerListsHelper = JETM2TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM2SlimmingHelper = SlimmingHelper("JETM2SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections)

    JETM2SlimmingHelper.SmartCollections = ["EventInfo",
                                             "Electrons", "Photons", "Muons", "TauJets",
                                             "InDetTrackParticles", "PrimaryVertices",
                                             "MET_Baseline_AntiKt4EMPFlow",
                                             "AntiKt4EMTopoJets","AntiKt4EMPFlowJets",
                                             "AntiKt4TruthJets","AntiKt10TruthJets",
                                             
                                             "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                             "BTagging_AntiKt4EMPFlow",
                                             "BTagging_AntiKtVR30Rmax4Rmin02Track",

    ]

    JETM2SlimmingHelper.AllVariables = ["CaloCalTopoClusters", "CaloCalFwdTopoTowers",
                                         "GlobalChargedParticleFlowObjects", "GlobalNeutralParticleFlowObjects",
                                         "CHSGChargedParticleFlowObjects","CHSGNeutralParticleFlowObjects",
                                         "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape",
                                         "TruthParticles",
                                         "TruthVertices",
                                         "TruthEvents"]

    JETM2SlimmingHelper.ExtraVariables = ["UFOCSSK.pt.eta.phi.m.signalType",
                                           "UFO.pt.eta.phi.m.signalType",
                                           "InDetTrackParticles.particleHypothesis.vx.vy.vz",
                                           "GSFTrackParticles.particleHypothesis.vx.vy.vz",
                                           "PrimaryVertices.x.y.z",
                                           "TauJets.clusterLinks",
                                           "Muons.energyLossType.EnergyLoss.ParamEnergyLoss.MeasEnergyLoss.EnergyLossSigma.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.clusterLinks.FSR_CandidateEnergy",
                                           "MuonSegments.x.y.z.px.py.pz",
                                           "CSSKGChargedParticleFlowObjects.pt.eta.phi.m.matchedToPV.originalObjectLink",
                                           "CSSKGNeutralParticleFlowObjects.pt.eta.phi.m.originalObjectLink"]


    JETM2SlimmingHelper.AppendToDictionary['GlobalChargedParticleFlowObjects'] ='xAOD::FlowElementContainer'
    JETM2SlimmingHelper.AppendToDictionary['GlobalChargedParticleFlowObjectsAux'] ='xAOD::FlowElementAuxContainer'
    JETM2SlimmingHelper.AppendToDictionary['GlobalNeutralParticleFlowObjects'] = 'xAOD::FlowElementContainer'
    JETM2SlimmingHelper.AppendToDictionary['GlobalNeutralParticleFlowObjectsAux'] = 'xAOD::FlowElementAuxContainer'
    JETM2SlimmingHelper.AppendToDictionary['CHSGChargedParticleFlowObjects'] = 'xAOD::FlowElementContainer'
    JETM2SlimmingHelper.AppendToDictionary['CHSGChargedParticleFlowObjectsAux'] = 'xAOD::ShallowAuxContainer'
    JETM2SlimmingHelper.AppendToDictionary['CHSGNeutralParticleFlowObjects'] = 'xAOD::FlowElementContainer'
    JETM2SlimmingHelper.AppendToDictionary['CHSGNeutralParticleFlowObjectsAux'] = 'xAOD::ShallowAuxContainer'
    JETM2SlimmingHelper.AppendToDictionary["CSSKGNeutralParticleFlowObjects"]='xAOD::FlowElementContainer'
    JETM2SlimmingHelper.AppendToDictionary["CSSKGNeutralParticleFlowObjectsAux"]='xAOD::ShallowAuxContainer'
    JETM2SlimmingHelper.AppendToDictionary['UFOCSSK'] = 'xAOD::FlowElementContainer'
    JETM2SlimmingHelper.AppendToDictionary['UFOCSSKAux'] = 'xAOD::FlowElementAuxContainer'
    JETM2SlimmingHelper.AppendToDictionary['UFO'] = 'xAOD::FlowElementContainer'
    JETM2SlimmingHelper.AppendToDictionary['UFOAux'] = 'xAOD::FlowElementAuxContainer'
    JETM2SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShape'] = 'xAOD::EventShape'
    JETM2SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShapeAux'] = 'xAOD::EventShapeAuxInfo'
    JETM2SlimmingHelper.AppendToDictionary['Kt4UFOCSSKNeutEventShape'] = 'xAOD::EventShape'
    JETM2SlimmingHelper.AppendToDictionary['Kt4UFOCSSKNeutEventShapeAux'] = 'xAOD::EventShapeAuxInfo'

    from DerivationFrameworkJetEtMiss.JetCommonConfig import addOriginCorrectedClustersToSlimmingTool
    addOriginCorrectedClustersToSlimmingTool(JETM2SlimmingHelper,writeLC=True,writeEM=True)

    # Truth containers
    if ConfigFlags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM2SlimmingHelper)

        JETM2SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticles'] = 'xAOD::TruthParticleContainer'
        JETM2SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
        JETM2SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVertices'] = 'xAOD::TruthVertexContainer'
        JETM2SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVerticesAux'] = 'xAOD::TruthVertexAuxContainer'
        JETM2SlimmingHelper.AppendToDictionary['TruthParticles'] = 'xAOD::TruthParticleContainer'
        JETM2SlimmingHelper.AppendToDictionary['TruthParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
        
        JETM2SlimmingHelper.AllVariables += ["TruthTopQuarkWithDecayParticles","TruthTopQuarkWithDecayVertices","TruthHFWithDecayParticles"]
        JETM2SlimmingHelper.AllVariables += ["AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", "TruthParticles"]
        JETM2SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]

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

    return acc

