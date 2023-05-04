# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_JETM12.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM12SkimmingToolCfg(ConfigFlags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    metTriggers = TriggerLists.MET_Trig(ConfigFlags)
    muTriggers = TriggerLists.single_mu_Trig(ConfigFlags)
    orstr  = ' || '
    andstr = ' && '
    trackRequirements = '(InDetTrackParticles.pt > 10.*GeV && InDetTrackParticles.TrkIsoPt1000_ptcone20 < 0.12*InDetTrackParticles.pt && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV) < 5.0*mm )'
    trackRequirementsMu = '(InDetTrackParticles.pt > 70.*GeV && InDetTrackParticles.TrkIsoPt1000_ptcone20 < 0.12*InDetTrackParticles.pt && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV) < 5.0*mm )'
    trackRequirementsTtbar = '(InDetTrackParticles.pt > 25.*GeV && InDetTrackParticles.TrkIsoPt1000_ptcone20 < 0.12*InDetTrackParticles.pt && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV) < 5.0*mm )'

    #Previously used also b-tagging criteria for slimming, to be checked in r22
    jetRequirementsTtbar = '( AntiKt4EMTopoJets.pt > 20*GeV )'
    expressionW = '( (' + orstr.join(metTriggers) + ' )' + andstr + '( count('+trackRequirements+') >=1 ) )'
    expressionMu = '( (' + orstr.join(muTriggers) + ' )' + andstr + '( count('+trackRequirementsMu+') >=1 ) )'
    expressionTtbar = '( (' + orstr.join(muTriggers) + ' )' + andstr + '( count('+trackRequirementsTtbar+') >=1 )' + andstr + '( count('+trackRequirements+') >=2 )' + andstr + '( count('+jetRequirementsTtbar+') >=1 ) )'
    expression = '( '+expressionW+' || '+expressionMu+' || '+expressionTtbar+' )'

    JETM12OfflineSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "JETM12SkimmingTool",
                                                                                       expression = expression)

    acc.addPublicTool(JETM12OfflineSkimmingTool, primary=True)

    return(acc)

def JETM12AugmentationToolsForSkimmingCfg(ConfigFlags):
    """Configure the augmentation tool for skimming"""
    acc = ComponentAccumulator()

    toolkwargs = {}
    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_Loose_Cfg
    toolkwargs["TrackSelectionTool"] = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(ConfigFlags, 
                                                                                              name = "TrackSelectionTool1000_JETM12", 
                                                                                              maxZ0SinTheta = 3.0,
                                                                                              minPt = 1000.))

    

    TrackIsoTool = CompFactory.xAOD.TrackIsolationTool(**toolkwargs)
    acc.addPublicTool(TrackIsoTool)

    from xAODPrimitives.xAODIso import xAODIso as isoPar
    Pt1000IsoTrackDecorator = CompFactory.DerivationFramework.trackIsolationDecorator(name = "Pt1000IsoTrackDecorator",
                                                                                      TrackIsolationTool = TrackIsoTool,
                                                                                      TargetContainer = "InDetTrackParticles",
                                                                                      ptcones = [isoPar.ptcone40,isoPar.ptcone30,isoPar.ptcone20],
                                                                                      Prefix = 'TrkIsoPt1000_')

    acc.addPublicTool(Pt1000IsoTrackDecorator, primary=True)

    return(acc)

def JETM12AugmentationToolsCfg(ConfigFlags):
    """Configure the augmentation tool"""
    acc = ComponentAccumulator()

    toolkwargs = {}
    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_Loose_Cfg
    toolkwargs["TrackSelectionTool"] = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(ConfigFlags,
                                                                                              name = "TrackSelectionTool500_JETM12",
                                                                                              maxZ0SinTheta = 3.0,
                                                                                              minPt = 500.))
    toolkwargs["name"] = "TrackIsolationToolPt500"
    TrackIsoTool = CompFactory.xAOD.TrackIsolationTool(**toolkwargs)
    acc.addPublicTool(TrackIsoTool)

    from xAODPrimitives.xAODIso import xAODIso as isoPar
    Pt500IsoTrackDecorator = CompFactory.DerivationFramework.trackIsolationDecorator(name = "Pt500IsoTrackDecorator",
                                                                                      TrackIsolationTool = TrackIsoTool,
                                                                                      TargetContainer = "InDetTrackParticles",
                                                                                      ptcones = [isoPar.ptcone40,isoPar.ptcone30,isoPar.ptcone20],
                                                                                      Prefix = 'TrkIsoPt500_')

    acc.addPublicTool(Pt500IsoTrackDecorator, primary=True)
    
    return(acc)

