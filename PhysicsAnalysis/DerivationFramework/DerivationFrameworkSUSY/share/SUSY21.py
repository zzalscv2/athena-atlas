#********************************************************************
# SUSY21.py
# Derivation for Displaced Track based search for WIMPs
#********************************************************************

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkFlavourTag.FlavourTagCommon import *

# adds TightEventCleaning flag on EventInfo, copied and adjusted from ExtendendJetCommon.py
def eventCleanTight_xAODCollSUSY21(jetalg='AntiKt4EMTopo',sequence=DerivationFrameworkJob):
    from JetSelectorTools.JetSelectorToolsConf import ECUtils__EventCleaningTool as EventCleaningTool
    from JetSelectorTools.JetSelectorToolsConf import EventCleaningTestAlg
    
    prefix = "DFCommonJets_"
    ecToolTight = EventCleaningTool('SUSY21EventCleaningTool_Tight',
                                     CleaningLevel='TightBad',
                                     NJets=1,
                                     DoDecorations=False # decorations already set in ExtendendJetCommon.py
                                     )

    ecToolTight.JetCleanPrefix = prefix
    ecToolTight.JetCleaningTool = getJetCleaningTool("TightBad")
    algCleanTight = EventCleaningTestAlg('SUSY21EventCleaningTestAlg_Tight',
                                          EventCleaningTool=ecToolTight,
                                          JetCollectionName="AntiKt4EMTopoJets",
                                          EventCleanPrefix="DFSUSY21_",
                                          CleaningLevel="TightBad",
                                          doEvent=True)
    sequence += algCleanTight


### Set up stream
streamName = derivationFlags.WriteDAOD_SUSY21Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_SUSY21Stream )
SUSY21Stream = MSMgr.NewPoolRootStream( streamName, fileName )
SUSY21Stream.AcceptAlgs(["SUSY21KernelSkim"])

### Init
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
SUSY21ThinningHelper = ThinningHelper( "SUSY21ThinningHelper" )
thinningTools       = []
AugmentationTools   = []


# stream-specific sequence for on-the-fly jet building
SeqSUSY21 = CfgMgr.AthSequencer("SeqSUSY21")
DerivationFrameworkJob += SeqSUSY21


#====================================================================
# Trigger navigation thinning
#====================================================================
from DerivationFrameworkSUSY.SUSY21TriggerList import triggersNavThin

SUSY21ThinningHelper.TriggerChains = '|'.join(triggersNavThin)
SUSY21ThinningHelper.AppendToStream( SUSY21Stream )


#====================================================================
# THINNING TOOLS 
#====================================================================
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__InDetTrackSelectionToolWrapper

SUSY21TrackSelection = DerivationFramework__InDetTrackSelectionToolWrapper(name = "SUSY21TrackSelection",
                                                                                 ContainerName = "InDetTrackParticles",
                                                                                 DecorationName = "DFLoose" )
SUSY21TrackSelection.TrackSelectionTool.CutLevel = "Loose"
ToolSvc += SUSY21TrackSelection

AugmentationTools.append(SUSY21TrackSelection)

thinning_expression = "InDetTrackParticles.DFLoose && (InDetTrackParticles.pt > 0.5*GeV) && (abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta) ) < 3.0)"

# TrackParticles directly
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
SUSY21TPThinningTool = DerivationFramework__TrackParticleThinning(name = "SUSY21TPThinningTool",
                                                                 ThinningService         = SUSY21ThinningHelper.ThinningSvc(),
                                                                 SelectionString         = thinning_expression,
                                                                 InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += SUSY21TPThinningTool
thinningTools.append(SUSY21TPThinningTool)

# TrackParticles associated with Muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
SUSY21MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name                    = "SUSY21MuonTPThinningTool",
                                                                         ThinningService         = SUSY21ThinningHelper.ThinningSvc(),
                                                                         MuonKey                 = "Muons",
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += SUSY21MuonTPThinningTool
thinningTools.append(SUSY21MuonTPThinningTool)

# TrackParticles associated with electrons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
SUSY21ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name                     = "SUSY21ElectronTPThinningTool",
                                                                                 ThinningService        = SUSY21ThinningHelper.ThinningSvc(),
                                                                                 SGKey              = "Electrons",
                                                                                 InDetTrackParticlesKey = "InDetTrackParticles")
