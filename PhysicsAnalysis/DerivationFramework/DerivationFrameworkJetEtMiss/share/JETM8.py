#====================================================================
# JETM8.py 
# reductionConf flag JETM8 in Reco_tf.py   
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkIsMonteCarlo, DerivationFrameworkJob, buildFileName
from DerivationFrameworkJetEtMiss.JetCommon import addOriginCorrectedClusters
from DerivationFrameworkPhys import PhysCommon

#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM8Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM8Stream )
JETM8Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM8Stream.AcceptAlgs(["JETM8MainKernel"])
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()

#====================================================================
# SKIMMING TOOL 
#====================================================================

from DerivationFrameworkJetEtMiss import TriggerLists
jetTriggers = TriggerLists.jetTrig()

# For first data
jetSelection = '(count( AntiKt10LCTopoJets.pt > 150.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1)'
jetSelection += '||(count( AntiKt10UFOCSSKJets.pt > 150.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >=1)'

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
JETM8TrigSkimmingTool = DerivationFramework__TriggerSkimmingTool(   name           = "JETM8TrigSkimmingTool",
                                                                    TriggerListOR  = jetTriggers )
ToolSvc += JETM8TrigSkimmingTool

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
JETM8OfflineSkimmingTool = DerivationFramework__xAODStringSkimmingTool( name = "JETM8OfflineSkimmingTool",
                                                                        expression = jetSelection)
ToolSvc += JETM8OfflineSkimmingTool

#====================================================================
# THINNING TOOLS 
#====================================================================

thinningTools = []

#====================================================================
# Thin TruthParticles for truth jet constituents
#====================================================================

if DerivationFrameworkIsMonteCarlo:
  from DerivationFrameworkJetEtMiss.DerivationFrameworkJetEtMissConf import DerivationFramework__ViewContainerThinning
  JETM8TruthJetInputThin = DerivationFramework__ViewContainerThinning( name = "JETM8ViewContThinning",
                                                                       StreamName = streamName,
                                                                       TruthParticleKey = "TruthParticles",
                                                                       TruthParticleViewKey = "JetInputTruthParticles")

  ToolSvc += JETM8TruthJetInputThin
  thinningTools.append(JETM8TruthJetInputThin)

#====================================================================
# Thin jet inputs for jet constituents
#====================================================================

# Calo clusters
from DerivationFrameworkCalo.DerivationFrameworkCaloConf import DerivationFramework__JetCaloClusterThinning
JETM8AKt10CCThinningTool = DerivationFramework__JetCaloClusterThinning(name                  = "JETM8AKt10CCThinningTool",
                                                                       StreamName            = streamName,
                                                                       SGKey                 = "AntiKt10LCTopoJets",
                                                                       SelectionString       = "(AntiKt10LCTopoJets.pt > 150*GeV && abs(AntiKt10LCTopoJets.eta) < 2.8)",
                                                                       TopoClCollectionSGKey = "CaloCalTopoClusters",
                                                                       AdditionalClustersKey = ["LCOriginTopoClusters"],
                                                                     )
ToolSvc += JETM8AKt10CCThinningTool
thinningTools.append(JETM8AKt10CCThinningTool)

# Tracks and CaloClusters associated with UFOs
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__UFOTrackParticleThinning
JETM8EMCSSKUFOTPThinningTool = DerivationFramework__UFOTrackParticleThinning(name                   = "JETM8CSSKUFOTPThinningTool",
                                                                             StreamName             = streamName,
                                                                             JetKey                 = "AntiKt10UFOCSSKJets",
                                                                             UFOKey                 = "UFOCSSK",
                                                                             InDetTrackParticlesKey = "InDetTrackParticles",
                                                                             ThinTrackingContainer  = False,
                                                                             PFOCollectionSGKey     = "Global",
                                                                             AdditionalPFOKey       = ["CHSG","CSSKG"])

ToolSvc += JETM8EMCSSKUFOTPThinningTool
thinningTools.append(JETM8EMCSSKUFOTPThinningTool)