# Main algorithm config
def JETM12KernelCfg(ConfigFlags, name='JETM12Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM12"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    
    #Pre-selection kernel
    from AthenaCommon.CFElements import seqAND
    acc.addSequence( seqAND("JETM12Sequence") )
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    skimmingTool = acc.getPrimaryAndMerge(JETM12SkimmingToolCfg(ConfigFlags))
    augmentationToolSkim = acc.getPrimaryAndMerge(JETM12AugmentationToolsForSkimmingCfg(ConfigFlags))
    #acc.merge(JETM12AugmentationToolsForSkimmingCfg(ConfigFlags))
    skimmingKernel = DerivationKernel(kwargs["PreselectionName"], SkimmingTools = [skimmingTool], AugmentationTools = [augmentationToolSkim])
    acc.addEventAlgo( skimmingKernel, sequenceName="JETM12Sequence" ) 

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, TauTrackParticleThinningCfg

    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    JETM12_thinning_expression = "( InDetTrackParticles.pt > 10*GeV && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV) < 5.0*mm )"
    JETM12TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        ConfigFlags,
        name                    = "JETM12TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'], 
        SelectionString         = JETM12_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with muons
    JETM12MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "JETM12MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    JETM12ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "JETM12ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with photons
    JETM12PhotonTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        ConfigFlags,
        name                     = "JETM12PhotonTPThinningTool",
        StreamName               = kwargs['StreamName'],
        SGKey                    = "Photons",
        InDetTrackParticlesKey   = "InDetTrackParticles",
        GSFConversionVerticesKey = "GSFConversionVertices"))

    # Include inner detector tracks associated with taus
    JETM12TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        ConfigFlags,
        name                   = "JETM12TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))

    thinningTools = [JETM12TrackParticleThinningTool,
                     JETM12MuonTPThinningTool,
                     JETM12ElectronTPThinningTool,
                     JETM12PhotonTPThinningTool,
                     JETM12TauTPThinningTool]

    #CaloClusterThinning does not support the usage of InDetTrackParticles at the moment, needs to be migrated to R22
    #from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    #selectionString = "( InDetTrackParticles.pt > 10*GeV && InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV) < 5.0*mm )"
    #JETM12CaloThinningTool = acc.getPrimaryAndMerge(CaloClusterThinningCfg(ConfigFlags,
    #                                                                       name                  = "JETM12CaloClusterThinning",
    #                                                                       StreamName            = kwargs['StreamName'],
    #                                                                       SGKey                 = "InDetTrackParticles",
    #                                                                       TopoClCollectionSGKey = "CaloCalTopoClusters",
    #                                                                       SelectionString = selectionString,
    #                                                                       ConeSize = 0.6))
    #thinningTools.append(JETM12CaloThinningTool)

    if ConfigFlags.Input.isMC:
        truth_cond_status    = "( (TruthParticles.status == 1) && (TruthParticles.barcode < 200000) && (TruthParticles.pt > 8*GeV) )"       # high pt pions for E/p
        truth_cond_Lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16) && (TruthParticles.barcode < 200000))" # Leptons
        truth_expression = '('+truth_cond_status+' || '+truth_cond_Lepton +')'

        JETM12TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(name = "JETM12TruthThinningTool",
                                                                                       StreamName              = kwargs['StreamName'],
                                                                                       ParticleSelectionString = truth_expression,
                                                                                       PreserveDescendants     = False,
                                                                                       PreserveGeneratorDescendants = True,
                                                                                       PreserveAncestors = False)

        acc.addPublicTool(JETM12TruthThinningTool)
        thinningTools.append(JETM12TruthThinningTool)

    # augmentation tool
    augmentationTool = acc.getPrimaryAndMerge(JETM12AugmentationToolsCfg(ConfigFlags))

    # Main kernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      AugmentationTools = [augmentationTool]),
                     sequenceName="JETM12Sequence")
    
    return acc

def JETM12Cfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM12TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM12KernelCfg(ConfigFlags, name="JETM12Kernel", PreselectionName="JETM12PreselectionKernel", StreamName = 'StreamDAOD_JETM12', TriggerListsHelper = JETM12TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM12SlimmingHelper = SlimmingHelper("JETM12SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)

    JETM12SlimmingHelper.SmartCollections = ["EventInfo",
                                             "Electrons", "Photons", "Muons", "TauJets",
                                             "InDetTrackParticles", "PrimaryVertices",
                                             "MET_Baseline_AntiKt4EMTopo",
                                             "MET_Baseline_AntiKt4EMPFlow",
                                             "AntiKt4EMTopoJets","AntiKt4EMPFlowJets",
                                             "BTagging_AntiKt4EMPFlow"]

    JETM12SlimmingHelper.AllVariables = ["MuonSegments","InDetTrackParticles",
                                         "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","CaloCalTopoClusters"]

    JETM12SlimmingHelper.ExtraVariables = ["InDetTrackParticles.TrkIsoPt1000_ptcone40.TrkIsoPt1000_ptcone30.TrkIsoPt1000_ptcone20.TrkIsoPt500_ptcone40.TrkIsoPt500_ptcone30.TrkIsoPt500_ptcone20"]

    if ConfigFlags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM12SlimmingHelper)

        JETM12SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})

        JETM12SlimmingHelper.SmartCollections += ["AntiKt4TruthJets"]
        JETM12SlimmingHelper.AllVariables += ["MuonTruthParticles", "TruthParticles", "TruthVertices"]

    # Trigger content
    JETM12SlimmingHelper.IncludeTriggerNavigation = False
    JETM12SlimmingHelper.IncludeJetTriggerContent = False
    JETM12SlimmingHelper.IncludeMuonTriggerContent = False
    JETM12SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM12SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM12SlimmingHelper.IncludeTauTriggerContent = False
    JETM12SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM12SlimmingHelper.IncludeBJetTriggerContent = False
    JETM12SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM12SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Output stream    
    JETM12ItemList = JETM12SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_JETM12", ItemList=JETM12ItemList, AcceptAlgs=["JETM12Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_JETM12", AcceptAlgs=["JETM12Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

