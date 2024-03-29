#********************************************************************
# HIGG6D2.py (for H+ -> tau-lep)
# reductionConf flag HIGG6D2 in Reco_tf.py   
#********************************************************************

# Set up common services and job object. 
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

#Include common variables from CP groups
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *

from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
applyJetCalibration_xAODColl(jetalg="AntiKt4EMTopo")

import AthenaCommon.SystemOfUnits as Units
from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.BeamFlags import jobproperties

# testing globalflags
is_MC = (globalflags.DataSource()=='geant4')
print "is_MC = ",is_MC
print "HIGG6D2.py globalflags.DataSource()", globalflags.DataSource()
print "HIGG6D2.py jobproperties.Beam.energy()", jobproperties.Beam.energy()

if is_MC:
  from DerivationFrameworkMCTruth.MCTruthCommon import *
  
#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_HIGG6D2Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_HIGG6D2Stream )
HIGG6D2Stream = MSMgr.NewPoolRootStream( streamName, fileName )
HIGG6D2Stream.AcceptAlgs(["HIGG6D2Kernel"])
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()

#====================================================================
# TAU SELECTOR TOOL 
#====================================================================
augmentationTools = []
from DerivationFrameworkTau.DerivationFrameworkTauConf import DerivationFramework__TauSelectionWrapper
HIGG6D2TauWrapper = DerivationFramework__TauSelectionWrapper(name = "HIGG6D2TauSelectionWrapper",
															IsTauFlag	        = 29,
															CollectionName		= "TauJets",
															StoreGateEntryName	= "HIGG6D2JetRNNSigLoose")
ToolSvc += HIGG6D2TauWrapper
augmentationTools.append(HIGG6D2TauWrapper)
#=======================================
# Tau truth matching tool
#=======================================

#truth matching
if is_MC:
    from TauAnalysisTools.TauAnalysisToolsConf import TauAnalysisTools__TauTruthMatchingTool
    HIGG6D2TauTruthMatchingTool = TauAnalysisTools__TauTruthMatchingTool(name="HIGG6D2TauTruthMatchingTool",
                                                                         WriteTruthTaus = True)


    ToolSvc += HIGG6D2TauTruthMatchingTool

    from DerivationFrameworkTau.DerivationFrameworkTauConf import DerivationFramework__TauTruthMatchingWrapper
    HIGG6D2TauTruthMatchingWrapper = DerivationFramework__TauTruthMatchingWrapper( name = "HIGG6D2TauTruthMatchingWrapper",
                                                                                TauTruthMatchingTool = HIGG6D2TauTruthMatchingTool,
                                                                                TauContainerName     = "TauJets")

    ToolSvc += HIGG6D2TauTruthMatchingWrapper
    augmentationTools.append(HIGG6D2TauTruthMatchingWrapper)


#====================================================================
# MC selection 
#====================================================================
MCselection = '1'
#if is_MC: MCselection = '(EventInfo.eventTypeBitmask==1)'

#====================================================================
# jet selection - jets with pt>20 and |eta|<2.6
#====================================================================
jetSelEM = ' count ((AntiKt4EMTopoJets.pt > 20.0*GeV) && (abs(AntiKt4EMTopoJets.eta) < 2.6)) >= 2'
jetSelLC = ' count ((AntiKt4LCTopoJets.pt > 20.0*GeV) && (abs(AntiKt4LCTopoJets.eta) < 2.6)) >= 2'
jetSelEMCalib = ' count ((AntiKt4EMTopoJets.DFCommonJets_Calib_pt > 20.0*GeV) && (abs(AntiKt4EMTopoJets.DFCommonJets_Calib_eta) < 2.6)) >= 2'
jetSel = '( '+ jetSelEM + ' ) || ( '+ jetSelLC + ' ) || ( '+ jetSelEMCalib + ' )'


#=============================================================================================
# tau selection - tau pt>25GeV, with Ntracks=1,3 and |q_\tau|=1 and |eta|<2.6
#=============================================================================================
tauSel = '(TauJets.pt > 25*GeV && (abs(TauJets.eta)<2.6) && (abs(TauJets.charge)<3) && ( (TauJets.nTracks == 1) || (TauJets.nTracks == 2) || (TauJets.nTracks == 3) ) )'