ToolSvc += SUSY21ElectronTPThinningTool
thinningTools.append(SUSY21ElectronTPThinningTool)

# Photon thinning
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__GenericObjectThinning
SUSY21PhotonThinningTool = DerivationFramework__GenericObjectThinning( name             = "SUSY21PhotonThinningTool",
                                                                      ThinningService  = SUSY21ThinningHelper.ThinningSvc(),
                                                                      ContainerName    = "Photons",
                                                                      ApplyAnd         = False,
                                                                      SelectionString  = "Photons.pt > 10*GeV")
ToolSvc += SUSY21PhotonThinningTool
thinningTools.append(SUSY21PhotonThinningTool)

# TrackParticles associated with photons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
SUSY21PhotonTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name       = "SUSY21PhotonTPThinningTool",
                                                                              ThinningService  = SUSY21ThinningHelper.ThinningSvc(),
                                                                              SGKey      = "Photons",
                                                                              InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += SUSY21PhotonTPThinningTool
thinningTools.append(SUSY21PhotonTPThinningTool)


#====================================================================
# TRUTH THINNING
#====================================================================
if DerivationFrameworkHasTruth:
  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__MenuTruthThinning
  SUSY21TruthThinningTool = DerivationFramework__MenuTruthThinning(name              = "SUSY21TruthThinningTool",
                                                                     ThinningService              = SUSY21ThinningHelper.ThinningSvc(),
                                                                     WritePartons                 = False,
                                                                     WriteHadrons                 = True,
                                                                     WriteBHadrons                = True,
                                                                     WriteGeant                   = False,
                                                                     GeantPhotonPtThresh          = 20000,
                                                                     WriteTauHad                  = True,
                                                                     PartonPtThresh               = -1.0,
                                                                     WriteBSM                     = True,
                                                                     WriteBosons                  = True,
                                                                     WriteBosonProducts           = False,
                                                                     WriteBSMProducts             = True,
                                                                     WriteTopAndDecays            = True,
                                                                     WriteEverything              = False,
                                                                     WriteAllLeptons              = True,
                                                                     WriteLeptonsNotFromHadrons   = False,
                                                                     WriteStatus3                 = False,
                                                                     WriteFirstN                  = 10,
                                                                     PreserveAncestors            = True,
                                                                     PreserveGeneratorDescendants = True,
                                                                     SimBarcodeOffset             = DerivationFrameworkSimBarcodeOffset)

  ToolSvc += SUSY21TruthThinningTool
  thinningTools.append(SUSY21TruthThinningTool)


#====================================================================
# Jet building
#====================================================================
#re-tag PFlow jets so they have b-tagging info.
FlavorTagInit(JetCollections = ['AntiKt4EMPFlowJets'], Sequencer = SeqSUSY21)

#====================================================================
# SKIMMING TOOL 
#====================================================================

#-------------------------------------------------------------
# 1e + 1gamma skimming for Z->2egamma
#-------------------------------------------------------------
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
electronRequirements_Z2eg = '(Electrons.pt > 30.*GeV) && (abs(Electrons.eta) < 2.5) && (Electrons.DFCommonElectronsLHMedium)'
photonRequirements_Z2eg = '(DFCommonPhotons_et > 30.*GeV) && (abs(DFCommonPhotons_eta) < 2.6)'
countSelection_Z2eg = '((count('+electronRequirements_Z2eg+')==1) && (count('+photonRequirements_Z2eg+')==1))'

from DerivationFrameworkEGamma.DerivationFrameworkEGammaConf import DerivationFramework__EGInvariantMassTool
SUSY21_EGAM_MassTool = DerivationFramework__EGInvariantMassTool( name = "SUSY21_EGAM_MassTool",
                                                          Object1Requirements = electronRequirements_Z2eg,
                                                          Object2Requirements = photonRequirements_Z2eg,
                                                          StoreGateEntryName = "SUSY21_ElectronPhotonMass",
                                                          Mass1Hypothesis = 0.511*MeV,
                                                          Mass2Hypothesis = 0*MeV,
                                                          Container1Name = "Electrons",
                                                          Container2Name = "Photons",
                                                          Pt2BranchName = "DFCommonPhotons_pt",
                                                          Eta2BranchName = "DFCommonPhotons_eta",
                                                          Phi2BranchName = "DFCommonPhotons_phi",
                                                          CheckCharge = False,
                                                          DoTransverseMass = False,
                                                          MinDeltaR = 0.0)
