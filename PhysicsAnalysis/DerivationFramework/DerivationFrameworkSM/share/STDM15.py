#********************************************************************
# STDM15.py 
# reductionConf flag STDM15 in Reco_tf.py   
#********************************************************************
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkCore.WeightMetadata import *
from DerivationFrameworkEGamma.EGammaCommon import *
import AthenaCommon.SystemOfUnits as Units
from DerivationFrameworkSM import STDMTriggers
from DerivationFrameworkFlavourTag.FlavourTagCommon import *

# Add sumOfWeights metadata for LHE3 multiweights =======
from DerivationFrameworkCore.LHE3WeightMetadata import *

# Add Truth MetaData
if DerivationFrameworkHasTruth:
    from DerivationFrameworkMCTruth.MCTruthCommon import *

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool

#====================================================================
# SET UP STREAM
#====================================================================

streamName = derivationFlags.WriteDAOD_STDM15Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_STDM15Stream )
STDM15Stream = MSMgr.NewPoolRootStream( streamName, fileName )
STDM15Stream.AcceptAlgs(["STDM15Kernel"])

#====================================================================
# THINNING TOOLS
#====================================================================

thinningTools=[]


#=====================
# TRIGGER NAV THINNING
#=====================

# Establish the thinning helper
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
STDM15ThinningHelper = ThinningHelper( "STDM15ThinningHelper" )

#trigger navigation content
STDM15ThinningHelper.TriggerChains = 'HLT_e.*|HLT_mu.*'
STDM15ThinningHelper.AppendToStream( STDM15Stream )


#===================== 
# TRACK  THINNING
#=====================  

# removed for full tracking info...
PHYS_thinning_expression = "abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 5*mm"

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
STDM15TPThinningTool = DerivationFramework__TrackParticleThinning(name                    =  "STDM15TPThinningTool",
                                                                  ThinningService = STDM15ThinningHelper.ThinningSvc(),
                                                                  SelectionString         = PHYS_thinning_expression, #"InDetTrackParticles.pt > 0.1*GeV && InDetTrackParticles.eta > -2.5 && InDetTrackParticles.eta < 2.5 " #&& abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) <= 5.0", # && (abs(DFCommonInDetTrackZ0AtPV-EventInfo.STDM15_PVz))<3.0",
                                                                  InDetTrackParticlesKey  =  "InDetTrackParticles")
#ToolSvc += STDM15TPThinningTool
#thinningTools.append(STDM15TPThinningTool)

#print "anastopoulos"
#print EventInfo.STDM15_PVz

# Truth leptons and their ancestors and descendants + final-state hadrons
truth_cond_boson = "((abs(TruthParticles.pdgId) == 22) || (abs(TruthParticles.pdgId) == 23) || (abs(TruthParticles.pdgId) == 24) || (abs(TruthParticles.pdgId) == 25) || (abs(TruthParticles.pdgId) == 35) || (abs(TruthParticles.pdgId) == 36))"
truth_cond_lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 14) &&(TruthParticles.pt > 1*GeV) && (TruthParticles.status ==1) && (TruthParticles.barcode<200000))"
# Truth hadrons for UE analysis
truth_cond_hadrons = "( (TruthParticles.status ==1) && (TruthParticles.barcode<200000) && (abs(TruthParticles.pdgId) == 2212))"


from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning

if globalflags.DataSource()=='geant4':
    from DerivationFrameworkSM.STDMCommonTruthTools import *
    from DerivationFrameworkMCTruth.MCTruthCommon import addTruthHSDeco
    addTruthHSDeco()
   
    STDM15TruthLepTool = DerivationFramework__GenericTruthThinning(name                         = "STDM15TruthLepTool",
                                                                  ThinningService              = STDM15ThinningHelper.ThinningSvc(),
                                                                  ParticleSelectionString      = truth_cond_lepton,
                                                                  PreserveDescendants          = False,
                                                                  PreserveGeneratorDescendants = False,
                                                                  PreserveAncestors            = True)
   
    STDM15TruthBosTool = DerivationFramework__GenericTruthThinning(name                         = "STDM15TruthBosTool",
                                                                  ThinningService              = STDM15ThinningHelper.ThinningSvc(),
                                                                  ParticleSelectionString      = truth_cond_boson,
                                                                  PreserveDescendants          = False,
                                                                  PreserveGeneratorDescendants = True,
                                                                  PreserveAncestors            = False)
   
    STDM15PhotonThinning = DerivationFramework__GenericTruthThinning(name                    = "STDM15PhotonThinning",
                                                                    ThinningService         = STDM15ThinningHelper.ThinningSvc(),
                                                                    ParticlesKey            = "STDMTruthPhotons",
                                                                    ParticleSelectionString = STDMphotonthinningexpr)

    STDM15TruthHadTool = DerivationFramework__GenericTruthThinning(name                         = "STDM15TruthHadTool",
                                                                  ThinningService              = STDM15ThinningHelper.ThinningSvc(),
                                                                  ParticleSelectionString      = truth_cond_hadrons,
                                                                  PreserveDescendants          = False,
                                                                  PreserveGeneratorDescendants = False,
                                                                  PreserveAncestors            = False)


    ToolSvc += STDM15TruthLepTool
    ToolSvc += STDM15TruthBosTool
#    ToolSvc += STDM15PhotonThinning
    ToolSvc += STDM15TruthHadTool
    thinningTools.append(STDM15TruthLepTool)
    thinningTools.append(STDM15TruthBosTool)
#    thinningTools.append(STDM15PhotonThinning)
    thinningTools.append(STDM15TruthHadTool)
    
#====================================================================
# SKIMMING TOOL 
#====================================================================

filterVal = derivationFlags.WriteDAOD_STDM15Stream.nChFilter
#print "kristin " + str(filterVal)

muonsRequirements = '(Muons.pt >= 22*GeV) && (abs(Muons.eta) < 2.6) && (Muons.DFCommonMuonsPreselection) && (Muons.DFCommonGoodMuon)'
electronsRequirements = '(Electrons.pt > 22*GeV) && (abs(Electrons.eta) < 2.6) && (Electrons.DFCommonElectronsLHMedium)'
chargedParticleRequirements = '(TruthParticles.pt > 500) && (TruthParticles.barcode < 200000) && (TruthParticles.status == 1) && (TruthParticles.charge != 0) && (TruthParticles.theta > 0.163803) && (TruthParticles.theta < 2.97779) && (TruthParticles.HSBool)' #the theta cuts correspond to |eta|<2.5.  Often, theta = 0 or pi, so using eta directly causes and Floating Point Exception
muonOnlySelection = 'count('+muonsRequirements+') >=1'
electronOnlySelection = 'count('+electronsRequirements+') >= 1'
electronMuonSelection = '(count('+electronsRequirements+') + count('+muonsRequirements+')) < 3'
jetRequirements = '((count (AntiKt4EMPFlowJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMPFlowJets.DFCommonJets_Calib_eta) < 2.6) >= 2) || (count (AntiKt4EMTopoJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMTopoJets.DFCommonJets_Calib_eta) < 2.6) >= 2)) && ((count (AntiKt4EMPFlowJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMPFlowJets.DFCommonJets_Calib_eta) < 2.6) <5) || (count (AntiKt4EMTopoJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMTopoJets.DFCommonJets_Calib_eta) < 2.6) <5)) '

trackRequirements = 'count( (abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 5*mm)) <100 ' 

#jetRequirements = '(((((count (AntiKt4EMPFlowJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMPFlowJets.DFCommonJets_Calib_eta) < 2.6) >= 2) || (count (AntiKt4EMTopoJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMTopoJets.DFCommonJets_Calib_eta) < 2.6) >= 2)) && ((count (AntiKt4EMPFlowJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMPFlowJets.DFCommonJets_Calib_eta) < 2.6) <5) || (count (AntiKt4EMTopoJets.DFCommonJets_Calib_pt > 15*GeV && abs(AntiKt4EMTopoJets.DFCommonJets_Calib_eta) < 2.6) <5)) ) ||( count(AntiKt10LCTopoJets.pt > 150*GeV && abs(AntiKt10LCTopoJets.eta) < 2.8)>0) ) '


