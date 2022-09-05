#====================================================================
# JETM13.py
# reductionConf flag JETM13 in Reco_tf.py
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkIsMonteCarlo, DerivationFrameworkJob, buildFileName
from DerivationFrameworkJetEtMiss.JetCommon import addDAODJets,OutputJets, addJetOutputs
from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSK
from DerivationFrameworkPhys import PhysCommon

if DerivationFrameworkIsMonteCarlo:
  from DerivationFrameworkMCTruth import MCTruthCommon
  MCTruthCommon.addBosonsAndDownstreamParticles(generations=4,rejectHadronChildren=True)
  MCTruthCommon.addTopQuarkAndDownstreamParticles(generations=4,rejectHadronChildren=True)

#=======================================
# CREATE PRIVATE SEQUENC
#=======================================
jetm13Seq = CfgMgr.AthSequencer("JETM13Sequence")
DerivationFrameworkJob += jetm13Seq

#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM13Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM13Stream )
JETM13Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM13Stream.AcceptAlgs(["JETM13MainKernel"])

#====================================================================
# THINNING TOOLS
#====================================================================
# Retain only stable truth particles, remove G4
# We want to keep all truth jet constituents
# Also keep the first 10 particles mainly for the HS truth vertex
jetm13thin = []
if DerivationFrameworkIsMonteCarlo:

  from DerivationFrameworkCore.ThinningHelper import ThinningHelper
  JETM13ThinningHelper = ThinningHelper( "JETM13ThinningHelper" )
  JETM13ThinningHelper.AppendToStream( JETM13Stream )
  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__MenuTruthThinning
  TruthThinningTool = DerivationFramework__MenuTruthThinning(name               = "JETM13TruthThinning",
                                                             StreamName         = streamName,
                                                             WriteAllStable     = True,
                                                             # Disable the flags that have been annoyingly
                                                             # defaulted to True
                                                             WritePartons       = False,
                                                             WriteHadrons       = False,
                                                             WriteBHadrons      = True,
                                                             WriteCHadrons      = False,
                                                             WriteGeant         = False,
                                                             WriteFirstN        = 10)
  ToolSvc += TruthThinningTool
  jetm13thin.append(TruthThinningTool)

#======================================= 
# CREATE THE DERIVATION KERNEL ALGORITHM
#======================================= 
jetm13Seq += CfgMgr.DerivationFramework__DerivationKernel( name = "JETM13MainKernel",
                                                          SkimmingTools = [],
                                                          ThinningTools = jetm13thin)


#=======================================
# UFO with CHS and small-R UFO jets
#=======================================
from JetRecConfig.JetRecConfig import getInputAlgs,reOrderAlgs
from JetRecConfig.StandardJetConstits import stdConstitDic as cst
from JetRecConfig.JetInputConfig import buildEventShapeAlg
from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
from AthenaConfiguration.AllConfigFlags import ConfigFlags

constit_algs = getInputAlgs(cst.UFO, configFlags=ConfigFlags)
constit_algs = reOrderAlgs( [a for a in constit_algs if a is not None])

for a in constit_algs:
  if not hasattr(DerivationFrameworkJob,a.getName()):
    DerivationFrameworkJob += conf2toConfigurable(a)

# R = 0.4 UFO CSSK jets
jetList = [AntiKt4UFOCSSK]
addDAODJets(jetList,DerivationFrameworkJob)

# Eventshape for UFO CSSK jets
eventshapealg = buildEventShapeAlg(cst.UFOCSSK,'')
if not hasattr(DerivationFrameworkJob, eventshapealg.getName()):
  DerivationFrameworkJob += conf2toConfigurable(eventshapealg)

OutputJets["JETM13"] = ["AntiKt4UFOCSSKJets"]

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM13SlimmingHelper = SlimmingHelper("JETM13SlimmingHelper")