#====================================================================
# Trigger selection 
#====================================================================
if jobproperties.Beam.energy()==6500000.0:
    # 13 TeV name should be HLT_xxx
    singleElectronTriggerRequirement='(HLT_e28_lhtight_iloose || HLT_e60_lhmedium)'
    singleMuonTriggerRequirement='(HLT_mu26_imedium || HLT_mu50)'
    singleLepTrigger='('+singleElectronTriggerRequirement+'||'+singleMuonTriggerRequirement+')'
    TauMETtrigSel = '(HLT_tau35_medium1_tracktwo_L1TAU20_xe70_L1XE45 || HLT_tau35_medium1_tracktwo_xe70_L1XE45 || HLT_xe70 || HLT_xe80 || HLT_xe100 || HLT_j30_xe10_razor170 || HLT_j30_xe10_razor185  || HLT_j30_xe60_razor170 )'
    BjetTrigger = 'HLT_j75_bmedium_3j75'
elif jobproperties.Beam.energy()==4000000.0:
    # 8 TeV name should be EF_xxx
    singleElectronTriggerRequirement='(EF_e24vhi_medium1 || EF_e60_medium1)'
    singleMuonTriggerRequirement='(EF_mu24i_tight || EF_mu36_tight)'
    singleLepTrigger='('+singleElectronTriggerRequirement+'||'+singleMuonTriggerRequirement+')'
    TauMETtrigSel = '(tau29T_medium1_xe40_tight || tau29T_medium1_xe50tclcw_tight || tau27T_medium1_L2loose_xe50_tclcw_tight)'
 

  
#========================================================================
# lepton selection
#========================================================================
electronRequirements = '( (Electrons.pt > 25*GeV) && (abs(Electrons.eta) < 2.6) && (Electrons.Loose || Electrons.DFCommonElectronsLHLoose) )'
muonRequirements = '( (Muons.pt > 25.0*GeV) && (abs(Muons.eta) < 2.6) && (Muons.DFCommonGoodMuon) )'
lepSel = '( (count( ' + electronRequirements + ' ) >=1)  || (count( ' + muonRequirements + ' ) >=1 ) )'


#====================================================================
# SKIMMING TOOL 
#====================================================================
#expression = '(count( '+ lepSel + ' ) >= 1) && (count( ' + tauSel +  ' ) >= 1) && ( ' +MCselection + ' ) && ( ' + singleLepTrigger +  ' )'
expression = '( '+ jetSel + ' ) && ( '+ lepSel + ' ) && (count( ' + tauSel +  ' ) >= 1) && ( ' +MCselection + ' )'

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
HIGG6D2SkimmingTool = DerivationFramework__xAODStringSkimmingTool( name = "HIGG6D2SkimmingTool1",
                                                                    expression = expression)
ToolSvc += HIGG6D2SkimmingTool
print HIGG6D2SkimmingTool


#====================================================================
# THINNING TOOLS
#====================================================================

thinningTools=[]

#====================================================================
# GenericTrackParticleThinning
#====================================================================
thinning_expression = "(InDetTrackParticles.pt > 0.5*GeV) && (InDetTrackParticles.numberOfPixelHits > 0) && (InDetTrackParticles.numberOfSCTHits > 5) && (abs(DFCommonInDetTrackZ0AtPV) < 1.5)"


#====================================================================
# JetTrackParticleThinning
#====================================================================
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
HIGG6D2JetTPThinningTool = DerivationFramework__JetTrackParticleThinning(name                    = "HIGG6D2JetTPThinningTool",
            StreamName              = streamName,
            JetKey                  = "AntiKt4EMTopoJets",
            InDetTrackParticlesKey  = "InDetTrackParticles",
            TrackSelectionString = thinning_expression)


thinningTools.append(HIGG6D2JetTPThinningTool)
ToolSvc += HIGG6D2JetTPThinningTool


