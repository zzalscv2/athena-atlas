# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY12.py
#====================================================================


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY12"
streamName = "StreamDAOD_BPHY12"
#====================================================================
# FLAGS TO PERSONALIZE THE DERIVATION
#====================================================================

skimTruth = False

def BPHY12Cfg(ConfigFlags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    print("ERROR NOT WORKING - UPDATE to match share/BPHY12 when metadata tools are fixed")
    isSimulation = ConfigFlags.Input.isMC

    triggerList = [
                 "HLT_mu11_mu6_bBmumuxv2",
                 "HLT_2mu10_bBmumuxv2",
                 "HLT_2mu6_bBmumuxv2_L1LFV-MU6",
                 "HLT_mu11_mu6_bBmumux_BpmumuKp",
                 "HLT_2mu6_bBmumux_BpmumuKp_L1BPH-2M9-2MU6_BPH-2DR15-2MU6",
                 "HLT_mu11_mu6_bDimu",
                 "HLT_4mu4_bDimu6000"
                  ]

    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(ConfigFlags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(ConfigFlags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(ConfigFlags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(ConfigFlags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    BPHY12DiMuonFinder = CompFactory.Analysis.JpsiFinder(
                             name                        = "BPHY12DiMuonFinder",
                             muAndMu                     = True,
                             muAndTrack                  = False,
                             TrackAndTrack               = False,
                             assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
                             invMassUpper                = 100000.0,
                             invMassLower                = 0.0,
                             Chi2Cut                     = 200.,
                             oppChargesOnly              = True,
                             atLeastOneComb              = True,
                             useCombinedMeasurement      = False, # Only takes effect if combOnly=True   
                             muonCollectionKey           = "Muons",
                             TrackParticleCollection     = "InDetTrackParticles",
                             V0VertexFitterTool          = None, # V0 vertex fitter
                             useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
                             TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
                             TrackSelectorTool           = trackselect,
                             VertexPointEstimator        = vpest,
                             useMCPCuts                  = False )

    BPHY12_Reco_DiMuon = CompFactory.DerivationFramework.Reco_Vertex(
                             name                   = "BPHY12_Reco_DiMuon",
                             VertexSearchTool       = BPHY12DiMuonFinder,
                             OutputVtxContainerName = "BPHY12DiMuonCandidates",
                             V0Tools                = V0Tools,
                             PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(ConfigFlags)),
                             PVContainerName        = "PrimaryVertices",
                             RefPVContainerName     = "BPHY12RefittedPrimaryVertices",
                             RefitPV                = True,
                             MaxPVrefit             = 100000,
                             DoVertexType           = 7)

    BPHY12_Select_DiMuons = CompFactory.DerivationFramework.Select_onia2mumu(
      name                  = "BPHY12_Select_DiMuons",
      HypothesisName        = "Jpsi",
      V0Tools               = V0Tools,
      InputVtxContainerName = "BPHY12DiMuonCandidates",
      VtxMassHypo           = 3096.916,
      MassMin               = 1.0,
      MassMax               = 7000.0,
      Chi2Max               = 200.,
      DoVertexType          = 7)

    BPHY12BmumuKstFinder = CompFactory.Analysis.JpsiPlus2Tracks(
        name                    = "BPHY12BmumuKstFinder",
        kaonkaonHypothesis      = False,
        pionpionHypothesis      = False,
        kaonpionHypothesis      = True,
        trkThresholdPt          = 500.0, #minimum track pT in MeV
        trkMaxEta               = 3.0, 
        BThresholdPt            = 1000.,
        BMassLower              = 3000.0, #OI makes no sense below Jpsi mass #same values as BPHY18 (original) - Bs->JpsiKK
        BMassUpper              = 6500.0,
        JpsiContainerKey        = "BPHY12DiMuonCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        #MuonsUsedInJpsi         = "Muons", #Don't remove all muons, just those in J/psi candidate (see the following cut)
        ExcludeCrossJpsiTracks  = False,   #setting this to False rejects the muons from J/psi candidate
        TrkVertexFitterTool     = vkalvrt,
        TrackSelectorTool       = trackselect,
        UseMassConstraint       = False,
        DiTrackMassUpper        = 1110., #OI was 1500. Can eventually set these to be the K* mass?
        DiTrackMassLower        = 690.,  #OI was 500
        Chi2Cut                 = 15., #THIS IS CHI2/NDOF, checked the code!!!
        DiTrackPt               = 500.,
        TrkQuadrupletMassLower  = 1000.0, #Two electrons + two tracks (one K, one pi)
        TrkQuadrupletMassUpper  = 100000.0, # same as BPHY18, original
        FinalDiTrackPt          = 500.,
        )

    BPHY12_Reco_BmumuKst  = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY12_Reco_BmumuKst",
        VertexSearchTool     = BPHY12BmumuKstFinder,
        OutputVtxContainerName = "BPHY12BmumuKstCandidates",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "BPHY12RefittedPrimaryVertices2",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(ConfigFlags)),
        RefitPV                = True,
        MaxPVrefit             = 10000,
        DoVertexType = 7)

    BPHY12_Select_BmumuKst = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY12_Select_BmumuKst",
        V0Tools                    = V0Tools,
        HypothesisName             = "Bd", #creates output variable pass_Bd
        InputVtxContainerName      = "BPHY12BmumuKstCandidates",
        TrkMasses                  = [105.658, 105.658, 493.677, 139.570],
        VtxMassHypo                = 5279.6, #mass of B
        MassMin                    = 1.0,      #no mass cuts here
        MassMax                    = 10000.0,   #no mass cuts here
        Chi2Max                    = 30.0) #THIS IS CHI2! NOT CHI2/NDOF! Careful!

    if skimTruth or not isSimulation: #Only Skim Data
        expression = "count(BPHY12BmumuKstCandidates.passed_Bd) > 0"
        BPHY12_SelectEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY12_SelectEvent",
                                                                    expression = expression)

        BPHY12TriggerSkim = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "BPHY12TriggerSkim",
                                                        TriggerListOR = triggerList,
                                                        TriggerListAND = [] )
        filterlist = [BPHY12_SelectEvent, BPHY12TriggerSkim]
        for t in  filterlist : acc.addPublicTool(t)
        #====================================================================
        # Make event selection based on an OR of the input skimming tools (though it seems we only have one here!)
        #====================================================================
        BPHY12SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR(
            name       = "BPHY12SkimmingOR",
            FilterList = filterlist) #OR of all your different filters

    BPHY12Thin_vtxTrk = CompFactory.DerivationFramework.Thin_vtxTrk(
            name                       = "BPHY12Thin_vtxTrk",
            TrackParticleContainerName = "InDetTrackParticles",
            VertexContainerNames       = ["BPHY12BmumuKstCandidates"],
            PassFlags                  = ["passed_Bd"] )
    BPHY12MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name    = "BPHY12MuonTPThinningTool",
                                                                         MuonKey                 = "Muons",
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")

    BPHY12TruthThinTool = CompFactory.DerivationFramework.GenericTruthThinning(name = "BPHY12TruthThinTool",
                                                        ParticleSelectionString = "TruthParticles.pdgId == 511 || TruthParticles.pdgId == -511 || TruthParticles.pdgId == 531 || TruthParticles.pdgId == -531",
                                                        PreserveDescendants     = True,
                                                        PreserveAncestors      = True)

    BPHY12ThinningTools = [BPHY12Thin_vtxTrk, BPHY12MuonTPThinningTool]
    if isSimulation:
       BPHY12ThinningTools.append(BPHY12TruthThinTool)

    augTools = [BPHY12_Reco_DiMuon, BPHY12_Select_DiMuons,
                               BPHY12_Reco_BmumuKst, BPHY12_Select_BmumuKst]
    skimTools = [BPHY12SkimmingOR] if skimTruth or not isSimulation else []


    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
         "BPHY12Kernel",
          AugmentationTools = augTools,
          SkimmingTools     = skimTools,
          ThinningTools     = BPHY12ThinningTools
          ))

    for t in  augTools + skimTools + BPHY12ThinningTools : acc.addPublicTool(t)
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY12SlimmingHelper = SlimmingHelper("BPHY12SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []

    # Needed for trigger objects
    BPHY12SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY12SlimmingHelper.IncludeBPhysTriggerContent = True

    ## primary vertices
    AllVariables += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY12RefittedPrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY12RefittedPrimaryVerticesAux."]
    StaticContent += ["xAOD::VertexContainer#BPHY12RefittedPrimaryVertices2"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY12RefittedPrimaryVertices2Aux."]

    ## ID track particles
    AllVariables += ["InDetTrackParticles"]

    ## combined / extrapolated muon track particles
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]

    ## muon container
    AllVariables += ["Muons"]

    ## Jpsi candidates
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY12_Reco_DiMuon.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux." % BPHY12_Reco_DiMuon.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY12_Reco_DiMuon.OutputVtxContainerName]

    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY12_Reco_BmumuKst.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux." % BPHY12_Reco_BmumuKst.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY12_Reco_BmumuKst.OutputVtxContainerName]

    # Added by ASC
    # Truth information for MC only
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY12SlimmingHelper.AllVariables = AllVariables
    BPHY12SlimmingHelper.StaticContent = StaticContent

    BPHY12ItemList = BPHY12SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_BPHY12", ItemList=BPHY12ItemList, AcceptAlgs=["BPHY12Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_BPHY12", AcceptAlgs=["BPHY12Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