ToolSvc += SUSY21_EGAM_MassTool
#AugmentationTools.append(SUSY21_EGAM_MassTool)

massRequirements_Z2eg = 'count(SUSY21_ElectronPhotonMass > 50.0*GeV)==1'
selection_Z2eg = '('+countSelection_Z2eg+' && '+massRequirements_Z2eg+')'

SUSY21EGammaSkimmingTool_Z2eg = DerivationFramework__xAODStringSkimmingTool( name = "SUSY21EGammaSkimmingTool_Z2eg",
                                                                             expression = selection_Z2eg)
ToolSvc += SUSY21EGammaSkimmingTool_Z2eg

# Trigger skimming
# ------------------------------------------------------------
from DerivationFrameworkSUSY.SUSY21TriggerList import triggersMET, triggersSingleLepton, triggersPhoton
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool

trigReq_Z2eg=triggersSingleLepton+triggersPhoton
SUSY21TriggerSkimmingTool_Z2eg = DerivationFramework__TriggerSkimmingTool( name = "SUSY21TriggerSkimmingTool_Z2eg",
                                                                           TriggerListOR = trigReq_Z2eg)
ToolSvc += SUSY21TriggerSkimmingTool_Z2eg


# Final skim selection, with trigger selection and jet selection
# ------------------------------------------------------------
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND, DerivationFramework__FilterCombinationOR

SUSY21SkimmingTool_Z2eg = DerivationFramework__FilterCombinationAND(name = "SUSY21SkimmingTool_Z2eg",
                                                                    FilterList = [SUSY21EGammaSkimmingTool_Z2eg, SUSY21TriggerSkimmingTool_Z2eg])
ToolSvc += SUSY21SkimmingTool_Z2eg


#====================================================================
# SUSY skimming selection
#====================================================================
# run CPU-intensive algorithms afterwards to restrict those to skimmed events
SeqSUSY21 += CfgMgr.DerivationFramework__DerivationKernel(
    "SUSY21KernelSkim",
    AugmentationTools = [SUSY21_EGAM_MassTool],
    SkimmingTools = [SUSY21SkimmingTool_Z2eg]
)


#====================================================================
# Tight Event Cleaning
#====================================================================
eventCleanTight_xAODCollSUSY21("AntiKt4EMTopo")


#====================================================================
# Truth collections
#====================================================================
# copied from PHYS.py to ensure having consistent standard Truth containers
if DerivationFrameworkHasTruth:
   from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents,addMiniTruthCollectionLinks,addHFAndDownstreamParticles,addPVCollection,addTausAndDownstreamParticles
   import DerivationFrameworkHiggs.TruthCategories
   # Add charm quark collection
   from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthCollectionMaker
   SUSY21TruthCharmTool = DerivationFramework__TruthCollectionMaker(name                  = "SUSY21TruthCharmTool",
                                                                  NewCollectionName       = "TruthCharm",
                                                                  KeepNavigationInfo      = False,
                                                                  ParticleSelectionString = "(abs(TruthParticles.pdgId) == 4)",
                                                                  Do_Compress             = True)
   ToolSvc += SUSY21TruthCharmTool
   from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__CommonAugmentation
   SeqSUSY21 += CfgMgr.DerivationFramework__CommonAugmentation("SUSY21TruthCharmKernel",AugmentationTools=[SUSY21TruthCharmTool])
   # Add HF particles
   addHFAndDownstreamParticles(SeqSUSY21)
   #Add custom tau collection with 2 generation below (To save photon information)
   addTausAndDownstreamParticles(SeqSUSY21, generations=2)
   # Add standard truth
   addStandardTruthContents(SeqSUSY21,prefix='')

   # Update to include charm quarks and HF particles - require a separate instance to be train safe
   from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthNavigationDecorator
   SUSY21TruthNavigationDecorator = DerivationFramework__TruthNavigationDecorator( name="SUSY21TruthNavigationDecorator",
                                                                                   InputCollections=["TruthElectrons", "TruthMuons", "TruthPhotons", "TruthTaus","TruthNeutrinos", "TruthBSM", "TruthBottom", "TruthTop", "TruthBoson","TruthCharm","TruthHFWithDecayParticles","TruthTauWithDecayParticles"])
   ToolSvc += SUSY21TruthNavigationDecorator
   SeqSUSY21.MCTruthNavigationDecoratorKernel.AugmentationTools = [SUSY21TruthNavigationDecorator]
   # Re-point links on reco objects
   addMiniTruthCollectionLinks(SeqSUSY21)
   addPVCollection(SeqSUSY21)
   # Set appropriate truth jet collection for tau truth matching
   ToolSvc.DFCommonTauTruthMatchingTool.TruthJetContainerName = "AntiKt4TruthDressedWZJets"
   # Add sumOfWeights metadata for LHE3 multiweights =======
   from DerivationFrameworkCore.LHE3WeightMetadata import *


