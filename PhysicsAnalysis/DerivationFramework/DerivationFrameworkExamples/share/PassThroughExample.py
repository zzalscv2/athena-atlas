#====================================================================
# PassThroughExample.py
# This an example job options script showing how to set up a 
# derivation that neither skims, thins or slims: all events are written
# in full to the output. 
# If an AODFix is scheduled in the release, this will be run,
# so it will not be an exact copy of the input   
# It requires the reductionConf flag PASSTHR in Reco_tf.py   
#====================================================================

# Set up common services and job object. 
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================

# The name of the kernel (LooseSkimKernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel("PASSTHRKernel")

#====================================================================
# SET UP STREAM   
#====================================================================

# SKIMMING
# The base name (DAOD_PASSTHR here) must match the string in 
# DerivationFrameworkProdFlags (in DerivationFrameworkCore) 
streamName = derivationFlags.WriteDAOD_PASSTHRStream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_PASSTHRStream )
PASSTHRStream = MSMgr.NewPoolRootStream( streamName, fileName )
# Only events that pass the filters listed below are written out.
# Name must match that of the kernel above
# AcceptAlgs  = logical OR of filters
# RequireAlgs = logical AND of filters
PASSTHRStream.AcceptAlgs(["PASSTHRKernel"])

#====================================================================
# CONTENT LIST  
#====================================================================
from PrimaryDPDMaker import PrimaryDPD_OutputDefinitions as dpdOutput
excludeList = []
dpdOutput.addAllItemsFromInputExceptExcludeList( streamName, excludeList )

