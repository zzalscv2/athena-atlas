#====================================================================
# JETM6.py
# reductionConf flag JETM6 in Reco_tf.py
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkIsMonteCarlo, DerivationFrameworkJob, buildFileName
from DerivationFrameworkJetEtMiss.JetCommon import addOriginCorrectedClusters
from DerivationFrameworkPhys import PhysCommon
from DerivationFrameworkTrigger.TriggerMatchingHelper import TriggerMatchingHelper

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM6Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM6Stream )
JETM6Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM6Stream.AcceptAlgs(["JETM6Kernel"])
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()

#====================================================================
# SKIMMING TOOL
#====================================================================

from DerivationFrameworkJetEtMiss import TriggerLists
electronTriggers = TriggerLists.single_el_Trig()
muonTriggers = TriggerLists.single_mu_Trig()
photonTriggers = TriggerLists.single_photon_Trig()
jetTriggers = TriggerLists.jetTrig()

# One electron or muon or high-pT photon + large-R jet or just a high-pT large-R jet

jetsofflinesel = '(count( AntiKt10LCTopoJets.pt > 400.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1 || count( AntiKt10UFOCSSKJets.pt > 400.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >= 1)'
if DerivationFrameworkIsMonteCarlo:
  jetsofflinesel = '(count( AntiKt10LCTopoJets.pt > 180.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1 || count( AntiKt10UFOCSSKJets.pt > 180.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >= 1)'

andstr = ' && '
jetsel_lep = '(count( AntiKt10LCTopoJets.pt > 150.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1 || count( AntiKt10UFOCSSKJets.pt > 150.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >= 1)'
elofflinesel = andstr.join(['count((Electrons.pt > 20*GeV) && (Electrons.DFCommonElectronsLHLoose)) >= 1',jetsel_lep])
muofflinesel = andstr.join(['count((Muons.pt > 20*GeV) && (Muons.DFCommonMuonPassPreselection)) >= 1',jetsel_lep])
gammaofflinesel = andstr.join(['count(Photons.pt > 150*GeV) >= 1',jetsel_lep])

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
JETM6OfflineSkimmingTool_ele   = DerivationFramework__xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_ele",
                                                                              expression = elofflinesel)
JETM6OfflineSkimmingTool_mu    = DerivationFramework__xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_mu",
                                                                              expression = muofflinesel)
JETM6OfflineSkimmingTool_gamma = DerivationFramework__xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_gamma",
                                                                              expression = gammaofflinesel)
JETM6OfflineSkimmingTool_jets  = DerivationFramework__xAODStringSkimmingTool( name = "JETM6OfflineSkimmingTool_jets",
                                                                              expression = jetsofflinesel)

ToolSvc += JETM6OfflineSkimmingTool_ele
ToolSvc += JETM6OfflineSkimmingTool_mu
ToolSvc += JETM6OfflineSkimmingTool_gamma
ToolSvc += JETM6OfflineSkimmingTool_jets

JETM6SkimmingTools = []

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationOR

