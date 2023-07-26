# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_LLP1.py
# This defines DAOD_LLP1, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority
# of physics analyses in ATLAS.
# It requires the flag LLP1 in Derivation_tf.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, MetadataCategory

MergedElectronContainer = "StdWithLRTElectrons"
MergedMuonContainer = "StdWithLRTMuons"
MergedTrackCollection = "InDetWithLRTTrackParticles"
LLP1VrtSecInclusiveSuffixes = []

# Main algorithm config
def LLP1KernelCfg(ConfigFlags, name='LLP1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for LLP1"""
    acc = ComponentAccumulator()

    # Augmentations

    # LRT track merge
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleMergerCfg
    LLP1TrackParticleMergerTool = acc.getPrimaryAndMerge(TrackParticleMergerCfg(
        ConfigFlags,
        name                        = "LLP1TrackParticleMergerTool",
        TrackParticleLocation       = ["InDetTrackParticles", "InDetLargeD0TrackParticles"],
        OutputTrackParticleLocation = MergedTrackCollection,
        CreateViewColllection       = True))

    LRTMergeAug = CompFactory.DerivationFramework.CommonAugmentation("InDetLRTMerge", AugmentationTools = [LLP1TrackParticleMergerTool])
    acc.addEventAlgo(LRTMergeAug)

    # LRT muons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
    acc.merge(LRTMuonMergerAlg( ConfigFlags,
                                PromptMuonLocation    = "Muons",
                                LRTMuonLocation       = "MuonsLRT",
                                OutputMuonLocation    = MergedMuonContainer,
                                CreateViewCollection  = True,
                                UseRun3WP = ConfigFlags.GeoModel.Run == LHCPeriod.Run3))

    # LRT electrons merge
    from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronMergerAlg
    acc.merge(LRTElectronMergerAlg( ConfigFlags,
                                    PromptElectronLocation = "Electrons",
                                    LRTElectronLocation    = "LRTElectrons",
                                    OutputCollectionName   = MergedElectronContainer,
                                    isDAOD                 = False,
                                    CreateViewCollection   = True))

    # Max Cell sum decoration tool
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    acc.merge(LArOnOffIdMappingCfg(ConfigFlags))

    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import MaxCellDecoratorCfg

    LLP1MaxCellDecoratorTool = acc.popToolsAndMerge(MaxCellDecoratorCfg(
        ConfigFlags,
        name = "LLP1MaxCellDecoratorTool",
        SGKey_electrons = "Electrons",
        SGKey_photons   = "Photons"))
    acc.addPublicTool(LLP1MaxCellDecoratorTool)


    if ConfigFlags.GeoModel.Run == LHCPeriod.Run3:
        LLP1LRTMaxCellDecoratorTool = acc.popToolsAndMerge(MaxCellDecoratorCfg(
            ConfigFlags,
            name = "LLP1LRTMaxCellDecoratorTool",
            SGKey_electrons = "LRTElectrons"))
    else:
        LLP1LRTMaxCellDecoratorTool = acc.popToolsAndMerge(MaxCellDecoratorCfg(
            ConfigFlags,
            name = "LLP1LRTMaxCellDecoratorTool",
            SGKey_electrons = "LRTElectrons",
            SGKey_egammaClusters   = "egammaClusters"))
    acc.addPublicTool(LLP1LRTMaxCellDecoratorTool)

    augmentationTools = [ LLP1MaxCellDecoratorTool,
                          LLP1LRTMaxCellDecoratorTool ]

    # Reclustered jets definitions
    from JetRecConfig.JetRecConfig import registerAsInputConstit, JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4Truth, AntiKt4EMTopo
    from JetRecConfig.JetDefinition import JetDefinition
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.JetConfigFlags import jetInternalFlags

    jetInternalFlags.isRecoJob = True
    registerAsInputConstit(AntiKt4EMTopo)
    registerAsInputConstit(AntiKt4Truth)
    cst.AntiKt4EMTopoJets.label = "EMTopoRC"
    cst.AntiKt4TruthJets.label = "TruthRC"

    AntiKt10RCEMTopo = JetDefinition(   "AntiKt",1.0,cst.AntiKt4EMTopoJets,
                                        ghostdefs = ["Track", "TrackLRT", "LCTopo"],
                                        modifiers = ("Sort", "Filter:200000",),
                                        standardRecoMode = True,
                                        lock = True,
    )
    if ConfigFlags.Input.isMC:
        AntiKt10RCTruth = JetDefinition("AntiKt",1.0,cst.AntiKt4TruthJets,
                                        ghostdefs = [],
                                        modifiers = ("Sort", "Filter:200000",),
                                        standardRecoMode = True,
                                        lock = True
        )

    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    acc.merge(JetRecCfg(ConfigFlags,AntiKt10RCEMTopo))
    if ConfigFlags.Input.isMC: acc.merge(JetRecCfg(ConfigFlags,AntiKt10RCTruth))

    # LRT Egamma
    from DerivationFrameworkEGamma.EGammaLRTConfig import EGammaLRTCfg
    acc.merge(EGammaLRTCfg(ConfigFlags))

    from DerivationFrameworkLLP.LLPToolsConfig import LRTElectronLHSelectorsCfg
    acc.merge(LRTElectronLHSelectorsCfg(ConfigFlags))

    #Photon ID Selector
    from DerivationFrameworkLLP.LLPToolsConfig import PhotonIsEMSelectorsCfg
    acc.merge(PhotonIsEMSelectorsCfg(ConfigFlags))

    # LRT Muons
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    acc.merge(MuonsCommonCfg(ConfigFlags,
                             suff="LRT"))

    # flavor tagging
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import FtagJetCollectionsCfg
    acc.merge(FtagJetCollectionsCfg(ConfigFlags, ['AntiKt4EMTopoJets']))

    # VrtSecInclusive
    from VrtSecInclusive.VrtSecInclusiveConfig import VrtSecInclusiveCfg

    acc.merge(VrtSecInclusiveCfg(ConfigFlags,
                                 name = "VrtSecInclusive",
                                 AugmentingVersionString  = "",
                                 FillIntermediateVertices = False,
                                 TrackLocation            = MergedTrackCollection))
    LLP1VrtSecInclusiveSuffixes.append("")


    if ConfigFlags.Input.isMC:
        LLP1TrackFilterToolLRT = CompFactory.InDet.InclusiveTrackFilterTool(name = "LLP1TrackFilterToolLRT")
        acc.addPublicTool(LLP1TrackFilterToolLRT)
        LLP1TrackFilterToolSTD = CompFactory.InDet.InDetTrackTruthFilterTool(name = "LLP1TrackFilterToolSTD")
        acc.addPublicTool(LLP1TrackFilterToolSTD)
        TrackSystSuffix = "_TRK_EFF_LARGED0_GLOBAL__1down"
        acc.addEventAlgo(CompFactory.InDet.TrackSystematicsAlg(
                            name ="InDetTrackSystematicsAlg",
                            InputTrackContainer  = MergedTrackCollection,
                            OutputTrackContainer = f"{MergedTrackCollection}{TrackSystSuffix}",
                            TrackFilterToolLRT      = LLP1TrackFilterToolLRT,
                            TrackFilterToolSTD      = LLP1TrackFilterToolSTD
                          ))
        acc.merge(VrtSecInclusiveCfg(ConfigFlags,
                                     name = f"VrtSecInclusive{TrackSystSuffix}",
                                     AugmentingVersionString  = TrackSystSuffix,
                                     FillIntermediateVertices = False,
                                     TrackLocation            = f"{MergedTrackCollection}{TrackSystSuffix}"))
        LLP1VrtSecInclusiveSuffixes.append(TrackSystSuffix)

    # leptons-only VSI
    LeptonsSuffix = "_Leptons"
    acc.merge(VrtSecInclusiveCfg(ConfigFlags,
                                 name = "VrtSecInclusive_InDet"+LeptonsSuffix,
                                 AugmentingVersionString     = LeptonsSuffix,
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = MergedTrackCollection,
                                 twoTrkVtxFormingD0Cut       = 1.0,
                                 doSelectTracksFromMuons     = True,
                                 doRemoveCaloTaggedMuons     = True,
                                 doSelectTracksFromElectrons = True,
                                 MuonLocation                = MergedMuonContainer,
                                 ElectronLocation            = MergedElectronContainer))
    LLP1VrtSecInclusiveSuffixes.append(LeptonsSuffix)

    # track VSI
    LepTrackSuffix = "_LepTrack"
    acc.merge(VrtSecInclusiveCfg(ConfigFlags,
                                 name = "VrtSecInclusive_InDet"+LepTrackSuffix,
                                 AugmentingVersionString     = LepTrackSuffix,
                                 FillIntermediateVertices    = False,
                                 TrackLocation               = MergedTrackCollection,
                                 MuonLocation                = MergedMuonContainer,
                                 ElectronLocation            = MergedElectronContainer,
                                 twoTrkVtxFormingD0Cut       = 1.0,
                                 doSelectIDAndGSFTracks      = True,
                                 doRemoveCaloTaggedMuons     = True,
                                 doRemoveNonLeptonVertices   = True,
                                 doAssociateNonSelectedTracks= False))
    LLP1VrtSecInclusiveSuffixes.append(LepTrackSuffix)

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg, TauTrackParticleThinningCfg, DiTauTrackParticleThinningCfg, TauJetLepRMParticleThinningCfg
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg

    # Inner detector group recommendations for indet tracks in analysis
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    LLP1_thinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"
    LLP1TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        ConfigFlags,
        name                    = "LLP1TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = LLP1_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with muons
    LLP1MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "LLP1MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    # Include LRT inner detector tracks associated with LRT muons
    LLP1LRTMuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "LLP1LRTMuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "MuonsLRT",
        InDetTrackParticlesKey  = "InDetLargeD0TrackParticles"))

    # disable tau thinning for now
    tau_thinning_expression = "(TauJets.ptFinalCalib >= 0)"
    LLP1TauJetsThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(ConfigFlags,
        name            = "LLP1TauJetThinningTool",
        StreamName      = kwargs['StreamName'],
        ContainerName   = "TauJets",
        SelectionString = tau_thinning_expression))

    # Only keep tau tracks (and associated ID tracks) classified as charged tracks
    LLP1TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        ConfigFlags,
        name                   = "LLP1TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))

    tau_murm_thinning_expression = tau_thinning_expression.replace('TauJets', 'TauJets_MuonRM')
    LLP1TauJetMuonRMParticleThinningTool = acc.getPrimaryAndMerge(TauJetLepRMParticleThinningCfg(
        ConfigFlags,
        name                   = "LLP1TauJets_MuonRMThinningTool",
        StreamName             = kwargs['StreamName'],
        originalTauKey         = "TauJets",
        LepRMTauKey            = "TauJets_MuonRM",
        InDetTrackParticlesKey = "InDetTrackParticles",
        TauTracksKey           = "TauTracks_MuonRM",
        SelectionString        = tau_murm_thinning_expression))

    # ID tracks associated with high-pt di-tau
    LLP1DiTauTPThinningTool = acc.getPrimaryAndMerge(DiTauTrackParticleThinningCfg(
        ConfigFlags,
        name                    = "LLP1DiTauTPThinningTool",
        StreamName              = kwargs['StreamName'],
        DiTauKey                = "DiTauJets",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    ## Low-pt di-tau thinning
    LLP1DiTauLowPtThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(ConfigFlags,
                                                                                 name            = "LLP1DiTauLowPtThinningTool",
                                                                                 StreamName      = kwargs['StreamName'],
                                                                                 ContainerName   = "DiTauJetsLowPt",
                                                                                 SelectionString = "DiTauJetsLowPt.nSubjets > 1"))

    # ID tracks associated with low-pt ditau
    LLP1DiTauLowPtTPThinningTool = acc.getPrimaryAndMerge(DiTauTrackParticleThinningCfg(ConfigFlags,
                                                                                        name                    = "LLP1DiTauLowPtTPThinningTool",
                                                                                        StreamName              = kwargs['StreamName'],
                                                                                        DiTauKey                = "DiTauJetsLowPt",
                                                                                        InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                                        SelectionString         = "DiTauJetsLowPt.nSubjets > 1"))




    # ID Tracks associated with secondary vertices
    from DerivationFrameworkLLP.LLPToolsConfig import VSITrackParticleThinningCfg
    LLP1VSITPThinningTool = acc.getPrimaryAndMerge(VSITrackParticleThinningCfg(ConfigFlags,
                                                                               name                    = "LLP1VSITPThinningTool",
                                                                               StreamName              = kwargs['StreamName'],
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                               AugVerStrings = LLP1VrtSecInclusiveSuffixes))
    LLP1LRTVSITPThinningTool = acc.getPrimaryAndMerge(VSITrackParticleThinningCfg(ConfigFlags,
                                                                                  name                    = "LLP1LRTVSITPThinningTool",
                                                                                  StreamName              = kwargs['StreamName'],
                                                                                  InDetTrackParticlesKey  = "InDetLargeD0TrackParticles",
                                                                                  AugVerStrings = LLP1VrtSecInclusiveSuffixes))



    # ID Tracks associated with jets
    from DerivationFrameworkLLP.LLPToolsConfig import JetTrackParticleThinningCfg, JetLargeD0TrackParticleThinningCfg
    LLP1JetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(ConfigFlags,
                                                                               name                    = "LLP1JetTPThinningTool",
                                                                               StreamName              = kwargs['StreamName'],
                                                                               JetKey                  = "AntiKt4EMTopoJets",
                                                                               SelectionString         = "(AntiKt4EMTopoJets.pt > 20.*GeV) && (abs(AntiKt4EMTopoJets.eta) < 2.5)",
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles"))

    LLP1FatJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(  ConfigFlags,
                                                                                    name                    = "LLP1FatJetTPThinningTool",
                                                                                    StreamName              = kwargs['StreamName'],
                                                                                    JetKey                  = "AntiKt10EMTopoRCJets",
                                                                                    SelectionString         = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                    InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                                    ))

    # LRT Tracks associated with jets
    if ConfigFlags.Tracking.doLargeD0:
        LLP1LRTJetTPThinningTool = acc.getPrimaryAndMerge(JetLargeD0TrackParticleThinningCfg(ConfigFlags,
                                                                                             name                    = "LLP1LRTJetTPThinningTool",
                                                                                             StreamName              = kwargs['StreamName'],
                                                                                             JetKey                  = "AntiKt4EMTopoJets",
                                                                                             SelectionString         = "(AntiKt4EMTopoJets.pt > 20.*GeV) && (abs(AntiKt4EMTopoJets.eta) < 2.5)",
                                                                                             InDetTrackParticlesKey  = "InDetLargeD0TrackParticles"))

        LLP1LRTFatJetTPThinningTool = acc.getPrimaryAndMerge(JetLargeD0TrackParticleThinningCfg(ConfigFlags,
                                                                                                name                    = "LLP1LRTFatJetTPThinningTool",
                                                                                                StreamName              = kwargs['StreamName'],
                                                                                                JetKey                  = "AntiKt10EMTopoRCJets",
                                                                                                SelectionString         = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                                InDetTrackParticlesKey  = "InDetLargeD0TrackParticles",
                                                                                                ))

    # Finally the kernel itself
    thinningTools = [LLP1TrackParticleThinningTool,
                     LLP1MuonTPThinningTool,
                     LLP1LRTMuonTPThinningTool,
                     LLP1TauJetsThinningTool,
                     LLP1TauTPThinningTool,
                     LLP1TauJetMuonRMParticleThinningTool,
                     LLP1DiTauTPThinningTool,
                     LLP1DiTauLowPtThinningTool,
                     LLP1DiTauLowPtTPThinningTool,
                     LLP1VSITPThinningTool,
                     LLP1LRTVSITPThinningTool,
                     LLP1JetTPThinningTool,
                     LLP1FatJetTPThinningTool]

    if ConfigFlags.Tracking.doLargeD0:
        thinningTools.append(LLP1LRTJetTPThinningTool)
        thinningTools.append(LLP1LRTFatJetTPThinningTool)

    # Additionnal augmentations

    # Compute RC substructure variables from energy clusters
    from DerivationFrameworkLLP.LLPToolsConfig import RCJetSubstructureAugCfg
    LLP1RCJetSubstructureAugTool = acc.getPrimaryAndMerge(RCJetSubstructureAugCfg(ConfigFlags,
                                                                                    name                              = "LLP1RCJetSubstructureAugTool",
                                                                                    StreamName                        = kwargs['StreamName'],
                                                                                    JetContainerKey                   = "AntiKt10EMTopoRCJets",
                                                                                    SelectionString                   = "(AntiKt10EMTopoRCJets.pt > 200.*GeV) && (abs(AntiKt10EMTopoRCJets.eta) < 2.5)",
                                                                                    GhostClusterName                  = "GhostLCTopo",
                                                                                    Suffix                            = "cluster"
                                                                                    ))
    RCSubstructureAug = CompFactory.DerivationFramework.CommonAugmentation("RCSubstructureAug", AugmentationTools = [LLP1RCJetSubstructureAugTool])
    acc.addEventAlgo(RCSubstructureAug)

    # Skimming
    skimmingTools = []

    from DerivationFrameworkLLP.LLPToolsConfig import LLP1TriggerSkimmingToolCfg
    LLP1TriggerSkimmingTool = acc.getPrimaryAndMerge(LLP1TriggerSkimmingToolCfg(ConfigFlags,
                                                                                name = "LLP1TriggerSkimmingTool"))

    skimmingTools.append(LLP1TriggerSkimmingTool)

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools = skimmingTools,
                                      ThinningTools = thinningTools,
                                      AugmentationTools = augmentationTools))

    return acc


