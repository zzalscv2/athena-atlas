# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY1.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY1"
streamName = "StreamDAOD_BPHY1"

OniaContainerName = "BPHY1OniaCandidates"

def BPHY1Kernel(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    acc = ComponentAccumulator()
    doLRT = flags.Tracking.doLargeD0
    BPHY1_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
       name = "BPHY1_AugOriginalCounts",
       VertexContainer = "PrimaryVertices",
       TrackContainer = "InDetTrackParticles",
       TrackLRTContainer = "InDetLargeD0TrackParticles" if doLRT else "" )
    mainMuonInput = "StdWithLRTMuons" if doLRT else "Muons"
    mainIDInput   = "InDetWithLRTTrackParticles" if doLRT else "InDetTrackParticles"
    if doLRT:
        from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
        from AthenaConfiguration.Enums import LHCPeriod
        acc.merge(LRTMuonMergerAlg( flags,
                                    PromptMuonLocation    = "Muons",
                                    LRTMuonLocation       = "MuonsLRT",
                                    OutputMuonLocation    = mainMuonInput,
                                    CreateViewCollection  = True,
                                    UseRun3WP = flags.GeoModel.Run == LHCPeriod.Run3))
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg(flags))

    toRelink = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else []
    MuonReLink = [ "Muons", "MuonsLRT" ] if doLRT else []
    
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    TrackSelector = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(TrackSelector)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    BPHY1JpsiFinder = CompFactory.Analysis.JpsiFinder(
           name                        = "BPHY1JpsiFinder",
           muAndMu                     = True,
           muAndTrack                  = False,
           TrackAndTrack               = False,
           assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
           invMassUpper                = 100000.0,
           invMassLower                = 0.0,
           Chi2Cut                     = 200.,
           oppChargesOnly               = True,
           atLeastOneComb              = True,
           useCombinedMeasurement      = False, # Only takes effect if combOnly=True
           muonCollectionKey           = mainMuonInput,
           TrackParticleCollection     = mainIDInput,
           useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
           TrkVertexFitterTool         = vkalvrt,
           V0VertexFitterTool          = None,
           TrackSelectorTool           = TrackSelector,
           VertexPointEstimator        = vpest,
           useMCPCuts                  = False )

    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    BPHY1_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
            name                   = "BPHY1_Reco_mumu",
            VertexSearchTool       = BPHY1JpsiFinder,
            OutputVtxContainerName = OniaContainerName,
            PVContainerName        = "PrimaryVertices",
            RefPVContainerName     = "BPHY1RefittedPrimaryVertices",
            RefitPV                = True,
            MaxPVrefit             = 100000,
            V0Tools                = V0Tools,
            RelinkTracks  =  toRelink,
            RelinkMuons   =  MuonReLink,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            DoVertexType           = 7)

    BPHY1_Select_Jpsi2mumu =CompFactory.DerivationFramework.Select_onia2mumu(
            name                  = "BPHY1_Select_Jpsi2mumu",
            HypothesisName        = "Jpsi",
            InputVtxContainerName = "BPHY1OniaCandidates",
            V0Tools               = V0Tools,
            VtxMassHypo           = 3096.916,
            MassMin               = 2000.0,
            MassMax               = 3600.0,
            Chi2Max               = 200,
            DoVertexType          = 7)
    BPHY1_Select_Psi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
          name                  = "BPHY1_Select_Psi2mumu",
          HypothesisName        = "Psi",
          InputVtxContainerName = "BPHY1OniaCandidates",
          V0Tools               = V0Tools,
          VtxMassHypo           = 3686.09,
          MassMin               = 3300.0,
          MassMax               = 4500.0,
          Chi2Max               = 200,
          DoVertexType          = 7)
    BPHY1_Select_Upsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
          name                  = "BPHY1_Select_Upsi2mumu",
          HypothesisName        = "Upsi",
          InputVtxContainerName = "BPHY1OniaCandidates",
          V0Tools               = V0Tools,
          VtxMassHypo           = 9460.30,
          MassMin               = 7000.0,
          MassMax               = 12500.0,
          Chi2Max               = 200,
          DoVertexType          = 7)
    augTools = [BPHY1_AugOriginalCounts, BPHY1_Reco_mumu, BPHY1_Select_Jpsi2mumu, BPHY1_Select_Psi2mumu,  BPHY1_Select_Upsi2mumu]
    for t in [BPHY1JpsiFinder] + augTools : acc.addPublicTool(t)


    BPHY1Thin_vtxTrk = CompFactory.DerivationFramework.Thin_vtxTrk(
          name                       = "BPHY1Thin_vtxTrk",
          TrackParticleContainerName = "InDetTrackParticles",
          StreamName = streamName,
          VertexContainerNames       = ["BPHY1OniaCandidates"],
          PassFlags                  = ["passed_Jpsi", "passed_Psi", "passed_Upsi"] )
    BPHY1MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name      = "BPHY1MuonTPThinningTool",
                                                                         MuonKey       = "Muons",
                                                                         StreamName = streamName,
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")
    BPHY1ThinningTools = [BPHY1Thin_vtxTrk, BPHY1MuonTPThinningTool]
    if flags.Input.isMC :
        TruthSelection ="TruthParticles.pdgId == 443 || TruthParticles.pdgId == 100443 || TruthParticles.pdgId == 553 || TruthParticles.pdgId == 100553 || TruthParticles.pdgId == 200553"
        BPHY1TruthThinTool = CompFactory.DerivationFramework.GenericTruthThinning(name = "BPHY1TruthThinTool",
                               StreamName = streamName,
                               ParticleSelectionString = TruthSelection,
                               PreserveDescendants     = True,
                               PreserveAncestors      = True)
        BPHY1ThinningTools.append(BPHY1TruthThinTool)
        

    SelectExpression = "count(BPHY1OniaCandidates.passed_Jpsi) > 0 || count(BPHY1OniaCandidates.passed_Psi) > 0 || count(BPHY1OniaCandidates.passed_Upsi) > 0"
    BPHY1_SelectEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY1_SelectEvent", expression = SelectExpression)
    acc.addPublicTool(BPHY1_SelectEvent)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY1Kernel",
                            SkimmingTools = [BPHY1_SelectEvent],
                            ThinningTools     = BPHY1ThinningTools,
                            AugmentationTools = augTools))
    for tool in BPHY1ThinningTools : acc.addPublicTool(tool)
    return acc

def BPHY1Cfg(flags):
    doLRT = flags.Tracking.doLargeD0
    acc = BPHY1Kernel(flags)
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
    

    ## primary vertices
    AllVariables += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY1RefittedPrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY1RefittedPrimaryVerticesAux."]
    
    ## ID track particles
    AllVariables += ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]
    
    ## muon container
    AllVariables += ["Muons", "MuonsLRT"] if doLRT else ["Muons"]
    
    ## Jpsi candidates 
    StaticContent += ["xAOD::VertexContainer#%s"        % OniaContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % OniaContainerName]
    
    # Truth information for MC only
    if flags.Input.isMC :
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY1SlimmingHelper = SlimmingHelper("BPHY1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, ConfigFlags = flags)
    # Needed for trigger objects
    BPHY1SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY1SlimmingHelper.IncludeBPhysTriggerContent = True

    BPHY1SlimmingHelper.AllVariables = AllVariables
    BPHY1SlimmingHelper.StaticContent = StaticContent
    BPHY1ItemList = BPHY1SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY1", ItemList=BPHY1ItemList, AcceptAlgs=["BPHY1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY1", AcceptAlgs=["BPHY1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