#=======================================
# CREATE PRIVATE SEQUENCE
#=======================================

jetm8Seq = CfgMgr.AthSequencer("JETM8Sequence")
DerivationFrameworkJob += jetm8Seq

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm8Seq += CfgMgr.DerivationFramework__DerivationKernel(	name = "JETM8TrigKernel", 
									SkimmingTools = [JETM8TrigSkimmingTool],
									ThinningTools = [])

jetm8Seq += CfgMgr.DerivationFramework__DerivationKernel( name = "JETM8MainKernel",
                                                          SkimmingTools = [JETM8OfflineSkimmingTool],
                                                          ThinningTools = thinningTools)

#====================================================================
# Add truth information
#====================================================================

if DerivationFrameworkIsMonteCarlo:
  from DerivationFrameworkMCTruth.MCTruthCommon import addTopQuarkAndDownstreamParticles
  from DerivationFrameworkMCTruth.MCTruthCommon import addHFAndDownstreamParticles
  from DerivationFrameworkMCTruth.MCTruthCommon import addTruthCollectionNavigationDecorations
  addTopQuarkAndDownstreamParticles()
  addTruthCollectionNavigationDecorations(TruthCollections=["TruthTopQuarkWithDecayParticles","TruthBosonsWithDecayParticles"],prefix='Top')


#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM8SlimmingHelper = SlimmingHelper("JETM8SlimmingHelper")

JETM8SlimmingHelper.AppendToDictionary['UFOCSSK'] = 'xAOD::FlowElementContainer'
JETM8SlimmingHelper.AppendToDictionary['UFOCSSKAux'] = 'xAOD::FlowElementAuxContainer'

if DerivationFrameworkIsMonteCarlo:
  JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticles'] = 'xAOD::TruthParticleContainer'
  JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
  JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVertices'] = 'xAOD::TruthVertexContainer'
  JETM8SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVerticesAux'] = 'xAOD::TruthVertexAuxContainer'
  JETM8SlimmingHelper.AppendToDictionary['TruthParticles'] = 'xAOD::TruthParticleContainer'
  JETM8SlimmingHelper.AppendToDictionary['TruthParticlesAux'] = 'xAOD::TruthParticleAuxContainer'

JETM8SlimmingHelper.SmartCollections = ["EventInfo",
                                        "Electrons", "Photons", "Muons",
                                        "InDetTrackParticles", "PrimaryVertices",
                                        "AntiKt4TruthJets",
                                        "AntiKt4TruthWZJets",
                                        "AntiKt4EMPFlowJets",
                                        "AntiKt10TruthJets",
                                        "AntiKt10TruthWZJets",
                                        "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10TruthSoftDropBeta100Zcut10Jets",
                                        "AntiKt10LCTopoJets",
                                        "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10UFOCSSKJets",
                                        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                        "BTagging_AntiKt4EMPFlow",
                                        "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                        ]
JETM8SlimmingHelper.AllVariables = ["CaloCalTopoClusters", "CaloCalFwdTopoTowers",
                                    "UFOCSSK",
                                    "TruthParticles"]

JETM8SlimmingHelper.ExtraVariables += ['AntiKt10LCTopoJets.SizeParameter.GhostTrack',
                                       'AntiKt10TruthJets.SizeParameter.Split12.Split23',
                                       'AntiKt10UFOCSSKJets.SizeParameter.GhostTrack',
                                       'AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets.SizeParameter.GhostTrack',
                                       'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.SizeParameter.GhostTrack',
                                       'AntiKt10TruthTrimmedPtFrac5SmallR20Jets.SizeParameter',
                                       'AntiKt10TruthSoftDropBeta100Zcut10Jets.SizeParameter',
                                       ]

# Add origin corrected clusters to keep LCTopo constituents
addOriginCorrectedClusters(JETM8SlimmingHelper, writeLC=True, writeEM=False)

JETM8SlimmingHelper.AppendContentToStream(JETM8Stream)