if filterVal > -1 and globalflags.DataSource()=='geant4':
    chargedParticleSelection = 'count('+chargedParticleRequirements+') < '+str(filterVal)
    offlineexpression = '(('+muonOnlySelection+' || '+electronOnlySelection+' || '+electronMuonSelection+') && ('+chargedParticleSelection+') && ('+jetRequirements+') && ('+trackRequirements+'))'
else:
    offlineexpression = '(('+muonOnlySelection+' || '+electronOnlySelection+' || '+electronMuonSelection+') && ('+jetRequirements+') && ('+trackRequirements+'))'

MuonTriggerRequirement=['HLT_mu20_iloose_L1MU15', 'HLT_mu24_imedium', 'HLT_mu26_ivarmedium', 'HLT_mu24_iloose']

from RecExConfig.InputFilePeeker import inputFileSummary
triggerRequirement=STDMTriggers.single_e_triggers  + STDMTriggers.single_mu_triggers
#MuonTriggerRequirement

#from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
STDM15SkimmingTool_Trig = DerivationFramework__TriggerSkimmingTool( name = "STDM15SkimmingTool_Trig", TriggerListOR = triggerRequirement )
ToolSvc += STDM15SkimmingTool_Trig

#from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
STDM15SkimmingTool_offsel = DerivationFramework__xAODStringSkimmingTool( name = "STDM15SkimmingTool_offsel", expression = offlineexpression )
ToolSvc += STDM15SkimmingTool_offsel

#from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
#STDM15SkimmingTool=DerivationFramework__FilterCombinationAND(name="STDM15SkimmingTool", FilterList=[STDM15SkimmingTool_Trig,STDM15SkimmingTool_offsel] )
#ToolSvc+=STDM15SkimmingTool

#STDM15SkimmingTool_ntrk = DerivationFramework__xAODStringSkimmingTool( name = "STDM15SkimmingTool_ntrk", expression = "EventInfoAuxDyn.STDM15_nTracks<20" )
#ToolSvc += STDM15SkimmingTool_ntrk

#====================================================================
# count tracks around vertex window
#====================================================================
#nTrkVertexWindowTool = CfgMgr.DerivationFramework__TrackVtxWindowCounting( name = "STDM15TrackVtxWindowCounting",
#                                                                   OutputLevel    = DEBUG,
#                                                                   DecorationPrefix      = "STDM15_",
#                                                                   TrackContainerName = "InDetTrackParticles",
#								   WindowSize        = 1.0)
#ToolSvc += nTrkVertexWindowTool


#STDM15SkimmingTool_ntrk = DerivationFramework__xAODStringSkimmingTool( name = "STDM15SkimmingTool_ntrk", expression = "EventInfo.STDM15_nTracks<100" )
#ToolSvc += STDM15SkimmingTool_ntrk



from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND

#STDM15SkimmingTool=DerivationFramework__FilterCombinationAND(name="STDM15SkimmingTool", FilterList=[STDM15SkimmingTool_Trig,STDM15SkimmingTool_offsel,STDM15SkimmingTool_ntrk] )
STDM15SkimmingTool=DerivationFramework__FilterCombinationAND(name="STDM15SkimmingTool", FilterList=[STDM15SkimmingTool_Trig,STDM15SkimmingTool_offsel] )
ToolSvc+=STDM15SkimmingTool

ToolSvc += STDM15TPThinningTool
thinningTools.append(STDM15TPThinningTool)


#ToolSvc += CfgMgr.GoodRunsListSelectionTool("STDM15GRLTool",GoodRunsListVec=['DerivationFrameworkSM/data/Lowmu.grl.xml'])
#ToolSvc += CfgMgr.GoodRunsListSelectionTool("STDM15GRLTool",GoodRunsListVec=['DerivationFrameworkSM/data/Highmu.grl.xml'])

#====================================================================
# AUGMENTATION TOOLS
#====================================================================
#augmentationTools = []
#augmentationTools.append( nTrkVertexWindowTool )


#====================================================================
# count tracks around vertex window
#====================================================================



#====================================================================
# Track-to-Lepton vertex fitting
#====================================================================
#ExtraVariablesLeptonVertexFit = ''
#if af.fileinfos["eventdata_items"].__contains__(('xAOD::TrackParticleContainer', "LowPtRoITrackParticles")) :
#    import MagFieldServices.SetupField
#    vtxTool = CfgMgr.TrkToLeptonPVTool("STDM15TrkToLeptonPVTool",OutputLevel=INFO)
#    ToolSvc += vtxTool
    
    # the augmentation tool