#====================================================================
# Track Isolation Decorations ( copied from DerivationFrameworkMuons/TrackIsolationDecorator.py )
#====================================================================

from IsolationTool.IsolationToolConf import xAOD__TrackIsolationTool
SUSY21TrackIsoTool = xAOD__TrackIsolationTool("SUSY21TrackIsoTool")
SUSY21TrackIsoTool.TrackSelectionTool.maxZ0SinTheta= 1.5
SUSY21TrackIsoTool.TrackSelectionTool.maxD0= 1.5
SUSY21TrackIsoTool.TrackSelectionTool.minPt= 1000.
SUSY21TrackIsoTool.TrackSelectionTool.CutLevel= "TightPrimary"
ToolSvc += SUSY21TrackIsoTool

from IsolationCorrections.IsolationCorrectionsConf import CP__IsolationCorrectionTool
SUSY21IsoCorrectionTool = CP__IsolationCorrectionTool (name = "SUSY21IsoCorrectionTool", IsMC = DerivationFrameworkHasTruth)
ToolSvc += SUSY21IsoCorrectionTool

# tool to collect topo clusters in cone
from ParticlesInConeTools.ParticlesInConeToolsConf import xAOD__CaloClustersInConeTool
SUSY21CaloClustersInConeTool = xAOD__CaloClustersInConeTool("SUSY21CaloClustersInConeTool",CaloClusterLocation = "CaloCalTopoClusters")
ToolSvc += SUSY21CaloClustersInConeTool

from CaloIdentifier import SUBCALO

from IsolationTool.IsolationToolConf import xAOD__CaloIsolationTool
SUSY21CaloIsoTool = xAOD__CaloIsolationTool("SUSY21CaloIsoTool")
SUSY21CaloIsoTool.IsoLeakCorrectionTool = ToolSvc.SUSY21IsoCorrectionTool
SUSY21CaloIsoTool.ClustersInConeTool = ToolSvc.SUSY21CaloClustersInConeTool
SUSY21CaloIsoTool.EMCaloNums  = [SUBCALO.LAREM]
SUSY21CaloIsoTool.HadCaloNums = [SUBCALO.LARHEC, SUBCALO.TILE]
SUSY21CaloIsoTool.UseEMScale  = True
SUSY21CaloIsoTool.UseCaloExtensionCaching = False
SUSY21CaloIsoTool.saveOnlyRequestedCorrections = True
SUSY21CaloIsoTool.addCaloExtensionDecoration = False
ToolSvc += SUSY21CaloIsoTool

import ROOT, PyCintex
PyCintex.loadDictionary('xAODCoreRflxDict')
PyCintex.loadDictionary('xAODPrimitivesDict')
isoPar = ROOT.xAOD.Iso

# Calculate ptcone&ptvarcone, etcone&topoetcone
deco_ptcones = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20]
deco_topoetcones = [isoPar.topoetcone40, isoPar.topoetcone30, isoPar.topoetcone20]
deco_prefix = ''  #'SUSY21_'