#====================================================================
# TauTrackParticleThinning
#====================================================================
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TauTrackParticleThinning
HIGG6D2TauTPThinningTool = DerivationFramework__TauTrackParticleThinning(name                    = "HIGG6D2TauTPThinningTool",
            StreamName              = streamName,
            TauKey                       = "TauJets",
            InDetTrackParticlesKey  = "InDetTrackParticles",
#            SelectionString = tauSel,
            ConeSize = 0.6)

thinningTools.append(HIGG6D2TauTPThinningTool)
ToolSvc += HIGG6D2TauTPThinningTool

#====================================================================
# MuonTrackParticleThinning
#====================================================================
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
HIGG6D2MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name= "HIGG6D2MuonTPThinningTool",
            StreamName              = streamName,
            MuonKey                 = "Muons",
            InDetTrackParticlesKey  = "InDetTrackParticles",
#            SelectionString = muonRequirements,
            ConeSize = 0.4)

thinningTools.append(HIGG6D2MuonTPThinningTool)
ToolSvc += HIGG6D2MuonTPThinningTool

#====================================================================
# EgammaTrackParticleThinning
#====================================================================
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
HIGG6D2ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name= "HIGG6D2ElectronTPThinningTool",
            StreamName              = streamName,
            SGKey                   = "Electrons",
            InDetTrackParticlesKey  = "InDetTrackParticles",
#            SelectionString = electronRequirements,
            ConeSize = 0.4)

thinningTools.append(HIGG6D2ElectronTPThinningTool)
ToolSvc += HIGG6D2ElectronTPThinningTool

#====================================================================
# Tracks themselves
#====================================================================
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
HIGG6D2TPThinningTool = DerivationFramework__TrackParticleThinning(name                    = "HIGG6D2TPThinningTool",
                                                                   StreamName              = streamName,
                                                                   SelectionString         = "abs( DFCommonInDetTrackZ0AtPV * sin(InDetTrackParticles.theta)) < 3.0",
                                                                   InDetTrackParticlesKey  = "InDetTrackParticles")

thinningTools.append(HIGG6D2TPThinningTool)
ToolSvc += HIGG6D2TPThinningTool

#====================================================================
# Truth particles thinning
#====================================================================

from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__MenuTruthThinning
HIGG6D2TruthThinningTool = DerivationFramework__MenuTruthThinning(name          = "HIGG6D2TruthThinningTool",
                                                     StreamName                 = streamName,
                                                     WritePartons               = False,
                                                     WriteHadrons               = False,
                                                     WriteBHadrons              = True,
                                                     WriteGeant                 = False,
                                                     GeantPhotonPtThresh        = -1.0,
                                                     WriteTauHad                = True,
                                                     PartonPtThresh             = -1.0,
                                                     WriteBSM                   = True,
                                                     WriteBosons                = True,
                                                     WriteBSMProducts           = True,
                                                     WriteTopAndDecays          = True,
                                                     WriteEverything            = False,
                                                     WriteAllLeptons            = False,
                                                     WriteLeptonsNotFromHadrons = True,
                                                     WriteStatus3               = True,
                                                     WriteFirstN                = -1,
                                                     PreserveGeneratorDescendants = True,
                                                     PreserveDescendants = False)

if is_MC:
    ToolSvc += HIGG6D2TruthThinningTool
    thinningTools.append(HIGG6D2TruthThinningTool)


#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel("HIGG6D2Kernel",
                                    AugmentationTools = augmentationTools,
                                    SkimmingTools = [HIGG6D2SkimmingTool],
                                    ThinningTools = thinningTools
                                    )


 
#====================================================================
# Add the containers to the output stream - slimming done here (smart slimming)
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
HIGG6D2SlimmingHelper = SlimmingHelper("HIGG6D2SlimmingHelper")