def LLP1Cfg(ConfigFlags):

    acc = ComponentAccumulator()
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    LLP1TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Common augmentations
    acc.merge(LLP1KernelCfg(ConfigFlags, name="LLP1Kernel", StreamName = 'StreamDAOD_LLP1', TriggerListsHelper = LLP1TriggerListsHelper))

    ## CloseByIsolation correction augmentation
    ## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
    ## Must set useSelTools to set elLHVLoose and phIsEMLoose with tools - not already set in LLP1 derivation
    from IsolationSelection.IsolationSelectionConfig import IsoCloseByAlgsCfg
    contNames = [ "Muons", "Electrons", "Photons", "LRTElectrons", "MuonsLRT" ]
    acc.merge(IsoCloseByAlgsCfg(ConfigFlags, isPhysLite = False, containerNames = contNames, useSelTools = True, stream_name = 'StreamDAOD_LLP1'))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    LLP1SlimmingHelper = SlimmingHelper("LLP1SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)

    LLP1SlimmingHelper.SmartCollections = ["EventInfo",
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

    LLP1SlimmingHelper.AllVariables =  ["MSDisplacedVertex",
                                        "MuonSpectrometerTrackParticles",
                                        "MuonSegments",
                                        "MSonlyTracklets",
                                        "CombinedMuonTrackParticles",
                                        "ExtrapolatedMuonTrackParticles",
                                        "CombinedMuonsLRTTrackParticles",
                                        "ExtraPolatedMuonsLRTTrackParticles",
                                        "MSOnlyExtraPolatedMuonsLRTTrackParticles",
                                        ]


    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::JetContainer#AntiKt10EMTopoRCJets","xAOD::JetAuxContainer#AntiKt10EMTopoRCJetsAux.-PseudoJet"]

    for wp in LLP1VrtSecInclusiveSuffixes:
        StaticContent += ["xAOD::VertexContainer#VrtSecInclusive_SecondaryVertices" + wp]
        StaticContent += ["xAOD::VertexAuxContainer#VrtSecInclusive_SecondaryVertices" + wp + "Aux."]

    LLP1SlimmingHelper.ExtraVariables += ["AntiKt10TruthTrimmedPtFrac5SmallR20Jets.Tau1_wta.Tau2_wta.Tau3_wta.D2.GhostBHadronsFinalCount",
                                          "Electrons.LHValue.DFCommonElectronsLHVeryLooseNoPixResult.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z.f3",
                                          "LRTElectrons.LHValue.DFCommonElectronsLHVeryLooseNoPixResult.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z.f3",
                                          "Photons.DFCommonPhotonsIsEMMedium.DFCommonPhotonsIsEMMediumIsEMValue.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z.f3",
                                          "egammaClusters.phi_sampl.eta0.phi0",
                                          "LRTegammaClusters.phi_sampl.eta0.phi0",
                                          "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostBHadronsFinal.GhostCHadronsFinal.GhostTrack.GhostTrackCount.GhostTrackLRT.GhostTrackLRTCount",
                                          "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PartonTruthLabelID.DFCommonJets_fJvt.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostBHadronsFinal.GhostCHadronsFinal",
                                          "AntiKtVR30Rmax4Rmin02TrackJets_BTagging201903.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostTausFinal.GhostTausFinalCount",
                                          "AntiKtVR30Rmax4Rmin02TrackJets_BTagging201810.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostTausFinal.GhostTausFinalCount",
                                          "TruthPrimaryVertices.t.x.y.z.sumPt2",
                                          "PrimaryVertices.t.x.y.z.sumPt2.covariance",
                                          "InDetTrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern",
                                          "InDetLargeD0TrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.hitPattern",
                                          "GSFTrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.hitPattern.truthOrigin.truthType",
                                          "LRTGSFTrackParticles.d0.z0.vz.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.truthParticleLink.truthMatchProbability.radiusOfFirstHit.numberOfPixelHoles.numberOfSCTHoles.numberDoF.chiSquared.hitPattern.truthOrigin.truthType",
                                          "EventInfo.hardScatterVertexLink.timeStampNSOffset",
                                          "TauJets.dRmax.etOverPtLeadTrk",
                                          "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                          "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]


    VSITrackAuxVars = [
        "is_selected", "is_associated", "is_svtrk_final", "pt_wrtSV", "eta_wrtSV",
        "phi_wrtSV", "d0_wrtSV", "z0_wrtSV", "errP_wrtSV", "errd0_wrtSV",
        "errz0_wrtSV", "chi2_toSV"
    ]

    for suffix in LLP1VrtSecInclusiveSuffixes:
        LLP1SlimmingHelper.ExtraVariables += [ "InDetTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        LLP1SlimmingHelper.ExtraVariables += [ "InDetLargeD0TrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        LLP1SlimmingHelper.ExtraVariables += [ "GSFTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]
        LLP1SlimmingHelper.ExtraVariables += [ "LRTGSFTrackParticles." + '.'.join( [ var + suffix for var in VSITrackAuxVars] ) ]

    ## CloseByIsolation content - CloseBy isolation correction (for all analyses)
    from IsolationSelection.IsolationSelectionConfig import  setupIsoCloseBySlimmingVariables
    setupIsoCloseBySlimmingVariables(LLP1SlimmingHelper, isLLP1 = True)

    # Truth containers
    if ConfigFlags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(LLP1SlimmingHelper)
        LLP1SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets', 'AntiKt4TruthJets']
        LLP1SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "LRTElectrons.TruthLink",
                                              "Muons.TruthLink",
                                              "MuonsLRT.TruthLink",
                                              "Photons.TruthLink"]

        StaticContent += ["xAOD::JetContainer#AntiKt10TruthRCJets","xAOD::JetAuxContainer#AntiKt10TruthRCJetsAux.-PseudoJet"]

    LLP1SlimmingHelper.StaticContent = StaticContent

    # Trigger content
    LLP1SlimmingHelper.IncludeTriggerNavigation = False
    LLP1SlimmingHelper.IncludeJetTriggerContent = False
    LLP1SlimmingHelper.IncludeMuonTriggerContent = False
    LLP1SlimmingHelper.IncludeEGammaTriggerContent = False
    LLP1SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    LLP1SlimmingHelper.IncludeTauTriggerContent = False
    LLP1SlimmingHelper.IncludeEtMissTriggerContent = False
    LLP1SlimmingHelper.IncludeBJetTriggerContent = False
    LLP1SlimmingHelper.IncludeBPhysTriggerContent = False
    LLP1SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkLLP.LLPToolsConfig import LLP1TriggerMatchingToolRun2Cfg
        acc.merge(LLP1TriggerMatchingToolRun2Cfg(ConfigFlags,
                                              name = "LRTTriggerMatchingTool",
                                              OutputContainerPrefix = "LRTTrigMatch_",
                                              TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesNoTau,
                                              InputElectrons=MergedElectronContainer,
                                              InputMuons=MergedMuonContainer
                                              ))
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLP1SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLP1SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesNoTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = LLP1SlimmingHelper,
                                         OutputContainerPrefix = "LRTTrigMatch_",
                                         TriggerList = LLP1TriggerListsHelper.Run2TriggerNamesNoTau,
                                         InputElectrons=MergedElectronContainer,
                                         InputMuons=MergedMuonContainer
                                         )
    # Run 3
    elif ConfigFlags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(LLP1SlimmingHelper)


    # Output stream
    LLP1ItemList = LLP1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_LLP1", ItemList=LLP1ItemList, AcceptAlgs=["LLP1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_LLP1", AcceptAlgs=["LLP1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