from DerivationFrameworkSUSY.DerivationFrameworkSUSYConf import DerivationFramework__CaloIsolationDecorator
SUSY21IDTrackDecorator = DerivationFramework__CaloIsolationDecorator(name = "SUSY21IDTrackDecorator",
                                                                    TrackIsolationTool = SUSY21TrackIsoTool,
                                                                    CaloIsolationTool = SUSY21CaloIsoTool,
                                                                    TargetContainer = "InDetTrackParticles",
                                                                    SelectionString = "InDetTrackParticles.pt>.5*GeV",
                                                                    ptcones = deco_ptcones,
                                                                    topoetcones = deco_topoetcones,
                                                                    Prefix = deco_prefix,
                                                                    )
ToolSvc += SUSY21IDTrackDecorator
AugmentationTools.append(SUSY21IDTrackDecorator)

SUSY21MuonDecorator = DerivationFramework__CaloIsolationDecorator(name = "SUSY21MuonDecorator",
                                                                  TrackIsolationTool = SUSY21TrackIsoTool,
                                                                  CaloIsolationTool = SUSY21CaloIsoTool,
                                                                  TargetContainer = "Muons",
                                                                  topoetcones = deco_topoetcones,
                                                                  Prefix = "SUSY21_",
                                                              )
ToolSvc += SUSY21MuonDecorator
AugmentationTools.append(SUSY21MuonDecorator)

SUSY21ElectronDecorator = DerivationFramework__CaloIsolationDecorator(name = "SUSY21ElectronDecorator",
                                                                      TrackIsolationTool = SUSY21TrackIsoTool,
                                                                      CaloIsolationTool = SUSY21CaloIsoTool,
                                                                      TargetContainer = "Electrons",
                                                                      topoetcones = deco_topoetcones,
                                                                      Prefix = "SUSY21_",
                                                                  )
ToolSvc += SUSY21ElectronDecorator
AugmentationTools.append(SUSY21ElectronDecorator)

SUSY21PhotonDecorator = DerivationFramework__CaloIsolationDecorator(name = "SUSY21PhotonDecorator",
                                                                    TrackIsolationTool = SUSY21TrackIsoTool,
                                                                    CaloIsolationTool = SUSY21CaloIsoTool,
                                                                    TargetContainer = "Photons",
                                                                    topoetcones = deco_topoetcones,
                                                                    Prefix = "SUSY21_",
                                                                    )
ToolSvc += SUSY21PhotonDecorator
AugmentationTools.append(SUSY21PhotonDecorator)

#====================================================================
# Augment after skim
#====================================================================
SeqSUSY21 += CfgMgr.DerivationFramework__DerivationKernel(
  "SUSY21KernelAug",
  AugmentationTools = AugmentationTools,
  ThinningTools = thinningTools,
)

#====================================================================
# Prompt Lepton Tagger
#====================================================================
import JetTagNonPromptLepton.JetTagNonPromptLeptonConfig as JetTagConfig

# simple call to replaceAODReducedJets(["AntiKt4PV0TrackJets"], SeqSUSY21, "SUSY21")
JetTagConfig.ConfigureAntiKt4PV0TrackJets(SeqSUSY21, "SUSY21")

# Electron and Muon algorithms: PromptLeptonIso and PromptLeptonVeto
SeqSUSY21 += JetTagConfig.GetDecoratePromptLeptonAlgs()

# Tau algorithm: PromptTauVeto
SeqSUSY21 += JetTagConfig.GetDecoratePromptTauAlgs()


#====================================================================
# TrackAssociatedCaloSampleDecorator and 
# TrackCaloClusterDecorator_LowPtE
#====================================================================
from DerivationFrameworkMuons.DerivationFrameworkMuonsConf import DerivationFramework__TrackAssociatedCaloSampleDecorator

SUSY21_TrackAssociatedCaloSampleDecorator = DerivationFramework__TrackAssociatedCaloSampleDecorator(
  name             = "SUSY21_TrackAssociatedCaloSampleDecorator",
  ContainerName    = "InDetTrackParticles"
)
ToolSvc += SUSY21_TrackAssociatedCaloSampleDecorator