#    fittingTool = CfgMgr.DerivationFramework__LeptonVertexFitting( name = "STDM15LeptonVertexFitting",
                                                                   #OutputLevel    = DEBUG,
#                                                                   ElectronContainerName = "Electrons",
#                                                                   MuonContainerName     = "Muons",
#                                                                   TrackContainerName    = "LowPtRoITrackParticles",
#                                                                   DecorationPrefix      = "STDM15_",
#                                                                   VtxFittingTool        = vtxTool)
#    ToolSvc += fittingTool
#    augmentationTools.append( fittingTool )
#
#    ExtraVariablesLeptonVertexFit += "LowPtRoITrackParticles.STDM15_LepTrkVtx_z.STDM15_LepTrkVtx_chi2.STDM15_LepTrkVtx_ndf"


#=====================================================
# CREATE AND SCHEDULE THE DERIVATION KERNEL ALGORITHM   
#=====================================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel

# CREATE THE PRIVATE SEQUENCE
STDM15Sequence = CfgMgr.AthSequencer("STDM15Sequence")

# ADD KERNEL
STDM15Sequence += CfgMgr.DerivationFramework__DerivationKernel("STDM15Kernel",
                                                              RunSkimmingFirst = False,
                                                           #   AugmentationTools = augmentationTools,
                                                              SkimmingTools = [STDM15SkimmingTool],
                                                              ThinningTools = thinningTools)

#                                                              AugmentationTools = augmentationTools)


# JET REBUILDING
OutputJets["STDM15"] = ["AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets","AntiKt10TruthTrimmedPtFrac5SmallR20Jets"]

reducedJetList = ["AntiKt4TruthJets","AntiKt10TruthTrimmedPtFrac5SmallR20Jets","AntiKt2PV0TrackJets","AntiKt4PV0TrackJets"]

if DerivationFrameworkHasTruth:

   OutputJets["STDM15"].append("AntiKt10TruthTrimmedPtFrac5SmallR20Jets")

   OutputJets["STDM15"].append("AntiKt10TruthSoftDropBeta100Zcut10Jets")


replaceAODReducedJets(reducedJetList, STDM15Sequence, "STDM15Jets")

add_largeR_truth_jets = DerivationFrameworkHasTruth #and not hasattr(STDM15Sequence,'jetalgAntiKt10TruthTrimmedPtFrac5SmallR20')

addDefaultTrimmedJets(STDM15Sequence,"STDM15Jets",dotruth=add_largeR_truth_jets)

# ADD SEQUENCE TO JOB
DerivationFrameworkJob += STDM15Sequence

#====================================================================
# SET UP STREAM   
#====================================================================
#streamName = derivationFlags.WriteDAOD_STDM15Stream.StreamName
#fileName   = buildFileName( derivationFlags.WriteDAOD_STDM15Stream )
#STDM15Stream = MSMgr.NewPoolRootStream( streamName, fileName )
#STDM15Stream.AcceptAlgs(["STDM15Kernel"])

# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
# from AthenaServices.Configurables import ThinningSvc, createThinningSvc
# augStream = MSMgr.GetStream( streamName )
# evtStream = augStream.GetEventStream()
# svcMgr += createThinningSvc( svcName="STDM15ThinningSvc", outStreams=[evtStream] )

#====================================================================
# Jet reconstruction/retagging
#====================================================================

#re-tag PFlow jets so they have b-tagging info.
FlavorTagInit(JetCollections = ['AntiKt4EMPFlowJets'], Sequencer = STDM15Sequence)

#improved fJVT
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import applyMVfJvtAugmentation,getPFlowfJVT
# MVfJvt #
applyMVfJvtAugmentation(jetalg='AntiKt4EMTopo',sequence=STDM15Sequence, algname='JetForwardJvtToolBDTAlg')
# PFlow fJvt #
getPFlowfJVT(jetalg='AntiKt4EMPFlow',sequence=STDM15Sequence, algname='JetForwardPFlowJvtToolAlg')