if not DerivationFrameworkIsMonteCarlo:
  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
  JETM6TriggerSkimmingTool_ele   = DerivationFramework__TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_ele",   TriggerListOR = electronTriggers)
  ToolSvc += JETM6TriggerSkimmingTool_ele
  JETM6TriggerSkimmingTool_mu    = DerivationFramework__TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_mu",    TriggerListOR = muonTriggers)
  ToolSvc += JETM6TriggerSkimmingTool_mu
  JETM6TriggerSkimmingTool_gamma = DerivationFramework__TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_gamma", TriggerListOR = photonTriggers)
  ToolSvc += JETM6TriggerSkimmingTool_gamma
  JETM6TriggerSkimmingTool_jets  = DerivationFramework__TriggerSkimmingTool(name = "JETM6TriggerSkimmingTool_jets",  TriggerListOR = jetTriggers)
  ToolSvc += JETM6TriggerSkimmingTool_jets

  # Combine trigger and offline selection
  JETM6SkimmingTool_ele   = DerivationFramework__FilterCombinationAND(name="JETM6SkimmingTool_ele",   FilterList=[JETM6OfflineSkimmingTool_ele,   JETM6TriggerSkimmingTool_ele] )
  JETM6SkimmingTool_mu    = DerivationFramework__FilterCombinationAND(name="JETM6SkimmingTool_mu",    FilterList=[JETM6OfflineSkimmingTool_mu,    JETM6TriggerSkimmingTool_mu] )
  JETM6SkimmingTool_gamma = DerivationFramework__FilterCombinationAND(name="JETM6SkimmingTool_gamma", FilterList=[JETM6OfflineSkimmingTool_gamma, JETM6TriggerSkimmingTool_gamma] )
  JETM6SkimmingTool_jets  = DerivationFramework__FilterCombinationAND(name="JETM6SkimmingTool_jets",  FilterList=[JETM6OfflineSkimmingTool_jets,  JETM6TriggerSkimmingTool_jets] )

  ToolSvc += JETM6SkimmingTool_ele
  ToolSvc += JETM6SkimmingTool_mu
  ToolSvc += JETM6SkimmingTool_gamma
  ToolSvc += JETM6SkimmingTool_jets

  # Combine electron and muon channel
  JETM6SkimmingTool = DerivationFramework__FilterCombinationOR(name="JETM6SkimmingTool", 
                                                               FilterList=[JETM6SkimmingTool_ele, JETM6SkimmingTool_mu, JETM6SkimmingTool_gamma, JETM6SkimmingTool_jets])
  ToolSvc += JETM6SkimmingTool

  JETM6SkimmingTools += [JETM6SkimmingTool]

else:
  JETM6SkimmingTool = DerivationFramework__FilterCombinationOR(name="JETM6SkimmingTool",
                                                               FilterList=[JETM6OfflineSkimmingTool_ele,JETM6OfflineSkimmingTool_mu,JETM6OfflineSkimmingTool_gamma,JETM6OfflineSkimmingTool_jets])
  ToolSvc += JETM6SkimmingTool

  JETM6SkimmingTools += [JETM6SkimmingTool]

#====================================================================
# CREATE PRIVATE SEQUENCE
#====================================================================
jetm6Seq = CfgMgr.AthSequencer("JETM6Sequence")
DerivationFrameworkJob += jetm6Seq

#====================================================================
# THINNING TOOLS
#====================================================================
thinningTools = []

JETM6BaselineTrack = "(InDetTrackParticles.JETM6DFLoose) && (InDetTrackParticles.pt > 0.5*GeV) && (abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 5.0*mm) && (InDetTrackParticles.d0 < 5.0*mm)"

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
JETM6TrackParticleThinningTool = DerivationFramework__TrackParticleThinning(name            = "JETM6TrackParticleThinningTool",
                                                                            StreamName      = streamName,
                                                                            SelectionString = JETM6BaselineTrack,
                                                                            InDetTrackParticlesKey = "InDetTrackParticles")

ToolSvc += JETM6TrackParticleThinningTool
thinningTools.append(JETM6TrackParticleThinningTool)