from DerivationFrameworkSUSY.DerivationFrameworkSUSYConf import DerivationFramework__TrackCaloClusterDecorator_LowPtE

SUSY21_TrackCaloClusterDecorator_LowPtE = DerivationFramework__TrackCaloClusterDecorator_LowPtE(
  name             = "SUSY21_TrackCaloClusterDecorator_LowPtE",
  ContainerName    = "InDetTrackParticles",
  Prefix           = "SUSY21_" 
)
ToolSvc += SUSY21_TrackCaloClusterDecorator_LowPtE

SeqSUSY21 += CfgMgr.DerivationFramework__DerivationKernel(
  "SUSY21KernelDeco",
    AugmentationTools = [SUSY21_TrackAssociatedCaloSampleDecorator, SUSY21_TrackCaloClusterDecorator_LowPtE]
)

#====================================================================
# CONTENT LIST  
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkSUSY.SUSY21ContentList import *

SUSY21SlimmingHelper = SlimmingHelper("SUSY21SlimmingHelper")
SUSY21SlimmingHelper.SmartCollections = ["Electrons", "Photons", "Muons",
                                         "AntiKt4EMTopoJets",
                                         "AntiKt4EMPFlowJets",
                                         "MET_Reference_AntiKt4EMPFlow",
                                         "PrimaryVertices",
                                         "AntiKt4EMPFlowJets_BTagging201903",
                                         "BTagging_AntiKt4EMPFlow_201903",
                                         "InDetTrackParticles"
                                         ]

SUSY21SlimmingHelper.AllVariables = ["MET_Truth", "TruthParticles", "TruthEvents", "TruthVertices"]
 

SUSY21SlimmingHelper.ExtraVariables = SUSY21ExtraVariables
SUSY21SlimmingHelper.ExtraVariables += JetTagConfig.GetExtraPromptVariablesForDxAOD()
SUSY21SlimmingHelper.ExtraVariables += JetTagConfig.GetExtraPromptTauVariablesForDxAOD()

SUSY21SlimmingHelper.IncludeMuonTriggerContent = True
SUSY21SlimmingHelper.IncludeEGammaTriggerContent = True
#SUSY21SlimmingHelper.IncludeJetTauEtMissTriggerContent = True
SUSY21SlimmingHelper.IncludeJetTriggerContent = True
SUSY21SlimmingHelper.IncludeTauTriggerContent = False

SUSY21SlimmingHelper.IncludeEtMissTriggerContent = True
SUSY21SlimmingHelper.IncludeBJetTriggerContent = False

# same Truth-related content as in PHYS.py to ensure having consistent standard Truth containers
if DerivationFrameworkHasTruth:

  SUSY21SlimmingHelper.AppendToDictionary = {'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
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
                                            'TruthTauWithDecayParticles':'xAOD::TruthParticleContainer','TruthTauWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthTauWithDecayVertices':'xAOD::TruthVertexContainer','TruthTauWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'HardScatterParticles':'xAOD::TruthParticleContainer','HardScatterParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'HardScatterVertices':'xAOD::TruthVertexContainer','HardScatterVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthHFWithDecayParticles':'xAOD::TruthParticleContainer','TruthHFWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthHFWithDecayVertices':'xAOD::TruthVertexContainer','TruthHFWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthCharm':'xAOD::TruthParticleContainer','TruthCharmAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'AntiKt10TruthTrimmedPtFrac5SmallR20Jets':'xAOD::JetContainer', 'AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux':'xAOD::JetAuxContainer',
                                            'AntiKt10TruthSoftDropBeta100Zcut10Jets':'xAOD::JetContainer', 'AntiKt10TruthSoftDropBeta100Zcut10JetsAux':'xAOD::JetAuxContainer'
                                           }

  from DerivationFrameworkMCTruth.MCTruthCommon import addTruth3ContentToSlimmerTool
  addTruth3ContentToSlimmerTool(SUSY21SlimmingHelper)
  SUSY21SlimmingHelper.AllVariables += ['TruthTauWithDecayParticles','TruthTauWithDecayVertices','TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm']

SUSY21SlimmingHelper.AppendContentToStream(SUSY21Stream)
