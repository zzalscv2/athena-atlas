# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_PHYSLITE.py
# This defines DAOD_PHYSLITE, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag PHYSLITE in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def PHYSLITEKernelCfg(ConfigFlags, name='PHYSLITEKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PHYSLITE"""
    acc = ComponentAccumulator()

    # This block does the common physics augmentation  which isn't needed (or possible) for PHYS->PHYSLITE
    # Ensure block only runs for AOD input
    if 'StreamAOD' in ConfigFlags.Input.ProcessingTags:
        # Common augmentations
        from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
        acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule 
    # This differs depending on whether the input is AOD or PHYS
    # These are needed whatever the input since they are not applied in PHYS
    thinningToolsArgs = {
        'ElectronCaloClusterThinningToolName' : "PHYSLITEElectronCaloClusterThinningTool",
        'PhotonCaloClusterThinningToolName'   : "PHYSLITEPhotonCaloClusterThinningTool",     
        'ElectronGSFTPThinningToolName'       : "PHYSLITEElectronGSFTPThinningTool",
        'PhotonGSFTPThinningToolName'         : "PHYSLITEPhotonGSFTPThinningTool"
    }
    # whereas these are only needed if the input is AOD since they are applied already in PHYS
    if 'StreamAOD' in ConfigFlags.Input.ProcessingTags:
        thinningToolsArgs.update({
            'TrackParticleThinningToolName'       : "PHYSLITETrackParticleThinningTool",
            'MuonTPThinningToolName'              : "PHYSLITEMuonTPThinningTool",
            'TauJetThinningToolName'              : "PHYSLITETauJetThinningTool",
            'TauJets_MuonRMThinningToolName'      : "PHYSLITETauJets_MuonRMThinningTool",
            'DiTauTPThinningToolName'             : "PHYSLITEDiTauTPThinningTool",
            'DiTauLowPtThinningToolName'          : "PHYSLITEDiTauLowPtThinningTool",
            'DiTauLowPtTPThinningToolName'        : "PHYSLITEDiTauLowPtTPThinningTool",
        })
    # Configure the thinning tools
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(ConfigFlags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))


    #==============================================================================
    # Analysis-level variables 
    #==============================================================================

    # Set up the systematics loader/handler algorithm:
    sysLoader = CompFactory.CP.SystematicsSvc( 'SystematicsSvc' )
    sysLoader.systematicsList= ['']
    acc.addService(sysLoader)

    dataType = "data"
    if ConfigFlags.Input.isMC: dataType = "mc"

    # Needed in principle to support MET association when running PHYS->PHYSLITE, 
    # but since this doesn't work for PHYS->PHYSLITE anyway, commenting for now
    #if 'StreamDAOD_PHYS' in ConfigFlags.Input.ProcessingTags
    #    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    #    acc.merge(GeoModelCfg(ConfigFlags))    

    # Create a pile-up analysis sequence
    if ConfigFlags.Input.isMC:
        from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence
        pileupSequence = makePileupAnalysisSequence( dataType, campaign=ConfigFlags.Input.MCCampaign, files=ConfigFlags.Input.Files, useDefaultConfig=True )
        pileupSequence.configure( inputName = {}, outputName = {} )
        for element in pileupSequence.getGaudiConfig2Components():
            acc.addEventAlgo(element)

    # Include, and then set up the electron analysis sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import  makeElectronAnalysisSequence
    electronSequence = makeElectronAnalysisSequence( dataType, 'LooseLHElectron.NonIso', shallowViewOutput = False, deepCopyOutput = True, trackSelection = False )
    electronSequence.configure( inputName = 'Electrons',
                                outputName = 'AnalysisElectrons' )
    for element in electronSequence.getGaudiConfig2Components():
        acc.addEventAlgo(element)

    # Include, and then set up the photon analysis sequence:                                       
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import makePhotonAnalysisSequence
    photonSequence = makePhotonAnalysisSequence( dataType, 'Loose.Undefined', deepCopyOutput = True, shallowViewOutput = False, recomputeIsEM=False )
    photonSequence.configure( inputName = 'Photons',
                              outputName = 'AnalysisPhotons' )
    for element in photonSequence.getGaudiConfig2Components():
        acc.addEventAlgo(element)

    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    isRun3Geo = False
    from AthenaConfiguration.Enums import LHCPeriod
    if ConfigFlags.GeoModel.Run >= LHCPeriod.Run3: isRun3Geo = True 
    muonSequence = makeMuonAnalysisSequence( dataType, shallowViewOutput = False, deepCopyOutput = True, workingPoint = 'Loose.NonIso', isRun3Geo = isRun3Geo, trackSelection = False)
    muonSequence.configure( inputName = 'Muons',
                            outputName = 'AnalysisMuons' )
    for element in muonSequence.getGaudiConfig2Components():
        acc.addEventAlgo(element)

    # Include, and then set up the tau analysis algorithm sequence:                                                    
    # Commented for now due to use of public tools
    from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence
    tauSequence = makeTauAnalysisSequence( dataType, 'Baseline', shallowViewOutput = False, deepCopyOutput = True )
    tauSequence.configure( inputName = 'TauJets', outputName = 'AnalysisTauJets' )
    for element in tauSequence.getGaudiConfig2Components():
        acc.addEventAlgo(element)

    # Include, and then set up the jet analysis algorithm sequence:
    jetContainer = 'AntiKt4EMPFlowJets'
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer, deepCopyOutput = True, shallowViewOutput = False, runFJvtUpdate = False, runFJvtSelection = False, runJvtSelection = False)
    jetSequence.configure( inputName = jetContainer, outputName = 'AnalysisJets' )
    for element in jetSequence.getGaudiConfig2Components():
        acc.addEventAlgo(element)

    largeRjetContainer='AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets'
    largeRjetSequence = makeJetAnalysisSequence( dataType, largeRjetContainer, postfix="largeR",
                                                     deepCopyOutput = True, shallowViewOutput = False,
                                                     runGhostMuonAssociation = False)
    largeRjetSequence.configure( inputName = largeRjetContainer, outputName = 'AnalysisLargeRJets')
    for element in largeRjetSequence.getGaudiConfig2Components():
        acc.addEventAlgo(element)
    
    # Build MET from our analysis objects
    # Currently only works for the AOD->PHYSLITE workflow
    if 'StreamAOD' in ConfigFlags.Input.ProcessingTags:
        from METReconstruction.METAssocCfg import AssocConfig, METAssocConfig
        from METReconstruction.METAssociatorCfg import getAssocCA
        associators = [AssocConfig('PFlowJet', 'AnalysisJets'),
                       AssocConfig('Muon', 'AnalysisMuons'),
                       AssocConfig('Ele', 'AnalysisElectrons'),
                       AssocConfig('Gamma', 'AnalysisPhotons'),
                       AssocConfig('Tau', 'AnalysisTauJets'),
                       AssocConfig('Soft', '')]
        PHYSLITE_cfg = METAssocConfig('AnalysisMET',
                                      ConfigFlags,
                                      associators,
                                      doPFlow=True,
                                      usePFOLinks=True)
        components_PHYSLITE_cfg = getAssocCA(PHYSLITE_cfg,METName='AnalysisMET')
        acc.merge(components_PHYSLITE_cfg)

    # The derivation kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools)) 

    return acc