#########################################
# Tracks associated with other jets
#########################################

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
JETM6Akt4PFlowJetTPThinningTool = DerivationFramework__JetTrackParticleThinning( name          = "JETM6Akt4PFlowJetTPThinningTool",
                                                                                 StreamName              = streamName,
                                                                                 JetKey                  = "AntiKt4EMPFlowJets",
                                                                                 InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM6Akt4PFlowJetTPThinningTool
thinningTools.append(JETM6Akt4PFlowJetTPThinningTool)

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
JETM6Akt10JetTPThinningTool = DerivationFramework__JetTrackParticleThinning( name          = "JETM6Akt10JetTPThinningTool",
                                                                             StreamName              = streamName,
                                                                             JetKey                  = "AntiKt10LCTopoJets",
                                                                             InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM6Akt10JetTPThinningTool
thinningTools.append(JETM6Akt10JetTPThinningTool)

JETM6Akt10JetCSSKUFOThinningTool = DerivationFramework__JetTrackParticleThinning( name          = "JETM6Akt10JetCSSKUFOThinningTool",
                                                                                  StreamName              = streamName,
                                                                                  JetKey                  = "AntiKt10UFOCSSKJets",
                                                                                  InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM6Akt10JetCSSKUFOThinningTool
thinningTools.append(JETM6Akt10JetCSSKUFOThinningTool)


# TrackParticles associated with Muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
JETM6MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name     = "JETM6MuonTPThinningTool",
                                                                         StreamName              = streamName,
                                                                         MuonKey                 = "Muons",
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM6MuonTPThinningTool
thinningTools.append(JETM6MuonTPThinningTool)

# TrackParticles associated with electrons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
JETM6ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name                    = "JETM6ElectronTPThinningTool",
                                                                               StreamName              = streamName,
                                                                               SGKey                   = "Electrons",
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM6ElectronTPThinningTool
thinningTools.append(JETM6ElectronTPThinningTool)

# TrackParticles associated with photons
JETM6PhotonTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name                    = "JETM6PhotonTPThinningTool",
                                                                             StreamName              = streamName,
                                                                             SGKey                   = "Photons",
                                                                             InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM6PhotonTPThinningTool
thinningTools.append(JETM6PhotonTPThinningTool)

# TrackParticles associated with taus
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TauTrackParticleThinning
JETM6TauTPThinningTool = DerivationFramework__TauTrackParticleThinning( name                   = "JETM6TauTPThinningTool",
                                                                        StreamName             = streamName,
                                                                        TauKey                 = "TauJets",
                                                                        InDetTrackParticlesKey = "InDetTrackParticles",
                                                                        DoTauTracksThinning    = True,
                                                                        TauTracksKey           = "TauTracks")

ToolSvc += JETM6TauTPThinningTool
thinningTools.append(JETM6TauTPThinningTool)

#====================================================================
# AUGMENTATION TOOLS
#====================================================================

augmentationTools = []

#====================================================================
# Tracking quality criteria decoration
#====================================================================

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__InDetTrackSelectionToolWrapper
JETM6TrackSelectionTool = DerivationFramework__InDetTrackSelectionToolWrapper(name = "JETM6TrackSelectionTool",
                                                                              ContainerName = "InDetTrackParticles",
                                                                              DecorationName = "JETM6DFLoose" )

JETM6TrackSelectionTool.TrackSelectionTool.CutLevel = "Loose"
ToolSvc += JETM6TrackSelectionTool
augmentationTools.append(JETM6TrackSelectionTool)

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm6Seq += CfgMgr.DerivationFramework__DerivationKernel( name = "JETM6Kernel",
                                                          AugmentationTools = augmentationTools,
                                                          SkimmingTools = JETM6SkimmingTools,
                                                          ThinningTools = thinningTools)

#====================================================================
# TRUTH3
#====================================================================

if DerivationFrameworkIsMonteCarlo:
  from DerivationFrameworkMCTruth.MCTruthCommon import addTopQuarkAndDownstreamParticles,addHFAndDownstreamParticles,addTruthCollectionNavigationDecorations
  addTopQuarkAndDownstreamParticles()
  addHFAndDownstreamParticles(addB=True, addC=False, generations=0)
  addTruthCollectionNavigationDecorations(TruthCollections=["TruthTopQuarkWithDecayParticles","TruthBosonsWithDecayParticles"],prefix='Top')

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM6SlimmingHelper = SlimmingHelper("JETM6SlimmingHelper")

if DerivationFrameworkIsMonteCarlo:
  JETM6SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticles'] = 'xAOD::TruthParticleContainer'
  JETM6SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
  JETM6SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVertices'] = 'xAOD::TruthVertexContainer'
  JETM6SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVerticesAux'] = 'xAOD::TruthVertexAuxContainer'
  JETM6SlimmingHelper.AppendToDictionary['TruthBottom'] = 'xAOD::TruthParticleContainer'
  JETM6SlimmingHelper.AppendToDictionary['TruthBottomAux'] = 'xAOD::TruthParticleAuxContainer'
  JETM6SlimmingHelper.AppendToDictionary['TruthParticles'] = 'xAOD::TruthParticleContainer'
  JETM6SlimmingHelper.AppendToDictionary['TruthParticlesAux'] = 'xAOD::TruthParticleAuxContainer'

JETM6SlimmingHelper.AppendToDictionary['UFOCSSK'] = 'xAOD::FlowElementContainer'
JETM6SlimmingHelper.AppendToDictionary['UFOCSSKAux'] = 'xAOD::FlowElementAuxContainer'
JETM6SlimmingHelper.AppendToDictionary["GlobalChargedParticleFlowObjects"]='xAOD::FlowElementContainer'
JETM6SlimmingHelper.AppendToDictionary["GlobalChargedParticleFlowObjectsAux"]='xAOD::FlowElementAuxContainer'
JETM6SlimmingHelper.AppendToDictionary["GlobalNeutralParticleFlowObjects"]='xAOD::FlowElementContainer'
JETM6SlimmingHelper.AppendToDictionary["GlobalNeutralParticleFlowObjectsAux"]='xAOD::FlowElementAuxContainer'
JETM6SlimmingHelper.AppendToDictionary["CSSKGChargedParticleFlowObjects"]='xAOD::FlowElementContainer'
JETM6SlimmingHelper.AppendToDictionary["CSSKGChargedParticleFlowObjectsAux"]='xAOD::ShallowAuxContainer'
JETM6SlimmingHelper.AppendToDictionary["CSSKGNeutralParticleFlowObjects"]='xAOD::FlowElementContainer'
JETM6SlimmingHelper.AppendToDictionary["CSSKGNeutralParticleFlowObjectsAux"]='xAOD::ShallowAuxContainer'

JETM6SlimmingHelper.SmartCollections = ["EventInfo","InDetTrackParticles","PrimaryVertices",
                                        "Electrons","Photons","Muons","TauJets",
                                        "MET_Baseline_AntiKt4EMPFlow",
                                        "AntiKt4EMPFlowJets", "AntiKt4TruthJets",
                                        "AntiKt10LCTopoJets","AntiKt10TruthJets", "AntiKt10UFOCSSKJets",
                                        "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                        "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                        "BTagging_AntiKt4EMPFlow"]

JETM6SlimmingHelper.AllVariables = ["Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape"]

JETM6SlimmingHelper.ExtraVariables  = ['CaloCalTopoClusters.calE.calEta.calM.calPhi.CENTER_MAG']
JETM6SlimmingHelper.ExtraVariables.append('GlobalChargedParticleFlowObjects.chargedObjectLinks')
JETM6SlimmingHelper.ExtraVariables.append('GlobalNeutralParticleFlowObjects.chargedObjectLinks')
JETM6SlimmingHelper.ExtraVariables.append('CSSKGChargedParticleFlowObjects.pt.eta.phi.m.matchedToPV.originalObjectLink')
JETM6SlimmingHelper.ExtraVariables.append('CSSKGNeutralParticleFlowObjects.pt.eta.phi.m.originalObjectLink')

if DerivationFrameworkIsMonteCarlo:
    JETM6SlimmingHelper.AllVariables += ["TruthMuons", "TruthElectrons", "TruthPhotons", "TruthBottom", "TruthTopQuarkWithDecayParticles", "TruthBosonsWithDecayParticles", "TruthHFWithDecayParticles",
                                         "TruthEvents", "TruthParticles"]

#====================================================================
# ORIGIN CORRECTED CLUSTERS
#====================================================================

addOriginCorrectedClusters(JETM6SlimmingHelper,writeLC=True,writeEM=True)

#====================================================================
# TRIGGER CONTENT
#====================================================================
 
JETM6SlimmingHelper.IncludeJetTriggerContent = True
JETM6SlimmingHelper.IncludeMuonTriggerContent = True
JETM6SlimmingHelper.IncludeEGammaTriggerContent = True

#==================================================================== 
# Add trigger matching
#==================================================================== 

photonTriggers_matching = ['HLT_g60_loose', 'HLT_g140_loose', 'HLT_g160_loose']

from AthenaConfiguration.AllConfigFlags import ConfigFlags
if ConfigFlags.Trigger.EDMVersion == 3:
  from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import TrigNavSlimmingMTDerivationCfg
  from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
  CAtoGlobalWrapper(TrigNavSlimmingMTDerivationCfg, ConfigFlags, chainsFilter=electronTriggers+muonTriggers+photonTriggers_matching+jetTriggers)
else:
  trigmatching_helper = TriggerMatchingHelper(name='JETM6TriggerMatchingTool',
                                              trigger_list = electronTriggers+muonTriggers+photonTriggers_matching+jetTriggers, add_to_df_job=True)
  trigmatching_helper.add_to_slimming(JETM6SlimmingHelper)

JETM6SlimmingHelper.AppendContentToStream(JETM6Stream)
