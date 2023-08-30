# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_MUON1.py
# This defines the component accumulator version of DAOD_MUON1 
# It requires the flag MUON1 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
import AthenaCommon.SystemOfUnits as Units

# Main algorithm config
def MUON1KernelCfg(ConfigFlags, name='MUON1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for MUON1"""
    acc = ComponentAccumulator()

    # --------------------
    # Common augmentations
    # --------------------
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # ------------
    # Augmentation
    # ------------

    # Augmentation tools
    from DerivationFrameworkMuons.MuonsToolsConfig import DiMuonTaggingAlgCfg
    MUON1AugmentTools = []                                     
    # Strings for applying cuts based on augmentations
    skimmingORs = []  

    muonThinFlags = []
    trkThinFlags = []


    # Z->mumu events
    brPrefix1a = 'Muon1ZmumuOC'
    ### Z mumu OC events
    acc.merge(DiMuonTaggingAlgCfg(ConfigFlags,
                                  name                    = "DiMuonTaggingZmumuOC",
                                  Mu1PtMin                = 24*Units.GeV,
                                  Mu1AbsEtaMax            = 2.5,
                                  Mu1Types                = [0],
                                  Mu2PtMin                = 2.5*Units.GeV,
                                  Mu2AbsEtaMax            = 9999.,
                                  UseTrackProbe           = True, # bool
                                  TrackContainerKey       = 'InDetTrackParticles', # str
                                  OppositeCharge          = True,
                                  InvariantMassLow        = 60*Units.GeV,
                                  InvariantMassHigh       = -1,
                                  IDTrackThinningConeSize = 0.4,
                                  BranchPrefix            = brPrefix1a))  

    skimmingORs.append("DIMU_pass{sel_flag}>0".format(sel_flag = brPrefix1a))
    muonThinFlags.append("Muons.pass{sel_flag}".format(sel_flag = brPrefix1a))
    trkThinFlags.append("InDetTrackParticles.pass{sel_flag}".format(sel_flag = brPrefix1a))
    
    brPrefix1b = 'Muon1ZmumuSC'
    ### Z mumu SC events
    acc.merge(DiMuonTaggingAlgCfg(ConfigFlags,
                                  name                    = "DiMuonTaggingZmumuSC",
                                  Mu1PtMin                = 24*Units.GeV,
                                  Mu1AbsEtaMax            = 2.5,
                                  Mu1Types                = [0],
                                  Mu2PtMin                = 2.5*Units.GeV,
                                  Mu2AbsEtaMax            = 9999.,
                                  UseTrackProbe           = True, # bool
                                  TrackContainerKey       = 'InDetTrackParticles', # str
                                  OppositeCharge          = False,
                                  InvariantMassLow        = 60*Units.GeV,
                                  InvariantMassHigh       = -1,
                                  IDTrackThinningConeSize = 0.4,
                                  BranchPrefix            = brPrefix1b)) 

    skimmingORs.append("DIMU_pass{sel_flag}>0".format(sel_flag = brPrefix1b))
    muonThinFlags.append("Muons.pass{sel_flag}".format(sel_flag = brPrefix1b))
    trkThinFlags.append("InDetTrackParticles.pass{sel_flag}".format(sel_flag = brPrefix1b))
    

    # Jpsi for tag-probe
    andTriggers1c = ['HLT_mu20_iloose_L1MU15', 'HLT_mu24', 'HLT_mu26', 'HLT_mu24_imedium', 'HLT_mu26_imedium']
    orTriggers1c = ['HLT_mu4','HLT_mu6','HLT_mu14','HLT_mu6_idperf',"HLT_mu6_bJpsi_lowpt_TrkPEB", 'HLT_mu4_bJpsi_Trkloose','HLT_mu6_bJpsi_Trkloose ','HLT_mu10_bJpsi_Trkloose','HLT_mu18_bJpsi_Trkloose','HLT_mu20_2mu0noL1_JpsimumuFS','HLT_mu18_2mu0noL1_JpsimumuFS','HLT_mu20_2mu4_JpsimumuL2','HLT_mu18_2mu4_JpsimumuL2','HLT_mu4_mu4_idperf_bJpsimumu_noid','HLT_mu4_bJpsi_TrkPEB','HLT_mu6_bJpsi_TrkPEB','HLT_mu10_bJpsi_TrkPEB','HLT_mu14_bJpsi_TrkPEB','HLT_mu20_bJpsi_TrkPEB','HLT_mu6_mu2noL1_msonly_bJpsimumu_noid','HLT_mu6_mu2noL1_msonly_bJpsimumu_noid_PEB']
    brPrefix1c = 'Muon1JPsiTP'
    acc.merge(DiMuonTaggingAlgCfg(ConfigFlags,
                                  name                    = "DiMuonTaggingJpsiTP",
                                  OrTrigs                 = orTriggers1c,
                                  AndTrigs                = andTriggers1c,
                                  Mu1PtMin                = 4*Units.GeV,
                                  Mu1AbsEtaMax            = 2.5,
                                  Mu1Types                = [0],
                                  Mu2PtMin                = 2.5*Units.GeV,
                                  UseTrackProbe           = True, # bool
                                  TrackContainerKey       = 'InDetTrackParticles', # str
                                  OppositeCharge          = True,
                                  InvariantMassLow        = 2.0*Units.GeV,
                                  InvariantMassHigh       = 4.8*Units.GeV,
                                  IDTrackThinningConeSize = 0.4,
                                  BranchPrefix            = brPrefix1c))
    skimmingORs.append("DIMU_pass{sel_flag}>0".format(sel_flag = brPrefix1c))
    muonThinFlags.append("Muons.pass{sel_flag}".format(sel_flag = brPrefix1c))
    trkThinFlags.append("InDetTrackParticles.pass{sel_flag}".format(sel_flag = brPrefix1c))
     
    ### Jpsi for calibration
    brPrefix1d = 'Muon1JPsiCalib' 
    acc.merge(DiMuonTaggingAlgCfg(ConfigFlags,
                                  name                    = "DiMuonTaggingJpsiCalib",
                                  Mu1PtMin                = 5.*Units.GeV,
                                  Mu1AbsEtaMax            = 999.,
                                  Mu1Types                = [0],
                                  Mu2PtMin                = 5.*Units.GeV,
                                  Mu2AbsEtaMax            = 999.,
                                  Mu2Types                = [0],
                                  UseTrackProbe           = False, # bool
                                  OppositeCharge          = True,
                                  InvariantMassLow        = 2.0*Units.GeV,
                                  InvariantMassHigh       = 4.8*Units.GeV,
                                  IDTrackThinningConeSize = 0.4,
                                  BranchPrefix            = brPrefix1d))
    
    skimmingORs.append("DIMU_pass{sel_flag}>0".format(sel_flag = brPrefix1d))
    muonThinFlags.append("Muons.pass{sel_flag}".format(sel_flag = brPrefix1d))
 
  
    ### Upsilon tagging
    brPrefix1e = 'Muon1Upsilon'
    acc.merge(DiMuonTaggingAlgCfg(ConfigFlags,
                                  name                    = "DiMuonTaggingUpsilon",
                                  Mu1PtMin                = 5.*Units.GeV,
                                  Mu1Types                = [0],
                                  Mu2PtMin                = 2.*Units.GeV,
                                  Mu2Types                = [0],
                                  OppositeCharge          = True,
                                  InvariantMassLow        = 7.0*Units.GeV,
                                  InvariantMassHigh       = 13.*Units.GeV,
                                  IDTrackThinningConeSize = 0.4,
                                  BranchPrefix            = brPrefix1e))
    skimmingORs.append("DIMU_pass{sel_flag}>0".format(sel_flag = brPrefix1e))
    muonThinFlags.append("Muons.pass{sel_flag}".format(sel_flag = brPrefix1e))

    ## MC truth classification and isolation
    if ConfigFlags.Input.isMC:        
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MuonTruthClassifierFallbackCfg
        MUON1MuonTruthClassifierFallback = acc.getPrimaryAndMerge(MuonTruthClassifierFallbackCfg(ConfigFlags,
                                                                                                 name         = "MUON1MuonTruthClassifierFallback",
                                                                                                 ContainerKey = "Muons"))
        MUON1AugmentTools.append(MUON1MuonTruthClassifierFallback)

        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MuonTruthIsolationToolCfg
        MUON1MuonTruthIsolationTool = acc.getPrimaryAndMerge(MuonTruthIsolationToolCfg(ConfigFlags, 
                                                                                       name         = "MUON1MuonTruthIsolationTool",
                                                                                       ContainerKey = "Muons"))
        MUON1AugmentTools.append(MUON1MuonTruthIsolationTool)
        brPrefix1f = "TruthMuon" 
        ### Accept every muon around a truth particle
        acc.merge(DiMuonTaggingAlgCfg(ConfigFlags,
                                      name                    = "MuonTruthTagging",
                                      Mu1PtMin                = 2.5*Units.GeV,
                                      Mu1Types                = [0],
                                      Mu2PtMin                = 60.*Units.TeV, # Dummy value
                                      UseTrackProbe           = True, # bool
                                      TrackContainerKey       = 'InDetTrackParticles', # str
                                      OppositeCharge          = False,
                                      InvariantMassLow        = 60*Units.TeV, # Dummy value
                                      InvariantMassHigh       = -1,
                                      IDTrackThinningConeSize = 0.1,
                                      BranchPrefix            = brPrefix1f)) 

        muonThinFlags.append("Muons.pass{sel_flag}".format(sel_flag = brPrefix1f))
        trkThinFlags.append("InDetTrackParticles.pass{sel_flag}".format(sel_flag = brPrefix1f))




    ### isolation decorations
    from DerivationFrameworkMuons.TrackIsolationDecoratorConfig import TrackIsolationCfg
    acc.merge(TrackIsolationCfg(ConfigFlags,TrackCollection="InDetTrackParticles", TrackSelections = trkThinFlags))
    acc.merge(TrackIsolationCfg(ConfigFlags,TrackCollection="ExtrapolatedMuonTrackParticles"))
    ### Calo deposits 
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonCaloDepositAlgCfg
    acc.merge(MuonCaloDepositAlgCfg(ConfigFlags)) ### Decorate directly the muons
    acc.merge(MuonCaloDepositAlgCfg(ConfigFlags, name = "IdTrkCaloDepsitDecorator", 
                                                 DecorateMuons = False)) ### Decorate the ID tracks
    
    # --------
    # Skimming
    # --------

    MUON1SkimmingTools = []
    skimming_expression = '||'.join(skimmingORs)
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    MUON1SkimmingTool1 = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(ConfigFlags,
                                                                          name       = "MUON1SkimmingTool1",
                                                                          expression = skimming_expression))
    MUON1SkimmingTools.append(MUON1SkimmingTool1) 

    # --------
    # Thinning
    # --------

    # Thinning tools
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    from DerivationFrameworkCalo.CaloCellDFGetterConfig import thinCaloCellsForDFCfg
    from DerivationFrameworkMuons.MuonsToolsConfig import AnalysisMuonThinningAlgCfg
    
    acc.merge(AnalysisMuonThinningAlgCfg(ConfigFlags,
                                         MuonPassFlags = muonThinFlags,
                                         TrkPassFlags = trkThinFlags,
                                         StreamName = kwargs['StreamName']))
    MUON1ThinningTools = [] 

    # keep topoclusters around muons
    MUON1ThinningTool4 = acc.getPrimaryAndMerge(CaloClusterThinningCfg(ConfigFlags,
                                                                       name                    = "MUON1ThinningTool4",
                                                                       StreamName              = kwargs['StreamName'],
                                                                       SGKey                   = "Muons",
                                                                       SelectionString         = "Muons.pt>4*GeV",
                                                                       TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                                                       ConeSize                = 0.5))
    MUON1ThinningTools.append(MUON1ThinningTool4)
    
    ### cell thinning
    acc.merge(thinCaloCellsForDFCfg(ConfigFlags,
                                    inputClusterKeys = ["MuonClusterCollection"],
                                    streamName       = kwargs['StreamName'],
                                    outputCellKey    = "DFMUONCellContainer"))

    ### Tracks associated with fitted vertices
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import Thin_vtxTrkCfg 
    MUON1Thin_vtxTrk = acc.getPrimaryAndMerge(Thin_vtxTrkCfg(ConfigFlags,
                                                             name                       = "MUON1Thin_vtxTrk",
                                                             StreamName                 = kwargs['StreamName'],
                                                             TrackParticleContainerName = "InDetTrackParticles",
                                                             VertexContainerNames       = ["MUON1JpsiCandidates"],
                                                             PassFlags                  = ["passed_Jpsi"] ))
    MUON1ThinningTools.append(MUON1Thin_vtxTrk)
                                                   
    ### Truth thinning
    if ConfigFlags.Input.isMC:        
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MenuTruthThinningCfg
        MUON1TruthThinningTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(ConfigFlags,
                                                                             name                            = "MUON1TruthThinningTool",
                                                                             StreamName                      = kwargs['StreamName'],
                                                                             WritePartons                    = False,
                                                                             WriteHadrons                    = False,
                                                                             WriteCHadrons                   = False,
                                                                             WriteBHadrons                   = True,
                                                                             WriteGeant                      = False,
                                                                             WriteTauHad                     = False,
                                                                             PartonPtThresh                  = -1.0,
                                                                             WriteBSM                        = True,
                                                                             WriteBosons                     = True,
                                                                             WriteBosonProducts              = False,
                                                                             WriteBSMProducts                = True,
                                                                             WriteTopAndDecays               = False,
                                                                             WriteEverything                 = False,
                                                                             WriteAllLeptons                 = True,
                                                                             WriteLeptonsNotFromHadrons      = False,
                                                                             WriteStatus3                    = False,
                                                                             WriteFirstN                     = -1,
                                                                             PreserveAncestors               = False,
                                                                             PreserveParentsSiblingsChildren = True,
                                                                             PreserveGeneratorDescendants    = False))
                                                                             # Not sure what this should be set to nowadays
                                                                             #SimBarcodeOffset                = DerivationFrameworkSimBarcodeOffset)  
        MUON1ThinningTools.append(MUON1TruthThinningTool)

    # --------------------
    # The kernel algorithm
    # --------------------
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      AugmentationTools = MUON1AugmentTools,
                                      SkimmingTools     = MUON1SkimmingTools, 
                                      ThinningTools     = MUON1ThinningTools))       
    return acc