def PHYSLITECfg(ConfigFlags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    PHYSLITETriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # Set the stream name - varies depending on whether the input is AOD or DAOD_PHYS
    streamName = 'StreamDAOD_PHYSLITE' if 'StreamAOD' in ConfigFlags.Input.ProcessingTags else 'StreamD2AOD_PHYSLITE' 

    # Common augmentations
    acc.merge(PHYSLITEKernelCfg(ConfigFlags, name="PHYSLITEKernel", StreamName = streamName, TriggerListsHelper = PHYSLITETriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    PHYSLITESlimmingHelper = SlimmingHelper("PHYSLITESlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)

    # Trigger content
    PHYSLITESlimmingHelper.IncludeTriggerNavigation = False
    PHYSLITESlimmingHelper.IncludeJetTriggerContent = False
    PHYSLITESlimmingHelper.IncludeMuonTriggerContent = False
    PHYSLITESlimmingHelper.IncludeEGammaTriggerContent = False
    PHYSLITESlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    PHYSLITESlimmingHelper.IncludeTauTriggerContent = False
    PHYSLITESlimmingHelper.IncludeEtMissTriggerContent = False
    PHYSLITESlimmingHelper.IncludeBJetTriggerContent = False
    PHYSLITESlimmingHelper.IncludeBPhysTriggerContent = False
    PHYSLITESlimmingHelper.IncludeMinBiasTriggerContent = False
    
    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        # Need to re-run matching so that new Analysis<X> containers are matched to triggers
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun2Cfg
        acc.merge(TriggerMatchingCommonRun2Cfg(ConfigFlags, 
                                               name = "PHYSLITETrigMatchNoTau", 
                                               OutputContainerPrefix = "AnalysisTrigMatch_", 
                                               ChainNames = PHYSLITETriggerListsHelper.Run2TriggerNamesNoTau,
                                               InputElectrons = "AnalysisElectrons",
                                               InputPhotons = "AnalysisPhotons",
                                               InputMuons = "AnalysisMuons",
                                               InputTaus = "AnalysisTauJets"))
        acc.merge(TriggerMatchingCommonRun2Cfg(ConfigFlags, 
                                               name = "PHYSLITETrigMatchTau", 
                                               OutputContainerPrefix = "AnlaysisTrigMatch_", 
                                               ChainNames = PHYSLITETriggerListsHelper.Run2TriggerNamesTau, 
                                               DRThreshold = 0.2,
                                               InputElectrons = "AnalysisElectrons",
                                               InputPhotons = "AnalysisPhotons",
                                               InputMuons = "AnalysisMuons",
                                               InputTaus = "AnalysisTauJets"))
        # Now add the resulting decorations to the output 
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSLITESlimmingHelper, 
                                         OutputContainerPrefix = "AnalysisTrigMatch_", 
                                         TriggerList = PHYSLITETriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSLITESlimmingHelper, 
                                         OutputContainerPrefix = "AnalysisTrigMatch_",
                                         TriggerList = PHYSLITETriggerListsHelper.Run2TriggerNamesNoTau)

    # Run 3, or Run 2 with navigation conversion
    if ConfigFlags.Trigger.EDMVersion == 3 or (ConfigFlags.Trigger.EDMVersion == 2 and ConfigFlags.Trigger.doEDMVersionConversion):
        # No need to run matching: just keep navigation so matching can be done by analysts
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(PHYSLITESlimmingHelper)

    # Event content
    PHYSLITESlimmingHelper.AppendToDictionary.update({
        'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
        'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
        'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
        'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
        'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
        'TruthTaus':'xAOD::TruthParticleContainer','TruthTausAux':'xAOD::TruthParticleAuxContainer',
        'TruthNeutrinos':'xAOD::TruthParticleContainer','TruthNeutrinosAux':'xAOD::TruthParticleAuxContainer',
        'TruthBSM':'xAOD::TruthParticleContainer','TruthBSMAux':'xAOD::TruthParticleAuxContainer',
        'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
        'TruthTop':'xAOD::TruthParticleContainer','TruthTopAux':'xAOD::TruthParticleAuxContainer',
        'TruthForwardProtons':'xAOD::TruthParticleContainer','TruthForwardProtonsAux':'xAOD::TruthParticleAuxContainer',
        'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
        'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
        'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
        'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
        'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
        'HardScatterParticles':'xAOD::TruthParticleContainer','HardScatterParticlesAux':'xAOD::TruthParticleAuxContainer',
        'HardScatterVertices':'xAOD::TruthVertexContainer','HardScatterVerticesAux':'xAOD::TruthVertexAuxContainer',
        'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer',
        'AnalysisElectrons':'xAOD::ElectronContainer', 'AnalysisElectronsAux':'xAOD::ElectronAuxContainer',
        'AnalysisMuons':'xAOD::MuonContainer', 'AnalysisMuonsAux':'xAOD::MuonAuxContainer',
        'AnalysisJets':'xAOD::JetContainer','AnalysisJetsAux':'xAOD::AuxContainerBase',
        'AnalysisPhotons':'xAOD::PhotonContainer', 'AnalysisPhotonsAux':'xAOD::PhotonAuxContainer',
        'AnalysisTauJets':'xAOD::TauJetContainer', 'AnalysisTauJetsAux':'xAOD::TauJetAuxContainer',
        'MET_Core_AnalysisMET':'xAOD::MissingETContainer', 'MET_Core_AnalysisMETAux':'xAOD::MissingETAuxContainer',
        'METAssoc_AnalysisMET':'xAOD::MissingETAssociationMap', 'METAssoc_AnalysisMETAux':'xAOD::MissingETAuxAssociationMap',
        'AntiKt10TruthTrimmedPtFrac5SmallR20Jets':'xAOD::JetContainer', 'AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux':'xAOD::JetAuxContainer',
        'AnalysisLargeRJets':'xAOD::JetContainer','AnalysisLargeRJetsAux':'xAOD::AuxContainerBase'
    })

    PHYSLITESlimmingHelper.SmartCollections = [
        'EventInfo',
        'InDetTrackParticles',
        'PrimaryVertices',
    ]

    PHYSLITESlimmingHelper.ExtraVariables = [ 
        'AnalysisElectrons.trackParticleLinks.pt.eta.phi.m.charge.author.DFCommonElectronsLHVeryLoose.DFCommonElectronsLHLoose.DFCommonElectronsLHLooseBL.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsLHVeryLooseIsEMValue.DFCommonElectronsLHLooseIsEMValue.DFCommonElectronsLHLooseBLIsEMValue.DFCommonElectronsLHMediumIsEMValue.DFCommonElectronsLHTightIsEMValue.DFCommonElectronsECIDS.DFCommonElectronsECIDSResult.topoetcone20.neflowisol20.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500.ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500.ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000.caloClusterLinks.ambiguityLink.TruthLink.truthParticleLink.truthOrigin.truthType.truthPdgId.firstEgMotherTruthType.firstEgMotherTruthOrigin.firstEgMotherTruthParticleLink.firstEgMotherPdgId.ambiguityType.OQ',
        'AnalysisPhotons.pt.eta.phi.m.author.OQ.DFCommonPhotonsIsEMLoose.DFCommonPhotonsIsEMTight.DFCommonPhotonsIsEMTightIsEMValue.DFCommonPhotonsCleaning.DFCommonPhotonsCleaningNoTime.ptcone20.topoetcone20.topoetcone40.topoetcone20ptCorrection.topoetcone40ptCorrection.caloClusterLinks.vertexLinks.ambiguityLink.TruthLink.truthParticleLink.truthOrigin.truthType',
        'GSFTrackParticles.chiSquared.phi.d0.theta.qOverP.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.z0.vz.charge.vertexLink.numberOfPixelHits.numberOfSCTHits.originalTrackParticle',
        'GSFConversionVertices.trackParticleLinks.x.y.z.px.py.pz.pt1.pt2.neutralParticleLinks.minRfirstHit',
        'egammaClusters.calE.calEta.calPhi.calM.e_sampl.eta_sampl.ETACALOFRAME.PHICALOFRAME.ETA2CALOFRAME.PHI2CALOFRAME.constituentClusterLinks',
        'AnalysisMuons.pt.eta.phi.truthType.truthOrigin.author.muonType.quality.inDetTrackParticleLink.muonSpectrometerTrackParticleLink.combinedTrackParticleLink.InnerDetectorPt.MuonSpectrometerPt.DFCommonGoodMuon.neflowisol20.topoetcone20.TruthLink.truthParticleLink.charge.extrapolatedMuonSpectrometerTrackParticleLink.allAuthors.ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000.ptcone20_Nonprompt_All_MaxWeightTTVA_pt500.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500.numberOfPrecisionLayers.combinedTrackOutBoundsPrecisionHits.numberOfPrecisionLayers.numberOfPrecisionHoleLayers.numberOfGoodPrecisionLayers.innerSmallHits.innerLargeHits.middleSmallHits.middleLargeHits.outerSmallHits.outerLargeHits.extendedSmallHits.extendedLargeHits.extendedSmallHoles.isSmallGoodSectors.cscUnspoiledEtaHits.EnergyLoss.energyLossType.momentumBalanceSignificance.scatteringCurvatureSignificance.scatteringNeighbourSignificance.CaloMuonIDTag.CaloMuonScore',
        'CombinedMuonTrackParticles.qOverP.d0.z0.vz.phi.theta.truthOrigin.truthType.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.numberOfPixelDeadSensors.numberOfPixelHits.numberOfPixelHoles.numberOfSCTDeadSensors.numberOfSCTHits.numberOfSCTHoles.numberOfTRTHits.numberOfTRTOutliers.chiSquared.numberDoF',
        'ExtrapolatedMuonTrackParticles.d0.z0.vz.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.truthOrigin.truthType.qOverP.theta.phi',
        'MuonSpectrometerTrackParticles.phi.d0.z0.vz.definingParametersCovMatrixDiag.definingParametersCovMatrixOffDiag.vertexLink.theta.qOverP.truthParticleLink',
        'AnalysisTauJets.pt.eta.phi.m.ptFinalCalib.etaFinalCalib.ptTauEnergyScale.etaTauEnergyScale.charge.isTauFlags.PanTau_DecayMode.NNDecayMode.RNNJetScore.RNNJetScoreSigTrans.JetDeepSetScore.JetDeepSetScoreTrans.JetDeepSetVeryLoose.JetDeepSetLoose.JetDeepSetMedium.JetDeepSetTight.RNNEleScore.RNNEleScoreSigTrans.RNNEleScoreSigTrans_v1.EleRNNLoose_v1.EleRNNMedium_v1.EleRNNTight_v1.tauTrackLinks.vertexLink.truthParticleLink.truthJetLink.IsTruthMatched.truthOrigin.truthType',
        'AnalysisJets.pt.eta.phi.m.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.NumTrkPt500.SumPtTrkPt500.DetectorEta.JVFCorr.NNJvtPass.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.PartonTruthLabelID.ConeTruthLabelID.HadronConeExclExtendedTruthLabelID.HadronConeExclTruthLabelID.TrueFlavor.DFCommonJets_jetClean_LooseBad.DFCommonJets_jetClean_TightBad.Timing.btagging.btaggingLink.GhostTrack.DFCommonJets_fJvt.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.PSFrac.JetAccessorMap.EMFrac.Width.ActiveArea4vec_pt.ActiveArea4vec_eta.ActiveArea4vec_m.ActiveArea4vec_phi.EnergyPerSampling.SumPtChargedPFOPt500',
        'BTagging_AntiKt4EMPFlow.DL1dv01_pu.DL1dv01_pc.DL1dv01_pb.GN120220509_pu.GN120220509_pc.GN120220509_pb.GN2v00_pu.GN2v00_pc.GN2v00_pb',
        'AntiKt10UFOCSSKJets.GhostAntiKtVR30Rmax4Rmin02PV0TrackJets',
        'BTagging_AntiKtVR30Rmax4Rmin02Track.DL1dv01_pu.DL1dv01_pc.DL1dv01_pb.GN2v00_pu.GN2v00_pc.GN2v00_pb',
        'TruthPrimaryVertices.t.x.y.z',
        'MET_Core_AnalysisMET.name.mpx.mpy.sumet.source',
        'METAssoc_AnalysisMET.',
        'InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.numberOfTRTHits.numberOfTRTOutliers',
        'EventInfo.RandomRunNumber.PileupWeight_NOSYS.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ',
        'Kt4EMPFlowEventShape.Density',
        'TauTracks.pt.eta.phi.flagSet.trackLinks',
        'AnalysisLargeRJets.pt.eta.phi.m.JetConstitScaleMomentum_pt.JetConstitScaleMomentum_eta.JetConstitScaleMomentum_phi.JetConstitScaleMomentum_m.DetectorEta.TrackSumMass.TrackSumPt.constituentLinks.ECF1.ECF2.ECF3.Tau1_wta.Tau2_wta.Tau3_wta.Split12.Split23.Qw.D2.C2.R10TruthLabel_R22v1.R10TruthLabel_R21Precision_2022v1.R10TruthLabel_R21Precision.GhostBHadronsFinalCount.GhostCHadronsFinalCount.Parent.GN2Xv00_phbb.GN2Xv00_phcc.GN2Xv00_ptop.GN2Xv00_pqcd.GN2XWithMassv00_phbb.GN2XWithMassv00_phcc.GN2XWithMassv00_ptop.GN2XWithMassv00_pqcd',
        'EventInfo.RandomRunNumber.PileupWeight_NOSYS.GenFiltHT.GenFiltMET'
    ]

    if ConfigFlags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(PHYSLITESlimmingHelper)

    # Output stream    
    PHYSLITEItemList = PHYSLITESlimmingHelper.GetItemList()
    formatString = 'D2AOD_PHYSLITE' if 'StreamDAOD_PHYS' in ConfigFlags.Input.ProcessingTags else 'DAOD_PHYSLITE'
    acc.merge(OutputStreamCfg(ConfigFlags, formatString, ItemList=PHYSLITEItemList, AcceptAlgs=["PHYSLITEKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, formatString, AcceptAlgs=["PHYSLITEKernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