#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkSM.STDMExtraContent import *

STDM15SlimmingHelper = SlimmingHelper("STDM15SlimmingHelper")
STDM15SlimmingHelper.SmartCollections = ["Electrons",
                                        "Photons",
                                        "Muons",
                                        "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
					"AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
#                                        "MET_Reference_AntiKt4EMTopo", #
#                                        "AntiKt4EMTopoJets", #
                                        "MET_Reference_AntiKt4EMPFlow",
                                        #"AntiKt10LCTopoJets",
                                        "AntiKt4EMPFlowJets",
#                                        "BTagging_AntiKt4EMPFlow_201810",
                                        "BTagging_AntiKt4EMPFlow_201903",
#                                        "AntiKt4EMPFlowJets_BTagging201810", 
                                        "AntiKt4EMPFlowJets_BTagging201903",
                                        "InDetTrackParticles",
#                                        "LowPtRoITrackParticles",
#                                        "LowPtRoIVertexContainer",
                                        "PrimaryVertices"]

STDM15SlimmingHelper.IncludeEGammaTriggerContent = True
STDM15SlimmingHelper.IncludeMuonTriggerContent = True


#STDM15SlimmingHelper.ExtraVariables = ExtraContentAll
STDM15SlimmingHelper.ExtraVariables = ExtraContentSTDM15
STDM15SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.JetEMScaleMomentum_phi.JetEMScaleMomentum_m"]
STDM15SlimmingHelper.ExtraVariables += ["AntiKt4EMPFlowJets.OriginVertex"]
#STDM15SlimmingHelper.ExtraVariables += [ExtraVariablesLeptonVertexFit]

# btagging variables
from  DerivationFrameworkFlavourTag.BTaggingContent import *
#STDM15SlimmingHelper.ExtraVariables += BTaggingStandardContent("AntiKt4EMTopoJets")
STDM15SlimmingHelper.ExtraVariables += BTaggingStandardContent("AntiKt4EMPFlowJets")

#ExtraDictionary["BTagging_AntiKt4EMTopo"]     = "xAOD::BTaggingContainer"
#ExtraDictionary["BTagging_AntiKt4EMTopoAux"]  = "xAOD::BTaggingAuxContainer"
ExtraDictionary["BTagging_AntiKt4EMPFlow"]    = "xAOD::BTaggingContainer"
ExtraDictionary["BTagging_AntiKt4EMPFlowAux"] = "xAOD::BTaggingAuxContainer"

STDM15SlimmingHelper.AllVariables = [ "AFPSiHitContainer", "AFPToFHitContainer"]
STDM15SlimmingHelper.AllVariables += ExtraContainersAll
STDM15SlimmingHelper.AllVariables += ["Electrons"]

if globalflags.DataSource()=='geant4':
    STDM15SlimmingHelper.ExtraVariables += ExtraContentAllTruth_STDM15
    STDM15SlimmingHelper.AllVariables += ExtraContainersTruth_STDM15
    STDM15SlimmingHelper.AppendToDictionary.update(ExtraDictionary)

addMETOutputs(STDM15SlimmingHelper,["AntiKt4EMPFlow"])

STDM15Stream.AddItem("xAOD::AuxContainerBase#TruthParticlesAux.-polarizationPhi.-polarizationTheta.-m.-decayVtxLink.-prodVtxLink.-prodR.-prodZ.-z0.-d0.-phi.-z0st.-theta.-qOverP.-nPhotons_dressed")
STDM15Stream.AddItem("xAOD::AuxContainerBase#ElectronsAux.-DFCommonElectronsDNN_pcf.-DFCommonElectronsDNN_phf.-DFCommonElectronsDNN_plh.-DFCommonElectronsDNN_ppc.-DFCommonElectronsDNN_pel.-DFCommonElectronsDNN_ple.-DFCommonProdTrueZ.-DFCommonSimpleConvPhi.-DFCommonProdTruePhi.-DFCommonElectronsDNNMedium")

STDM15Stream.AddItem("xAOD::AuxContainerBase#MuonsAux.definingParametersCovMatrix")