JETM13SlimmingHelper.AppendToDictionary['GlobalChargedParticleFlowObjects'] ='xAOD::FlowElementContainer'
JETM13SlimmingHelper.AppendToDictionary['GlobalChargedParticleFlowObjectsAux'] ='xAOD::FlowElementAuxContainer'
JETM13SlimmingHelper.AppendToDictionary['GlobalNeutralParticleFlowObjects'] = 'xAOD::FlowElementContainer'
JETM13SlimmingHelper.AppendToDictionary['GlobalNeutralParticleFlowObjectsAux'] = 'xAOD::FlowElementAuxContainer'
JETM13SlimmingHelper.AppendToDictionary['CHSGChargedParticleFlowObjects'] = 'xAOD::FlowElementContainer'
JETM13SlimmingHelper.AppendToDictionary['CHSGChargedParticleFlowObjectsAux'] = 'xAOD::ShallowAuxContainer'
JETM13SlimmingHelper.AppendToDictionary['CHSGNeutralParticleFlowObjects'] = 'xAOD::FlowElementContainer'
JETM13SlimmingHelper.AppendToDictionary['CHSGNeutralParticleFlowObjectsAux'] = 'xAOD::ShallowAuxContainer'
JETM13SlimmingHelper.AppendToDictionary['UFOCSSK'] = 'xAOD::FlowElementContainer'
JETM13SlimmingHelper.AppendToDictionary['UFOCSSKAux'] = 'xAOD::FlowElementAuxContainer'
JETM13SlimmingHelper.AppendToDictionary['UFO'] = 'xAOD::FlowElementContainer'
JETM13SlimmingHelper.AppendToDictionary['UFOAux'] = 'xAOD::FlowElementAuxContainer'
JETM13SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShape'] = 'xAOD::EventShape'
JETM13SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShapeAux'] = 'xAOD::EventShapeAuxInfo'
if DerivationFrameworkIsMonteCarlo:
  JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticles'] = 'xAOD::TruthParticleContainer'
  JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
  JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVertices'] = 'xAOD::TruthVertexContainer'
  JETM13SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVerticesAux'] = 'xAOD::TruthVertexAuxContainer'
  JETM13SlimmingHelper.AppendToDictionary['TruthParticles'] = 'xAOD::TruthParticleContainer'
  JETM13SlimmingHelper.AppendToDictionary['TruthParticlesAux'] = 'xAOD::TruthParticleAuxContainer'

JETM13SlimmingHelper.SmartCollections = ["EventInfo",
                                         "Electrons", "Photons", "Muons", "TauJets",
                                         "InDetTrackParticles", "PrimaryVertices",
                                         "MET_Baseline_AntiKt4EMPFlow",
                                         "AntiKt4EMTopoJets","AntiKt4EMPFlowJets",
                                         "AntiKt4TruthJets","AntiKt10TruthJets",
                                         "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"
                                        ]

JETM13SlimmingHelper.AllVariables = ["CaloCalTopoClusters", "CaloCalFwdTopoTowers",
                                     "GlobalChargedParticleFlowObjects", "GlobalNeutralParticleFlowObjects",
                                     "CHSGChargedParticleFlowObjects","CHSGNeutralParticleFlowObjects",
                                     "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape",
                                     "TruthParticles",
                                     "TruthVertices",
                                     "TruthEvents",
                                    ]

if DerivationFrameworkIsMonteCarlo:
    JETM13SlimmingHelper.AllVariables += ["TruthMuons", "TruthElectrons", "TruthPhotons", "TruthTopQuarkWithDecayParticles", "TruthBosonsWithDecayParticles"]
    JETM13SlimmingHelper.AllVariables += ["TruthBosonsWithDecayVertices", "TruthTopQuarkWithDecayVertices"]

JETM13SlimmingHelper.ExtraVariables = ["UFOCSSK.pt.eta.phi.m.signalType",
                                       "UFO.pt.eta.phi.m.signalType",
                                       "InDetTrackParticles.particleHypothesis.vx.vy.vz",
                                       "GSFTrackParticles.particleHypothesis.vx.vy.vz",
                                       "PrimaryVertices.x.y.z",
                                       "TauJets.clusterLinks",
                                       "Muons.energyLossType.EnergyLoss.ParamEnergyLoss.MeasEnergyLoss.EnergyLossSigma.MeasEnergyLossSigma.ParamEnergyLossSigmaPlus.ParamEnergyLossSigmaMinus.clusterLinks.FSR_CandidateEnergy",
                                       "MuonSegments.x.y.z.px.py.pz"]

addJetOutputs(JETM13SlimmingHelper,
              ["JETM13"],
              JETM13SlimmingHelper.SmartCollections
)

JETM13SlimmingHelper.AppendContentToStream(JETM13Stream)