## Smart Slimming
HIGG6D2SlimmingHelper.SmartCollections = ["Electrons",
                                      "Muons",
                                      "TauJets",
                                      "MET_Reference_AntiKt4LCTopo",
                                      "MET_Reference_AntiKt4EMTopo",
                                      "AntiKt4LCTopoJets",
                                      "AntiKt4EMTopoJets",
                                      "BTagging_AntiKt4LCTopo",
                                      "BTagging_AntiKt4EMTopo",
                                      "InDetTrackParticles",
                                      "PrimaryVertices" ]


## Add extra variables
HIGG6D2SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_Calib_pt.DFCommonJets_Calib_eta"]
HIGG6D2SlimmingHelper.ExtraVariables += ["TauJets.BDTEleScore.pantau_CellBasedInput_isPanTauCandidate.pantau_CellBasedInput_DecayMode.ptPanTauCellBased.etaPanTauCellBased.phiPanTauCellBased.mPanTauCellBased.pantau_CellBasedInput_BDTValue_1p0n_vs_1p1n.pantau_CellBasedInput_BDTValue_1p1n_vs_1pXn.pantau_CellBasedInput_BDTValue_3p0n_vs_3pXn","TauNeutralParticleFlowObjects.pt.eta.phi.m.rapidity.bdtPi0Score"]
HIGG6D2SlimmingHelper.ExtraVariables += ["Electrons.DFCommonElectronsLHLoose.DFCommonElectronsLHMedium.DFCommonElectronsLHTight.DFCommonElectronsML.author.OQ.charge.LHLoose.LHMedium.LHTight.LHValue"]
HIGG6D2SlimmingHelper.ExtraVariables += ["Muons.DFCommonGoodMuons","CombinedMuonTrackParticles.d0.z0.vz","InDetTrackParticles.numberOfTRTHits.numberOfTRTOutliers"]
HIGG6D2SlimmingHelper.ExtraVariables += ["PrimaryVertices.x.y.z.vertexType"]

if is_MC:
  HIGG6D2SlimmingHelper.StaticContent = ["xAOD::TruthParticleContainer#TruthMuons","xAOD::TruthParticleAuxContainer#TruthMuonsAux.","xAOD::TruthParticleContainer#TruthElectrons","xAOD::TruthParticleAuxContainer#TruthElectronsAux.","xAOD::TruthParticleContainer#TruthPhotons","xAOD::TruthParticleAuxContainer#TruthPhotonsAux.","xAOD::TruthParticleContainer#TruthNeutrinos","xAOD::TruthParticleAuxContainer#TruthNeutrinosAux.","xAOD::TruthParticleContainer#TruthTaus","xAOD::TruthParticleAuxContainer#TruthTausAux."]  
  HIGG6D2SlimmingHelper.AllVariables = ["TruthParticles","TruthEvents","MET_Truth","METMap_Truth","TruthVertices","AntiKt4TruthJets"]
  HIGG6D2SlimmingHelper.ExtraVariables += ["AntiKt4LCTopoJets.PartonTruthLabelID.TruthLabelDeltaR_B.TruthLabelDeltaR_C.TruthLabelDeltaR_T"]
  HIGG6D2SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.PartonTruthLabelID.TruthLabelDeltaR_B.TruthLabelDeltaR_C.TruthLabelDeltaR_T"]
  HIGG6D2SlimmingHelper.ExtraVariables += ["Electrons.truthOrigin.truthType.truthParticleLink","MuonTruthParticles.truthOrigin.truthType"]
  HIGG6D2SlimmingHelper.ExtraVariables += ["TauJets.IsTruthMatched.truthParticleLink"]

# Add MET_RefFinalFix
addMETOutputs(HIGG6D2SlimmingHelper,["AntiKt4LCTopo","AntiKt4EMTopo","Track"])
HIGG6D2SlimmingHelper.IncludeMuonTriggerContent = True
HIGG6D2SlimmingHelper.IncludeEGammaTriggerContent = True
HIGG6D2SlimmingHelper.IncludeEtMissTriggerContent = True
HIGG6D2SlimmingHelper.IncludeTauTriggerContent = True
HIGG6D2SlimmingHelper.IncludeJetTriggerContent = True
                                        
HIGG6D2SlimmingHelper.AppendContentToStream(HIGG6D2Stream)