def MUON1Cfg(ConfigFlags):
    stream_name = 'StreamDAOD_MUON1'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    MUON1TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # J/psi vertexing
    from DerivationFrameworkMuons.JPsiVertexFitSetupCfg import AddMCPJPsiVertexFitCfg
    acc.merge(AddMCPJPsiVertexFitCfg(ConfigFlags, prefix = 'MUON1'))  

    # Common augmentations
    acc.merge(MUON1KernelCfg(ConfigFlags, name="MUON1Kernel", StreamName = stream_name, TriggerListsHelper = MUON1TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    MUON1SlimmingHelper = SlimmingHelper("MUON1SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)
    MUON1SlimmingHelper.SmartCollections = ["EventInfo",
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

    MUON1SlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    MUON1SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]

    # Truth content
    if ConfigFlags.Input.isMC:

        MUON1SlimmingHelper.AllVariables += ["TruthEvents", "TruthParticles", "TruthVertices"]

    # Trigger content
    MUON1SlimmingHelper.IncludeTriggerNavigation = False
    MUON1SlimmingHelper.IncludeJetTriggerContent = False
    MUON1SlimmingHelper.IncludeMuonTriggerContent = False
    MUON1SlimmingHelper.IncludeEGammaTriggerContent = False
    MUON1SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    MUON1SlimmingHelper.IncludeTauTriggerContent = False
    MUON1SlimmingHelper.IncludeEtMissTriggerContent = False
    MUON1SlimmingHelper.IncludeBJetTriggerContent = False
    MUON1SlimmingHelper.IncludeBPhysTriggerContent = False
    MUON1SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON1SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = MUON1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON1SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = MUON1TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if ConfigFlags.Trigger.EDMVersion == 3 or (ConfigFlags.Trigger.EDMVersion == 2 and ConfigFlags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(MUON1SlimmingHelper)
        ##################################################### 
        ## NOTE: This block is temporary, during validation of the doEDMVersionConversion flag.
        ## This adds a LOT of containers to the output! In order to help validate the conversion.
        ## It should be removed once doEDMVersionConversion goes into production use.
        if ConfigFlags.Trigger.doEDMVersionConversion:   
            from DerivationFrameworkTrigger.TrigSlimmingHelper import addTrigEDMSetToOutput
            from AthenaCommon.Logging import logging
            msg = logging.getLogger('MUON1Cfg')
            msg.warn('doEDMVersionConversion is still in validation, WRITING FULL TRIGGER EDM TO THE DAOD!')
            addTrigEDMSetToOutput(ConfigFlags, MUON1SlimmingHelper, "AODFULL")
            MUON1SlimmingHelper.AppendToDictionary.update({'HLTNav_R2ToR3Summary':'xAOD::TrigCompositeContainer','HLTNav_R2ToR3SummaryAux':'xAOD::TrigCompositeAuxContainer'})
            MUON1SlimmingHelper.AllVariables += ['HLTNav_R2ToR3Summary']
        ##
        #####################################################

    # Output stream    
    MUON1ItemList = MUON1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_MUON1", ItemList=MUON1ItemList, AcceptAlgs=["MUON1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_MUON1", AcceptAlgs=["MUON1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