# Float compression
SeqNANO = CfgMgr.AthSequencer("SeqNANO")
DerivationFrameworkJob += SeqNANO
SeqNANO += CfgMgr.DerivationFramework__FloatCompressionAlg("NANOFloatCompression", SGKeys=["HLT_xAOD__TrigEMClusterContainer_TrigT2CaloEgammaAux.",
"TruthVerticesAux.",
"HLT_xAOD__TrigT2ZdcSignalsContainer_zdcsignalsAux.",
"InDetTrackParticlesClusterAssociationsAux.",
"MET_CaloAux.", "MET_Core_AntiKt4EMPFlowAux.", "MET_Core_AntiKt4EMTopoAux.", "MET_Core_AntiKt4LCTopoAux.", "MET_EMTopoAux.", "MET_EMTopoRegionsAux.", "MET_LocHadTopoAux.", "MET_LocHadTopoRegionsAux.", "MET_Reference_AntiKt4EMPFlowAux.", "MET_Reference_AntiKt4EMTopoAux.", "MET_Reference_AntiKt4LCTopoAux.", "MET_TrackAux.", "MET_TruthAux.", "MET_TruthRegionsAux.",
"HLT_xAOD__JetContainer_EFJetAux.", "HLT_xAOD__JetContainer_FarawayJetAux.", "HLT_xAOD__JetContainer_GSCJetAux.", "HLT_xAOD__JetContainer_SplitJetAux.", "HLT_xAOD__JetContainer_SuperRoiAux.", "HLT_xAOD__JetContainer_a10r_tcemsubjesFSAux.", "HLT_xAOD__JetContainer_a10r_tcemsubjesISFSAux.", "HLT_xAOD__JetContainer_a10tclcwsubjesFSAux.", "HLT_xAOD__JetContainer_a10ttclcwjesFSAux.", "HLT_xAOD__JetContainer_a3ionemsubjesFSAux.", "HLT_xAOD__JetContainer_a4ionemsubjesFSAux.", "HLT_xAOD__JetContainer_a4tcemsubjesFSAux.", "HLT_xAOD__JetContainer_a4tcemsubjesISFSAux.", "HLT_xAOD__JetContainer_a4tcemsubjesISFSftkAux.", "HLT_xAOD__JetContainer_a4tcemsubjesISFSftkrefitAux.", "HLT_xAOD__JetContainer_a4tclcwsubjesISFSAux.",
#"HLT_xAOD__TauJetContainer_TrigTauRecMergedAux.", 
#"HLT_xAOD__TauJetContainer_TrigTauRecPreselectionAux.", 
"TauJetsAux.",
"HLT_xAOD__EmTauRoIContainer_L1TopoEMAux.", "HLT_xAOD__EmTauRoIContainer_L1TopoTauAux.", "LVL1EmTauRoIsAux.",
"TrigNavigationAux.",
"BTagging_AntiKt4EMTopoJFVtxAux.", "HLT_xAOD__BTagVertexContainer_BjetVertexFexAux.",
"xTrigDecisionAux.",
"HLT_xAOD__TrigPassBitsContainer_passbitsAux.",
"Kt4EMPFlowEventShapeAux.", "Kt4EMTopoOriginEventShapeAux.", "Kt4LCTopoOriginEventShapeAux.", "NeutralParticleFlowIsoCentralEventShapeAux.", "NeutralParticleFlowIsoForwardEventShapeAux.", "ParticleFlowIsoCentralEventShapeAux.", "ParticleFlowIsoForwardEventShapeAux.", "TopoClusterIsoCentralEventShapeAux.", "TopoClusterIsoForwardEventShapeAux.", "TopoClusterIsoVeryForwardEventShapeAux.",
"HLT_xAOD__L2CombinedMuonContainer_MuonL2CBInfoAux.",
"HLT_xAOD__TrigTrackCountsContainer_trackcountsAux.",
"HLT_xAOD__MuonRoIContainer_L1TopoMuonAux.", "LVL1MuonRoIsAux.",
"METMap_TruthAux.",
"PhotonsAux.",                       
"LVL1EnergySumRoIAux.",
"ElectronCaloRingsAux.",                   
"HLT_xAOD__TrigT2MbtsBitsContainer_T2MbtsAux.",
"SlowMuonsAux.",
"HLT_xAOD__TrigMissingETContainer_EFJetEtSumAux.", "HLT_xAOD__TrigMissingETContainer_EFMissingET_Fex_2sidednoiseSupp_PUCAux.", "HLT_xAOD__TrigMissingETContainer_T2MissingETAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingETAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_FEBAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mhtAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht_emAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topoclAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PSAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PUCAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_trkmhtAux.", "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_trkmhtFTKAux.", "HLT_xAOD__TrigMissingETContainer_TrigL2MissingET_FEBAux.",
"TruthEventsAux.",                    
"HLT_xAOD__RoiDescriptorStore_SplitJetAux.", "HLT_xAOD__RoiDescriptorStore_SuperRoiAux.", "HLT_xAOD__RoiDescriptorStore_forID1Aux.", "HLT_xAOD__RoiDescriptorStore_forID2Aux.", "HLT_xAOD__RoiDescriptorStore_forID3Aux.", "HLT_xAOD__RoiDescriptorStore_forIDAux.", "HLT_xAOD__RoiDescriptorStore_forMSAux.", "HLT_xAOD__RoiDescriptorStore_initialRoIAux.", "HLT_xAOD__RoiDescriptorStore_secondaryRoI_EFAux.", "HLT_xAOD__RoiDescriptorStore_secondaryRoI_HLTAux.", "HLT_xAOD__RoiDescriptorStore_secondaryRoI_L2Aux.",
"HLT_xAOD__TrigRNNOutputContainer_TrigRingerNeuralFexAux.", "HLT_xAOD__TrigRNNOutputContainer_TrigTRTHTCountsAux.",
"DiTauJetsAux.",
"MuonSegmentsAux.", "MuonTruthSegmentsAux.", "NCB_MuonSegmentsAux.",
"HLT_xAOD__TrigBphysContainer_EFBMuMuFexAux.", "HLT_xAOD__TrigBphysContainer_EFBMuMuXFexAux.", "HLT_xAOD__TrigBphysContainer_EFBMuMuXFex_XAux.", "HLT_xAOD__TrigBphysContainer_EFDsPhiPiFexAux.", "HLT_xAOD__TrigBphysContainer_EFMuPairsAux.", "HLT_xAOD__TrigBphysContainer_EFMultiMuFexAux.", "HLT_xAOD__TrigBphysContainer_EFTrackMassAux.", "HLT_xAOD__TrigBphysContainer_L2BMuMuFexAux.", "HLT_xAOD__TrigBphysContainer_L2BMuMuXFexAux.", "HLT_xAOD__TrigBphysContainer_L2DiMuXFexAux.", "HLT_xAOD__TrigBphysContainer_L2DsPhiPiFexDsAux.", "HLT_xAOD__TrigBphysContainer_L2DsPhiPiFexPhiAux.", "HLT_xAOD__TrigBphysContainer_L2JpsieeFexAux.", "HLT_xAOD__TrigBphysContainer_L2MultiMuFexAux.", "HLT_xAOD__TrigBphysContainer_L2TrackMassAux.",
"AntiKt4EMPFlowJetsAux.", "AntiKt4EMTopoJetsAux.", "AntiKt4LCTopoJetsAux.",
"HLT_xAOD__L2StandAloneMuonContainer_MuonL2SAInfoAux.",
"ElectronsAux.", "ForwardElectronsAux.",     
"JetETMissChargedParticleFlowObjectsAux.", "JetETMissNeutralParticleFlowObjectsAux.", "TauChargedParticleFlowObjectsAux.", "TauHadronicParticleFlowObjectsAux.", "TauNeutralParticleFlowObjectsAux.", "TauShotParticleFlowObjectsAux.",
"BTagging_AntiKt4EMTopoAux.",
"HLT_xAOD__ElectronContainer_egamma_ElectronsAux.", "HLT_xAOD__ElectronContainer_egamma_Iso_ElectronsAux.",
"HLT_xAOD__TrigCompositeContainer_ExpressInfo_HLTAux.", "HLT_xAOD__TrigCompositeContainer_L1TopoCompositeAux.", "HLT_xAOD__TrigCompositeContainer_L1TopoMETAux.", "HLT_xAOD__TrigCompositeContainer_MuonRoIClusterAux.", "HLT_xAOD__TrigCompositeContainer_TrigEFDielectronMassFexAux.",
#"HLT_xAOD__BTaggingContainer_HLTBjetFexAux.",
#"MuonTruthParticlesAux.", 
"TruthParticlesAux.", "egammaTruthParticlesAux.",
"HLT_xAOD__HIEventShapeContainer_HIUEAux.",
"ElectronRingSetsAux.",
"HLT_xAOD__TrigVertexCountsContainer_vertexcountsAux.",
"BTagging_AntiKt4EMTopoSecVtxAux.", "GSFConversionVerticesAux.", "HLT_xAOD__VertexContainer_BjetSecondaryVertexFexAux.", "HLT_xAOD__VertexContainer_EFHistoPrmVtxAux.", "HLT_xAOD__VertexContainer_PrimVertexFTKAux.", "HLT_xAOD__VertexContainer_PrimVertexFTKRawAux.", "HLT_xAOD__VertexContainer_PrimVertexFTKRefitAux.", "HLT_xAOD__VertexContainer_SecondaryVertexAux.", "HLT_xAOD__VertexContainer_xPrimVxAux.", "MSDisplacedVertexAux.", "PrimaryVerticesAux.", "TauSecondaryVerticesAux.",
#"HLT_xAOD__CaloClusterContainer_TrigEFCaloCalibFexAux.",
"HLT_xAOD__JetRoIContainer_L1TopoJetAux.", "LVL1JetRoIsAux.",
"HLT_xAOD__TrigRingerRingsContainer_TrigT2CaloEgammaAux.",
"TauTracksAux.",
"EventInfoAux.",                          
"HLT_xAOD__MuonContainer_MuonEFInfoAux.", "HLT_xAOD__MuonContainer_MuonEFInfo_FullScanAux.", "HLT_xAOD__MuonContainer_MuonEFInfo_MSonlyTrackParticles_FullScanAux.", "MuonsAux.", "StausAux.",
"LVL1JetEtRoIAux.",        
"HLT_xAOD__TrigSpacePointCountsContainer_spacepointsAux.",
"HLT_xAOD__TrigElectronContainer_L2ElectronFexAux.",
"CaloCalTopoClustersAux.", "ForwardElectronClustersAux.", "InDetTrackParticlesAssociatedClustersAux.", "MuonClusterCollectionAux.", "TauPi0ClustersAux.", "egammaClustersAux.",
"CombinedMuonTrackParticlesAux.", "CombinedStauTrackParticlesAux.", 
#"ExtrapolatedMuonTrackParticlesAux.", 
"ExtrapolatedStauTrackParticlesAux.", "GSFTrackParticlesAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_FTKAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_FTKRefitAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_FTKRefit_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_FTK_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bjet_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_BphysHighPt_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bphysics_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Electron_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_FullScan_FTKAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_FullScan_FTKRefitAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTKAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTKRefitAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTKRefit_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTK_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_FTKAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_FTKRefitAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_FTKRefit_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Tau_FTK_IDTrigAux.", "HLT_xAOD__TrackParticleContainer_MuonEFInfo_CombTrackParticlesAux.", "HLT_xAOD__TrackParticleContainer_MuonEFInfo_CombTrackParticles_FullScanAux.", "HLT_xAOD__TrackParticleContainer_MuonEFInfo_ExtrapTrackParticlesAux.", "HLT_xAOD__TrackParticleContainer_MuonEFInfo_ExtrapTrackParticles_FullScanAux.", "InDetForwardTrackParticlesAux.", "InDetPixelPrdAssociationTrackParticlesAux.", "InDetTrackParticlesAux.", "MSOnlyExtrapolatedMuonTrackParticlesAux.", "MSonlyTrackletsAux.", "MuonSpectrometerTrackParticlesAux.",
"HLT_xAOD__PhotonContainer_egamma_Iso_PhotonsAux.", "HLT_xAOD__PhotonContainer_egamma_PhotonsAux.",
"METAssoc_AntiKt4EMPFlowAux.", "METAssoc_AntiKt4EMTopoAux.", "METAssoc_AntiKt4LCTopoAux.",
 ])