STDM15Stream.AddItem("xAOD::AuxContainerBase#InDetTrackParticlesAux.truthMatchProbability.numberDoF.qOverP.theta.phi.z0.vz.definingParametersCovMatrix.d0.numberOfSCTDeadSensors.numberOfInnermostPixelLayerHits.expectInnermostPixelLayerHit.numberOfPixelHoles.numberOfPixelSharedHits.expectNextToInnermostPixelLayerHit.numberOfSCTDeadSensors.numberOfInnermostPixelLayerHits.numberOfPixelDeadSensors.numberOfSCTHoles.numberOfSCTSharedHits.numberOfNextToInnermostPixelLayerHits.numberOfPixelHits.numberOfSCTHits.numberOfTRTOutliers.numberOfTRTHits")

STDM15Stream.AddItem("xAOD::TruthVertexContainer#TruthVertices")
STDM15Stream.AddItem("xAOD::TruthVertexAuxContainer#TruthVerticesAux.-incomingParticleLinks.-x.-y.-t.-id.-barcode.-outgoingParticleLinks")

STDM15Stream.AddItem("xAOD::AuxContainerBase#InDetTrackParticlesAux.phi.chiSquared.numberDoF.expectInnermostPixelLayerHit.expectNextToInnermostPixelLayerHit.numberOfInnermostPixelLayerHits.numberOfNextToInnermostPixelLayerHits.numberOfPixelHits.numberOfPixelHoles.numberOfPixelSharedHits.numberOfPixelDeadSensors.numberOfSCTHits.numberOfSCTHoles.numberOfSCTSharedHits.numberOfSCTDeadSensors")

