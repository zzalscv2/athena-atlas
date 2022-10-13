# *****************************************************
# HDBS5.py 
# reductionConf flag HDBS5 in Reco_tf.py  
DAOD_StreamID = 'HDBS5' 
# *****************************************************
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from AthenaCommon.GlobalFlags import globalflags
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkFlavourTag.FlavourTagCommon import *

if DerivationFrameworkHasTruth:
  from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
  addStandardTruthContents()


# =============================================
# Private sequence here
# =============================================
HDBS5seq = CfgMgr.AthSequencer("HDBS5Sequence")

#b-tag AntiKt4PFlowJets
FlavorTagInit(JetCollections = ['AntiKt4EMPFlowJets'], Sequencer = HDBS5seq)


# =============================================
# Set up stream
# =============================================
streamName      = derivationFlags.WriteDAOD_HDBS5Stream.StreamName
fileName        = buildFileName( derivationFlags.WriteDAOD_HDBS5Stream )
HDBS5Stream     = MSMgr.NewPoolRootStream( streamName, fileName )
HDBS5Stream.AcceptAlgs(["HDBS5Kernel"])

# =============================================
# Ditau mass decoration
# =============================================
from DerivationFrameworkTau.DerivationFrameworkTauConf import DerivationFramework__DiTauMassDecorator
DiTauMassDecorator = DerivationFramework__DiTauMassDecorator(
    name              = "DiTauMassDecorator",
    DiTauContainerName  = "DiTauJetsLowPt",
    )
ToolSvc += DiTauMassDecorator


# =============================================
# Thinning tool
# =============================================
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
HDBS5ThinningHelper                              = ThinningHelper( "HDBS5ThinningHelper" )
HDBS5ThinningHelper.TriggerChains                = '(^(?!.*_[0-9]*(tau|mu|j|xe|e|b|perf|idperf))(?!HLT_g.*_[0-9]*g.*)(HLT_2*g.*))'
HDBS5ThinningHelper.AppendToStream( HDBS5Stream )

thinningTools = []

# MET/Jet tracks
thinning_expression     = "(InDetTrackParticles.pt > 0.5*GeV) && (InDetTrackParticles.numberOfPixelHits > 0) && (InDetTrackParticles.numberOfSCTHits > 5) && (abs(DFCommonInDetTrackZ0AtPV) < 1.5)"

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
HDBS5MetTPThinningTool    = DerivationFramework__TrackParticleThinning(
  name                      = "HDBS5MetTPThinningTool",
  ThinningService           = HDBS5ThinningHelper.ThinningSvc(),
  SelectionString           = thinning_expression,
  InDetTrackParticlesKey    = "InDetTrackParticles",
  ApplyAnd                  = True)
ToolSvc  += HDBS5MetTPThinningTool
thinningTools.append(HDBS5MetTPThinningTool)

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
HDBS5JetTPThinningTool    = DerivationFramework__JetTrackParticleThinning(
  name                      = "HDBS5JetTPThinningTool",
  ThinningService           = HDBS5ThinningHelper.ThinningSvc(),
  JetKey                    = "AntiKt4EMPFlowJets",
  InDetTrackParticlesKey    = "InDetTrackParticles",
  ApplyAnd                  = True)
ToolSvc   += HDBS5JetTPThinningTool
thinningTools.append(HDBS5JetTPThinningTool)

# Tracks themselves (inspired by HIGG1D2)
HDBS5TPThinningTool    = DerivationFramework__TrackParticleThinning(
  name                      = "HDBS5TPThinningTool",
  ThinningService           = HDBS5ThinningHelper.ThinningSvc(),
  SelectionString           = "abs( DFCommonInDetTrackZ0AtPV * sin(InDetTrackParticles.theta)) < 3.0",
  InDetTrackParticlesKey    = "InDetTrackParticles")
ToolSvc  += HDBS5TPThinningTool
thinningTools.append(HDBS5TPThinningTool)

# Tracks associated with electrons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
HDBS5ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(
  name                      = "HDBS5ElectronTPThinningTool",
  ThinningService           = HDBS5ThinningHelper.ThinningSvc(),
  SGKey                     = "Electrons",
  InDetTrackParticlesKey    = "InDetTrackParticles")
