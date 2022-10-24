# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM13.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Main algorithm config
def JETM13KernelCfg(ConfigFlags, name='JETM13Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM13"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    if ConfigFlags.Input.isMC:
        # thinning tools: 
        truthThinningTool = CompFactory.DerivationFramework.MenuTruthThinning(name               = "JETM13TruthThinning",
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
    acc.merge(JETM13ExtraContentCfg(ConfigFlags))

    return acc


def JETM13ExtraContentCfg(ConfigFlags):

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
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(generations=4,rejectHadronChildren=True))

    return acc


def JETM13Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM13TriggerListsHelper = TriggerListsHelper()

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM13KernelCfg(ConfigFlags, name="JETM13Kernel", StreamName = 'StreamDAOD_JETM13', TriggerListsHelper = JETM13TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM13SlimmingHelper = SlimmingHelper("JETM13SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections)

    JETM13SlimmingHelper.SmartCollections = ["EventInfo",
                                             "Electrons", "Photons", "Muons", "TauJets",
                                             "InDetTrackParticles", "PrimaryVertices",
                                             "MET_Baseline_AntiKt4EMPFlow",
                                             "AntiKt4EMTopoJets","AntiKt4EMPFlowJets",
                                             "AntiKt4TruthJets","AntiKt10TruthJets",
                                             "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"]

    JETM13SlimmingHelper.AllVariables = ["CaloCalTopoClusters", "CaloCalFwdTopoTowers",
                                         "GlobalChargedParticleFlowObjects", "GlobalNeutralParticleFlowObjects",
                                         "CHSGChargedParticleFlowObjects","CHSGNeutralParticleFlowObjects",
                                         "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape",
                                         "TruthParticles",
                                         "TruthVertices",
                                         "TruthEvents"]

    JETM13SlimmingHelper.ExtraVariables = ["UFOCSSK.pt.eta.phi.m.signalType",
                                           "UFO.pt.eta.phi.m.signalType",
                                           "InDetTrackParticles.particleHypothesis.vx.vy.vz",
                                           "GSFTrackParticles.particleHypothesis.vx.vy.vz",
                                           "PrimaryVertices.x.y.z",
                                           "TauJets.clusterLinks",
                                           "Muons.energyLossType.EnergyLoss.ParamEnergyLoss.MeasEnergyLoss.EnergyLossSigma.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.clusterLinks.FSR_CandidateEnergy",
                                           "MuonSegments.x.y.z.px.py.pz"]

    JETM13SlimmingHelper.AppendToDictionary['GlobalChargedParticleFlowObjects'] ='xAOD::FlowElementContainer'
    JETM13SlimmingHelper.AppendToDictionary['GlobalChargedParticleFlowObjectsAux'] ='xAOD::FlowElementAuxContainer'
    JETM13SlimmingHelper.AppendToDictionary['GlobalNeutralParticleFlowObjects'] = 'xAOD::FlowElementContainer'
    JETM13SlimmingHelper.AppendToDictionary['GlobalNeutralParticleFlowObjectsAux'] = 'xAOD::FlowElementAuxContainer'
    JETM13SlimmingHelper.AppendToDictionary['CHSGChargedParticleFlowObjects'] = 'xAOD::FlowElementContainer'
    JETM13SlimmingHelper.AppendToDictionary['CHSGChargedParticleFlowObjectsAux'] = 'xAOD::ShallowAuxContainer'
    JETM13SlimmingHelper.AppendToDictionary['CHSGNeutralParticleFlowObjects'] = 'xAOD::FlowElementContainer'
    JETM13SlimmingHelper.AppendToDictionary['CHSGNeutralParticleFlowObjectsAux'] = 'xAOD::ShallowAuxContainer'
    JETM13SlimmingHelper.AppendToDictionary['UFOCSSK'] = 'xAOD::FlowElementContainer'
    JETM13SlimmingHelper.AppendToDictionary['UFOCSSKAux'] = 'xAOD::FlowElementAuxContainer'
    JETM13SlimmingHelper.AppendToDictionary['UFO'] = 'xAOD::FlowElementContainer'
    JETM13SlimmingHelper.AppendToDictionary['UFOAux'] = 'xAOD::FlowElementAuxContainer'
    JETM13SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShape'] = 'xAOD::EventShape'
    JETM13SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShapeAux'] = 'xAOD::EventShapeAuxInfo'
    JETM13SlimmingHelper.AppendToDictionary['Kt4UFOCSSKNeutEventShape'] = 'xAOD::EventShape'
    JETM13SlimmingHelper.AppendToDictionary['Kt4UFOCSSKNeutEventShapeAux'] = 'xAOD::EventShapeAuxInfo'

    # Truth containers
    if ConfigFlags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM13SlimmingHelper)

        JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticles'] = 'xAOD::TruthParticleContainer'
        JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
        JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVertices'] = 'xAOD::TruthVertexContainer'
        JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVerticesAux'] = 'xAOD::TruthVertexAuxContainer'
        JETM13SlimmingHelper.AppendToDictionary['TruthParticles'] = 'xAOD::TruthParticleContainer'
        JETM13SlimmingHelper.AppendToDictionary['TruthParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
        
        JETM13SlimmingHelper.AllVariables += ["TruthTopQuarkWithDecayParticles","TruthTopQuarkWithDecayVertices"]
        JETM13SlimmingHelper.AllVariables += ["AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", "TruthParticles"]
        JETM13SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]

    # Trigger content
    JETM13SlimmingHelper.IncludeTriggerNavigation = False
    JETM13SlimmingHelper.IncludeJetTriggerContent = False
    JETM13SlimmingHelper.IncludeMuonTriggerContent = False
    JETM13SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM13SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM13SlimmingHelper.IncludeTauTriggerContent = False
    JETM13SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM13SlimmingHelper.IncludeBJetTriggerContent = False
    JETM13SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM13SlimmingHelper.IncludeMinBiasTriggerContent = False

    jetOutputList = ["AntiKt4UFOCSSKNoPtCutJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(JETM13SlimmingHelper, jetOutputList, JETM13SlimmingHelper.SmartCollections)

    # Output stream    
    JETM13ItemList = JETM13SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_JETM13", ItemList=JETM13ItemList, AcceptAlgs=["JETM13Kernel"]))

    return acc