STDM15Stream.AddItem("xAOD::EventAuxInfo#EventInfoAux.-weight_CT10_0.-weight_CT10_1.-weight_CT10_2.-weight_CT10_3.-weight_CT10_4.-weight_CT10_5.-weight_CT10_6.-weight_CT10_7.-weight_CT10_8.-weight_CT10_9.-weight_CT10_10.-weight_CT10_11.-weight_CT10_12.-weight_CT10_13.-weight_CT10_14.-weight_CT10_15.-weight_CT10_16.-weight_CT10_17.-weight_CT10_18.-weight_CT10_19.-weight_CT10_20.-weight_CT10_21.-weight_CT10_22.-weight_CT10_23.-weight_CT10_24.-weight_CT10_25.-weight_CT10_26.-weight_CT10_27.-weight_CT10_28.-weight_CT10_29.-weight_CT10_30.-weight_CT10_31.-weight_CT10_32.-weight_CT10_33.-weight_CT10_34.-weight_CT10_35.-weight_CT10_36.-weight_CT10_37.-weight_CT10_38.-weight_CT10_39.-weight_CT10_40.-weight_CT10_41.-weight_CT10_42.-weight_CT10_43.-weight_CT10_44.-weight_CT10_45.-weight_CT10_46.-weight_CT10_47.-weight_CT10_48.-weight_CT10_49.-weight_CT10_50.-weight_CT10_51.-weight_CT10_52.-weight_MMHT2014nlo68cl_0.-weight_MMHT2014nlo68cl_1.-weight_MMHT2014nlo68cl_2.-weight_MMHT2014nlo68cl_3.-weight_MMHT2014nlo68cl_4.-weight_MMHT2014nlo68cl_5.-weight_MMHT2014nlo68cl_6.-weight_MMHT2014nlo68cl_7.-weight_MMHT2014nlo68cl_8.-weight_MMHT2014nlo68cl_9.-weight_MMHT2014nlo68cl_10.-weight_MMHT2014nlo68cl_11.-weight_MMHT2014nlo68cl_12.-weight_MMHT2014nlo68cl_13.-weight_MMHT2014nlo68cl_14.-weight_MMHT2014nlo68cl_15.-weight_MMHT2014nlo68cl_16.-weight_MMHT2014nlo68cl_17.-weight_MMHT2014nlo68cl_18.-weight_MMHT2014nlo68cl_19.-weight_MMHT2014nlo68cl_20.-weight_MMHT2014nlo68cl_21.-weight_MMHT2014nlo68cl_22.-weight_MMHT2014nlo68cl_23.-weight_MMHT2014nlo68cl_24.-weight_MMHT2014nlo68cl_25.-weight_MMHT2014nlo68cl_26.-weight_MMHT2014nlo68cl_27.-weight_MMHT2014nlo68cl_28.-weight_MMHT2014nlo68cl_29.-weight_MMHT2014nlo68cl_30.-weight_MMHT2014nlo68cl_31.-weight_MMHT2014nlo68cl_32.-weight_MMHT2014nlo68cl_33.-weight_MMHT2014nlo68cl_34.-weight_MMHT2014nlo68cl_35.-weight_MMHT2014nlo68cl_36.-weight_MMHT2014nlo68cl_37.-weight_MMHT2014nlo68cl_38.-weight_MMHT2014nlo68cl_39.-weight_MMHT2014nlo68cl_40.-weight_MMHT2014nlo68cl_41.-weight_MMHT2014nlo68cl_42.-weight_MMHT2014nlo68cl_43.-weight_MMHT2014nlo68cl_44.-weight_MMHT2014nlo68cl_45.-weight_MMHT2014nlo68cl_46.-weight_MMHT2014nlo68cl_47.-weight_MMHT2014nlo68cl_48.-weight_MMHT2014nlo68cl_49.-weight_MMHT2014nlo68cl_50.-weight_PDF4LHC15_nlo_30_0.-weight_PDF4LHC15_nlo_30_1.-weight_PDF4LHC15_nlo_30_2.-weight_PDF4LHC15_nlo_30_3.-weight_PDF4LHC15_nlo_30_4.-weight_PDF4LHC15_nlo_30_5.-weight_PDF4LHC15_nlo_30_6.-weight_PDF4LHC15_nlo_30_7.-weight_PDF4LHC15_nlo_30_8.-weight_PDF4LHC15_nlo_30_9.-weight_PDF4LHC15_nlo_30_10.-weight_PDF4LHC15_nlo_30_11.-weight_PDF4LHC15_nlo_30_12.-weight_PDF4LHC15_nlo_30_13.-weight_PDF4LHC15_nlo_30_14.-weight_PDF4LHC15_nlo_30_15.-weight_PDF4LHC15_nlo_30_16.-weight_PDF4LHC15_nlo_30_17.-weight_PDF4LHC15_nlo_30_18.-weight_PDF4LHC15_nlo_30_19.-weight_PDF4LHC15_nlo_30_20.-weight_PDF4LHC15_nlo_30_21.-weight_PDF4LHC15_nlo_30_22.-weight_PDF4LHC15_nlo_30_23.-weight_PDF4LHC15_nlo_30_24.-weight_PDF4LHC15_nlo_30_25.-weight_PDF4LHC15_nlo_30_26.-weight_PDF4LHC15_nlo_30_27.-weight_PDF4LHC15_nlo_30_28.-weight_PDF4LHC15_nlo_30_29.-weight_PDF4LHC15_nlo_30_30.-weight_NNPDF30_nlo_as_0118_0.-weight_NNPDF30_nlo_as_0118_1.-weight_NNPDF30_nlo_as_0118_2.-weight_NNPDF30_nlo_as_0118_3.-weight_NNPDF30_nlo_as_0118_4.-weight_NNPDF30_nlo_as_0118_5.-weight_NNPDF30_nlo_as_0118_6.-weight_NNPDF30_nlo_as_0118_7.-weight_NNPDF30_nlo_as_0118_8.-weight_NNPDF30_nlo_as_0118_9.-weight_NNPDF30_nlo_as_0118_10.-weight_NNPDF30_nlo_as_0118_11.-weight_NNPDF30_nlo_as_0118_12.-weight_NNPDF30_nlo_as_0118_13.-weight_NNPDF30_nlo_as_0118_14.-weight_NNPDF30_nlo_as_0118_15.-weight_NNPDF30_nlo_as_0118_16.-weight_NNPDF30_nlo_as_0118_17.-weight_NNPDF30_nlo_as_0118_18.-weight_NNPDF30_nlo_as_0118_19.-weight_NNPDF30_nlo_as_0118_20.-weight_NNPDF30_nlo_as_0118_21.-weight_NNPDF30_nlo_as_0118_22.-weight_NNPDF30_nlo_as_0118_23.-weight_NNPDF30_nlo_as_0118_24.-weight_NNPDF30_nlo_as_0118_25.-weight_NNPDF30_nlo_as_0118_26.-weight_NNPDF30_nlo_as_0118_27.-weight_NNPDF30_nlo_as_0118_28.-weight_NNPDF30_nlo_as_0118_29.-weight_NNPDF30_nlo_as_0118_30.-weight_NNPDF30_nlo_as_0118_31.-weight_NNPDF30_nlo_as_0118_32.-weight_NNPDF30_nlo_as_0118_33.-weight_NNPDF30_nlo_as_0118_34.-weight_NNPDF30_nlo_as_0118_35.-weight_NNPDF30_nlo_as_0118_36.-weight_NNPDF30_nlo_as_0118_37.-weight_NNPDF30_nlo_as_0118_38.-weight_NNPDF30_nlo_as_0118_39.-weight_NNPDF30_nlo_as_0118_40.-weight_NNPDF30_nlo_as_0118_41.-weight_NNPDF30_nlo_as_0118_42.-weight_NNPDF30_nlo_as_0118_43.-weight_NNPDF30_nlo_as_0118_44.-weight_NNPDF30_nlo_as_0118_45.-weight_NNPDF30_nlo_as_0118_46.-weight_NNPDF30_nlo_as_0118_47.-weight_NNPDF30_nlo_as_0118_48.-weight_NNPDF30_nlo_as_0118_49.-weight_NNPDF30_nlo_as_0118_50.-weight_NNPDF30_nlo_as_0118_51.-weight_NNPDF30_nlo_as_0118_52.-weight_NNPDF30_nlo_as_0118_53.-weight_NNPDF30_nlo_as_0118_54.-weight_NNPDF30_nlo_as_0118_55.-weight_NNPDF30_nlo_as_0118_56.-weight_NNPDF30_nlo_as_0118_57.-weight_NNPDF30_nlo_as_0118_58.-weight_NNPDF30_nlo_as_0118_59.-weight_NNPDF30_nlo_as_0118_60.-weight_NNPDF30_nlo_as_0118_61.-weight_NNPDF30_nlo_as_0118_62.-weight_NNPDF30_nlo_as_0118_63.-weight_NNPDF30_nlo_as_0118_64.-weight_NNPDF30_nlo_as_0118_65.-weight_NNPDF30_nlo_as_0118_66.-weight_NNPDF30_nlo_as_0118_67.-weight_NNPDF30_nlo_as_0118_68.-weight_NNPDF30_nlo_as_0118_69.-weight_NNPDF30_nlo_as_0118_70.-weight_NNPDF30_nlo_as_0118_71.-weight_NNPDF30_nlo_as_0118_72.-weight_NNPDF30_nlo_as_0118_73.-weight_NNPDF30_nlo_as_0118_74.-weight_NNPDF30_nlo_as_0118_75.-weight_NNPDF30_nlo_as_0118_76.-weight_NNPDF30_nlo_as_0118_77.-weight_NNPDF30_nlo_as_0118_78.-weight_NNPDF30_nlo_as_0118_79.-weight_NNPDF30_nlo_as_0118_80.-weight_NNPDF30_nlo_as_0118_81.-weight_NNPDF30_nlo_as_0118_82.-weight_NNPDF30_nlo_as_0118_83.-weight_NNPDF30_nlo_as_0118_84.-weight_NNPDF30_nlo_as_0118_85.-weight_NNPDF30_nlo_as_0118_86.-weight_NNPDF30_nlo_as_0118_87.-weight_NNPDF30_nlo_as_0118_88.-weight_NNPDF30_nlo_as_0118_89.-weight_NNPDF30_nlo_as_0118_90.-weight_NNPDF30_nlo_as_0118_91.-weight_NNPDF30_nlo_as_0118_92.-weight_NNPDF30_nlo_as_0118_93.-weight_NNPDF30_nlo_as_0118_94.-weight_NNPDF30_nlo_as_0118_95.-weight_NNPDF30_nlo_as_0118_96.-weight_NNPDF30_nlo_as_0118_97.-weight_NNPDF30_nlo_as_0118_98.-weight_NNPDF30_nlo_as_0118_99.-weight_NNPDF30_nlo_as_0118_100")
    
STDM15SlimmingHelper.AppendContentToStream(STDM15Stream)