ToolSvc += HDBS5ElectronTPThinningTool
thinningTools.append(HDBS5ElectronTPThinningTool)

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
HDBS5PhotonTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(
  name                    = "HDBS5PhotonTPThinningTool",
  ThinningService         = HDBS5ThinningHelper.ThinningSvc(),
  SGKey                   = "Photons",
  SelectionString 	  = "",
  InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += HDBS5PhotonTPThinningTool
thinningTools.append(HDBS5PhotonTPThinningTool)

# Tracks associated with muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
HDBS5MuonTPThinningTool   = DerivationFramework__MuonTrackParticleThinning(
  name                      = "HDBS5MuonTPThinningTool",
  ThinningService           = HDBS5ThinningHelper.ThinningSvc(),
  MuonKey                   = "Muons",
  InDetTrackParticlesKey    = "InDetTrackParticles")
ToolSvc += HDBS5MuonTPThinningTool
thinningTools.append(HDBS5MuonTPThinningTool)

# Tracks associated with taus
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TauTrackParticleThinning
HDBS5TauTPThinningTool  = DerivationFramework__TauTrackParticleThinning(
  name                    = "HDBS5TauTPThinningTool",
  ThinningService         = HDBS5ThinningHelper.ThinningSvc(),
  TauKey                  = "TauJets",
  InDetTrackParticlesKey  = "InDetTrackParticles",
  ConeSize                = 0.6)
ToolSvc += HDBS5TauTPThinningTool
thinningTools.append(HDBS5TauTPThinningTool)

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__DiTauTrackParticleThinning
HDBS5DiTauTPThinningToolLowPt = DerivationFramework__DiTauTrackParticleThinning(
  name               = "HDBS5DiTauLowPtTPThinningTool",
  ThinningService         = HDBS5ThinningHelper.ThinningSvc(),
  DiTauKey                = "DiTauJetsLowPt",
  SelectionString         = "",
  InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += HDBS5DiTauTPThinningToolLowPt
thinningTools.append(HDBS5DiTauTPThinningToolLowPt)

# truth thinning here:
import DerivationFrameworkTau.TAUPThinningHelper 
HDBS5TruthThinningTools = DerivationFrameworkTau.TAUPThinningHelper.setup("HDBS5",
                                                                          HDBS5ThinningHelper.ThinningSvc(),
                                                                          ToolSvc)
thinningTools += HDBS5TruthThinningTools


# =============================================
# Skimming tool
# =============================================
gReq = '(count (Photons.pt > 0.0*GeV && abs(Photons.eta) < 2.5) >=2 ) '
eReq = '( Electrons.pt > 10.0*GeV && abs(Electrons.eta) < 2.5 && Electrons.DFCommonElectronsLHTight )'
muReq = '( Muons.pt > 10.0*GeV && abs(Muons.eta) < 2.5 && Muons.DFCommonMuonsPreselection )' 
trigger_ph = '(HLT_g140_loose || HLT_g300_etcut || HLT_2g50_loose_L12EM20VH || HLT_2g25_loose_g15_loose || HLT_g35_medium_g25_medium_L12EM20VH || HLT_2g25_tight_L12EM20VH || HLT_2g22_tight_L12EM15VHI || HLT_g35_loose_g25_loose || HLT_g35_medium_g25_medium || HLT_2g50_loose || HLT_2g20_tight || HLT_2g22_tight || HLT_2g20_tight_icalovloose_L12EM15VHI || HLT_2g20_tight_icalotight_L12EM15VHI || HLT_2g22_tight_L12EM15VHI || HLT_2g22_tight_icalovloose_L12EM15VHI || HLT_2g22_tight_icalotight_L12EM15VHI || HLT_2g22_tight_icalovloose || HLT_2g25_tight_L12EM20VH || HLT_2g20_loose || HLT_2g20_loose_L12EM15 || HLT_g35_medium_g25_medium || HLT_g35_medium_g25_medium_L12EM15VH || HLT_g35_loose_g25_loose || HLT_g35_loose_g25_loose_L12EM15VH || HLT_2g20_loose_g15_loose || HLT_3g20_loose || HLT_3g15_loose || HLT_2g6_tight_icalotight_L1J100 || HLT_2g6_loose_L1J100 || HLT_2g6_tight_icalotight_L1J50 || HLT_2g6_loose_L1J50 || HLT_g120_loose || HLT_g140_loose)' #single photon and di-photon
noLep = '( (count('+eReq+') == 0) && (count('+muReq+') == 0) )' #Lepton veto (tight elec, loose mu)
ditau = '( count( (DiTauJetsLowPt.pt > 50.0*GeV) && (DiTauJetsLowPt.nSubjets > 1) ) >= 1 )'
singletau = '( count( (TauJets.pt > 15.0*GeV || TauJets.ptFinalCalib > 15.0*GeV ) && (abs(TauJets.eta) < 2.6) && (abs(TauJets.charge)==1.0 && (TauJets.nTracks == 1 || TauJets.nTracks == 3)) ) >= 2 )'
skim_expression = gReq + "&&" + noLep + "&&" + trigger_ph + "&&(" + ditau + "||" + singletau + ")"
# # Keep photon triggers only
# skim_expression = trigger_ph

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
HDBS5SkimmingTool = DerivationFramework__xAODStringSkimmingTool(
  name          = "HDBS5SkimmingTool",
  expression    = skim_expression)

ToolSvc += HDBS5SkimmingTool

# =============================================
# Jet and Ditau reconstruction
# =============================================

reducedJetList = ["AntiKt4TruthJets","AntiKt4TruthWZJets","AntiKt4PV0TrackJets", "AntiKt2PV0TrackJets"]
replaceAODReducedJets(reducedJetList,HDBS5seq, "HDBS5")
from DerivationFrameworkTau.TauTruthCommon import *
addCHSPFlowObjects()
addStandardJets("AntiKt", 1.0, "EMPFlow", ptmin=50000, ptminFilter=50000, mods="pflow_ungroomed", calibOpt="none", algseq=HDBS5seq, outputGroup=DAOD_StreamID)
import DiTauRec.DiTauAlgorithmsHolder as DiTauAlgs
from DiTauRec.DiTauRecConf import DiTauBuilder

ditauTools = []
ditauTools.append(DiTauAlgs.getSeedJetBuilder("AntiKt10EMPFlowJets"))
ditauTools.append(DiTauAlgs.getElMuFinder())
ditauTools.append(DiTauAlgs.getSubjetBuilder())
ditauTools.append(DiTauAlgs.getVertexFinder())
ditauTools.append(DiTauAlgs.getDiTauTrackFinder())
ditauTools.append(DiTauAlgs.getIDVarCalculator(False))

DiTauBuilderBase = DiTauBuilder(
    name="DiTauBuilderLowPt",
    DiTauContainer="DiTauJetsLowPt",
    DiTauAuxContainer="DiTauJetsLowPtAux.",
    Tools=ditauTools,
    SeedJetName="AntiKt10EMPFlowJets",
    minPt=50000,
    maxEta=2.5,
    OutputLevel=2,
    Rjet=1.0,
    Rsubjet=0.2,
    Rcore=0.1)
HDBS5seq += DiTauBuilderBase


#====================================================================
# Diphoton vertex decoration tool
#====================================================================

# Creates a shallow copy of PrimaryVertices (HggPrimaryVertices) for diphoton events
# Must be created before the jetalg in the sequence as it is input to the modified PFlow jets
from RecExConfig.RecFlags  import rec
from egammaRec.Factories import ToolFactory, AlgFactory
import PhotonVertexSelection.PhotonVertexSelectionConf as PVS 

PhotonPointingTool = ToolFactory(PVS.CP__PhotonPointingTool, name = "yyVtxPhotonPointingTool", isSimulation = rec.doTruth() )
PhotonVertexSelectionTool = ToolFactory(PVS.CP__PhotonVertexSelectionTool, PhotonPointingTool = PhotonPointingTool)

from DerivationFrameworkHiggs.DerivationFrameworkHiggsConf import DerivationFramework__DiphotonVertexDecorator
DiphotonVertexDecorator = ToolFactory(DerivationFramework__DiphotonVertexDecorator, PhotonVertexSelectionTool = PhotonVertexSelectionTool)()
DerivationFrameworkJob += CfgMgr.DerivationFramework__CommonAugmentation("DiphotonVertexKernel", AugmentationTools = [DiphotonVertexDecorator])

from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkMuons.MuonsCommon import *

import AthenaCommon.SystemOfUnits as Units
from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.BeamFlags import jobproperties

if globalflags.DataSource()=='geant4':
    from DerivationFrameworkHiggs.TruthCategories import *

from DerivationFrameworkCore.LHE3WeightMetadata import *

if DerivationFrameworkHasTruth:
    from DerivationFrameworkMCTruth.MCTruthCommon import *
    addStandardTruthContents()
    addPVCollection()
    print "HDBS5.py Applying MCTruthCommon"


# =============================================
# Create derivation Kernel
# =============================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
HDBS5seq += CfgMgr.DerivationFramework__DerivationKernel(
  "HDBS5Kernel",
  AugmentationTools         = [DiTauMassDecorator],
  SkimmingTools             = [HDBS5SkimmingTool],
  ThinningTools             = thinningTools
  )
DerivationFrameworkJob += HDBS5seq


# =============================================
# Add the containers to the output stream (slimming done here)
# =============================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkTau.TAUPExtraContent import *
HDBS5SlimmingHelper       = SlimmingHelper("HDBS5SlimmingHelper")
HDBS5SlimmingHelper.SmartCollections = ["Electrons",
                                        "Photons",
                                        "Muons",
                                        "TauJets",
                                        "MET_Reference_AntiKt4EMPFlow",  
                                        "MET_Reference_AntiKt4EMTopo",
                                        "AntiKt4EMTopoJets",
                                        "AntiKt4EMTopoJets_BTagging201810", 
                                        "BTagging_AntiKt4EMTopo_201810",
                                        "AntiKt4EMPFlowJets",                                        
                                        "AntiKt4EMPFlowJets_BTagging201810",
                                        "AntiKt4EMPFlowJets_BTagging201903",
                                        "BTagging_AntiKt4EMPFlow_201810",
                                        "BTagging_AntiKt4EMPFlow_201903",
                                        "InDetTrackParticles",
                                        "PrimaryVertices"]

if DerivationFrameworkHasTruth:
  HDBS5SlimmingHelper.StaticContent  = ["xAOD::TruthParticleContainer#TruthBoson",
                                        "xAOD::TruthParticleAuxContainer#TruthBosonAux.",
                                        "xAOD::TruthParticleContainer#TruthElectrons",
                                        "xAOD::TruthParticleAuxContainer#TruthElectronsAux.",
                                        "xAOD::TruthParticleContainer#TruthMuons",
                                        "xAOD::TruthParticleAuxContainer#TruthMuonsAux.",
                                        "xAOD::TruthParticleContainer#TruthTaus",
                                        "xAOD::TruthParticleAuxContainer#TruthTausAux.",
                                        "xAOD::TruthParticleContainer#TruthPhotons",
                                        "xAOD::TruthParticleAuxContainer#TruthPhotonsAux.",
                                        "xAOD::TruthParticleContainer#TruthNeutrinos",
                                        "xAOD::TruthParticleAuxContainer#TruthNeutrinosAux."]

HDBS5SlimmingHelper.IncludeMuonTriggerContent    = False
HDBS5SlimmingHelper.IncludeTauTriggerContent     = False
HDBS5SlimmingHelper.IncludeEGammaTriggerContent  = True
HDBS5SlimmingHelper.IncludeEtMissTriggerContent  = False
HDBS5SlimmingHelper.IncludeJetTriggerContent     = False
HDBS5SlimmingHelper.IncludeBJetTriggerContent    = False


## add ditau container
HDBS5SlimmingHelper.ExtraVariables               = ExtraContentTAUP5
HDBS5SlimmingHelper.AllVariables                 = ExtraContainersTAUP5

addJetOutputs(HDBS5SlimmingHelper, [DAOD_StreamID], ['AntiKt4TruthJets', 'AntiKt4TruthWZJets'], ['AntiKt4PV0TrackJets','AntiKt2PV0TrackJets','AntiKt10LCTopoJets'])
HDBS5SlimmingHelper.AppendToDictionary["DiTauJetsLowPt"] = 'xAOD::DiTauJetContainer' #Add in the ditaus
HDBS5SlimmingHelper.AppendToDictionary["DiTauJetsLowPtAux"] = 'xAOD::DiTauJetAuxContainer' #Add in the ditaus
HDBS5SlimmingHelper.AllVariables += ["DiTauJetsLowPt"] #Add in the ditaus
HDBS5SlimmingHelper.ExtraVariables += ["DiTauJetsLowPt.LSLMass"] #Add ditau lead+sublead subjet system invariant mass decoration because why not

if DerivationFrameworkHasTruth:
  HDBS5SlimmingHelper.ExtraVariables            += ExtraContentTruthTAUP5
  HDBS5SlimmingHelper.AllVariables              += ExtraContainersTruthTAUP5


## diphoton vertex
HDBS5SlimmingHelper.AppendToDictionary["HggPrimaryVertices"] = 'xAOD::VertexContainer'
HDBS5SlimmingHelper.AppendToDictionary["HggPrimaryVerticesAux"] = 'xAOD::ShallowAuxContainer'

HDBS5SlimmingHelper.AllVariables += ["HggPrimaryVertices"]


## append to stream
HDBS5SlimmingHelper.AppendContentToStream(HDBS5Stream)
HDBS5Stream.AddItem("xAOD::EventShape#*")
HDBS5Stream.AddItem("xAOD::EventShapeAuxInfo#*")

