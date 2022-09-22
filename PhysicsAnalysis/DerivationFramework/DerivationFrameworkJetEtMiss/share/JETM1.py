
#====================================================================
# JETM1.py
# reductionConf flag JETM1 in Reco_tf.py
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkIsMonteCarlo, DerivationFrameworkJob, buildFileName

from DerivationFrameworkPhys import PhysCommon

from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMPFlow, AntiKt4EMPFlowNoPtCut, AntiKt4EMTopoLowPt, AntiKt4EMPFlowCSSKNoPtCut, AntiKt4UFOCSSKNoPtCut

from DerivationFrameworkJetEtMiss.JetCommon import addJetOutputs, addDAODJets, OutputJets

#====================================================================
# SKIMMING TOOL
#====================================================================
from DerivationFrameworkJetEtMiss import TriggerLists
triggers = TriggerLists.jetTrig()

# Trigger API doesn't currently return all triggers used in Run-3
# Adding all jets triggers via explicit list for the moment
triggers += ["HLT_j0_pf_ftf_L1RD0_FILLED",
             "HLT_j15_pf_ftf_L1RD0_FILLED",
             "HLT_j25_pf_ftf_L1RD0_FILLED",
             "HLT_j35_pf_ftf_L1RD0_FILLED",
             "HLT_j45_pf_ftf_preselj20_L1RD0_FILLED",
             "HLT_j45_pf_ftf_preselj20_L1J15",
             "HLT_j60_pf_ftf_preselj50_L1J20",
             "HLT_j85_pf_ftf_preselj50_L1J20",
             "HLT_j110_pf_ftf_preselj80_L1J30",
             "HLT_j175_pf_ftf_preselj140_L1J50",
             "HLT_j260_pf_ftf_preselj200_L1J75",
             "HLT_j360_pf_ftf_preselj225_L1J100",
             "HLT_j420_pf_ftf_preselj225_L1J100",
             "HLT_j0_L1RD0_FILLED",
             "HLT_j15_L1RD0_FILLED",
             "HLT_j25_L1RD0_FILLED",
             "HLT_j35_L1RD0_FILLED",
             "HLT_j45_preselj20_L1RD0_FILLED",
             "HLT_j45_preselj20_L1J15",
             "HLT_j60_preselj50_L1J20",
             "HLT_j85_preselj50_L1J20",
             "HLT_j110_preselj80_L1J30",
             "HLT_j175_preselj140_L1J50",
             "HLT_j260_preselj200_L1J75",
             "HLT_j360_preselj225_L1J100",
             "HLT_j420_L1J100",
             "HLT_j420_pf_ftf_L1J100",
             "HLT_j15f_L1RD0_FILLED",
             "HLT_j25f_L1RD0_FILLED",
             "HLT_j35f_L1RD0_FILLED",
             "HLT_j45f_L1J15p31ETA49",
             "HLT_j60f_L1J20p31ETA49",
             "HLT_j85f_L1J20p31ETA49",
             "HLT_j110f_L1J30p31ETA49",
             "HLT_j175f_L1J50p31ETA49",
             "HLT_j220f_L1J75p31ETA49",
             "HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1J100",
             "HLT_3j200_pf_ftf_presel3j150_L1J100",
             "HLT_4j115_pf_ftf_presel4j85_L13J50",
             "HLT_5j70c_pf_ftf_presel5c50_L14J15",
             "HLT_5j85_pf_ftf_presel5j50_L14J15",
             "HLT_6j55c_pf_ftf_presel6j40_L14J15",
             "HLT_6j70_pf_ftf_presel6j40_L14J15",
             "HLT_7j45_pf_ftf_presel7j30_L14J15",
             "HLT_10j40_pf_ftf_presel7j30_L14J15",
             "HLT_3j200_L1J100",
             "HLT_4j120_L13J50",
             "HLT_5j70c_L14J15",
             "HLT_5j85_L14J15",
             "HLT_6j55c_L14J15",
             "HLT_6j70_L14J15",
             "HLT_7j45_L14J15",
             "HLT_10j40_L14J15",
             "HLT_6j35c_020jvt_pf_ftf_presel6c25_L14J15",
             "HLT_3j200_pf_ftf_L1J100",
             "HLT_6j35c_pf_ftf_presel6c25_L14J15",
             "HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1J20",
             "HLT_j85_a10t_lcw_jes_L1J20",
             "HLT_j110_a10sd_cssk_pf_jes_ftf_preselj80_L1J30",
             "HLT_j110_a10t_lcw_jes_L1J30",
             "HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1J50",
             "HLT_j175_a10t_lcw_jes_L1J50",
             "HLT_j260_a10sd_cssk_pf_jes_ftf_preselj200_L1J75",
             "HLT_j260_a10t_lcw_jes_L1J75",
             "HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
             "HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
             "HLT_j360_a10t_lcw_jes_L1J100",
             "HLT_j360_a10t_lcw_jes_L1SC111-CJ15",
             "HLT_j420_35smcINF_a10t_lcw_jes_L1J100",
             "HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CJ15",
             "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
             "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
             "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
             "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
             "HLT_j460_a10t_lcw_jes_L1J100",
             "HLT_j460_a10t_lcw_jes_L1SC111-CJ15",
             "HLT_j460_a10r_L1J100",
             "HLT_j460_a10r_L1SC111-CJ15",
             "HLT_j460_a10_lcw_subjes_L1J100",
             "HLT_j460_a10_lcw_subjes_L1SC111-CJ15",
             "HLT_j420_a10t_lcw_jes_L1J100",
             "HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
             "HLT_2j330_35smcINF_a10t_lcw_jes_L1J100",
             "HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1J100",
             "HLT_2j330_35smcINF_a10t_lcw_jes_L1SC111-CJ15",
             "HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
             "HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC111-CJ15",
             "HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC111-CJ15",
             "HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
             "HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
             "HLT_2j330_a10t_lcw_jes_L1J100",
             "HLT_2j330_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
             "HLT_j0_HT1000_L1J100",
             "HLT_j0_HT1000_L1HT190-J15s5pETA21",
             "HLT_j0_HT1000_pf_ftf_preselj180_L1J100",
             "HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-J15s5pETA21",
             "HLT_j0_HT1000_pf_ftf_preselcHT450_L1HT190-J15s5pETA21"
]

JETM1SkimmingTools = []

if not DerivationFrameworkIsMonteCarlo:

    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
    JETM1TrigSkimmingTool = DerivationFramework__TriggerSkimmingTool( name                   = "JETM1TrigSkimmingTool1",
                                                                      TriggerListOR          = triggers )
    ToolSvc += JETM1TrigSkimmingTool

    expression = 'HLT_xe120_pufit_L1XE50'
    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
    JETM1OfflineSkimmingTool = DerivationFramework__xAODStringSkimmingTool(name       = "JETM1OfflineSkimmingTool1",
                                                                           expression = expression)
    ToolSvc += JETM1OfflineSkimmingTool

    # OR of the above two selections
    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationOR
    JETM1ORTool = DerivationFramework__FilterCombinationOR(name="JETM1ORTool", FilterList=[JETM1TrigSkimmingTool,JETM1OfflineSkimmingTool] )
    ToolSvc+=JETM1ORTool

    JETM1SkimmingTools += [JETM1ORTool]

#=======================================
# CREATE PRIVATE SEQUENCE
#=======================================

jetm1Seq = CfgMgr.AthSequencer("JETM1Sequence")
DerivationFrameworkJob += jetm1Seq

#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM1Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM1Stream )
JETM1Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM1Stream.AcceptAlgs(["JETM1Kernel"])

#====================================================================
# THINNING TOOLS
#====================================================================
thinningTools = []

# TrackParticles associated with Muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
JETM1MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name     = "JETM1MuonTPThinningTool",
                                                                    StreamName              = JETM1Stream.Name,
                                                                    MuonKey                 = "Muons",
                                                                    InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM1MuonTPThinningTool
thinningTools.append(JETM1MuonTPThinningTool)

# TrackParticles associated with electrons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
JETM1ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name                    = "JETM1ElectronTPThinningTool",
                                                                               StreamName              = JETM1Stream.Name,
                                                                               SGKey                   = "Electrons",
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM1ElectronTPThinningTool
thinningTools.append(JETM1ElectronTPThinningTool)

thinning_expression = "InDetTrackParticles.JETM1DFLoose && ( abs(InDetTrackParticles.d0) < 5.0*mm ) && ( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"

# TrackParticles associated with small-R jets
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
JETM1Akt4JetTPThinningTool = DerivationFramework__JetTrackParticleThinning( name          = "JETM1Akt4JetTPThinningTool",
                                                                            StreamName              = streamName,
                                                                            JetKey                  = "AntiKt4EMTopoJets",
                                                                            SelectionString         = "AntiKt4EMTopoJets.pt > 18*GeV",
                                                                            TrackSelectionString    = thinning_expression, 
                                                                            InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM1Akt4JetTPThinningTool
thinningTools.append(JETM1Akt4JetTPThinningTool)

JETM1Akt4PFlowJetTPThinningTool = DerivationFramework__JetTrackParticleThinning( name          = "JETM1Akt4PFlowJetTPThinningTool",
                                                                                 StreamName              = streamName,
                                                                                 JetKey                  = "AntiKt4EMPFlowJets",
                                                                                 SelectionString         = "AntiKt4EMPFlowJets.pt > 18*GeV",
                                                                                 TrackSelectionString    = thinning_expression,
                                                                                 InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM1Akt4PFlowJetTPThinningTool
thinningTools.append(JETM1Akt4PFlowJetTPThinningTool)

#=======================================
# Augmentation tools
#=======================================

augmentationTools = []

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__InDetTrackSelectionToolWrapper
JETM1TrackSelectionTool = DerivationFramework__InDetTrackSelectionToolWrapper(name = "JETM1TrackSelectionTool",
                                                                              ContainerName = "InDetTrackParticles",
                                                                              DecorationName = "JETM1DFLoose" )

JETM1TrackSelectionTool.TrackSelectionTool.CutLevel = "Loose"
ToolSvc += JETM1TrackSelectionTool
augmentationTools.append(JETM1TrackSelectionTool)

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm1Seq += CfgMgr.DerivationFramework__DerivationKernel("JETM1Kernel" ,
                                                         AugmentationTools = augmentationTools,
                                                         SkimmingTools = JETM1SkimmingTools,
                                                         ThinningTools = thinningTools)

#=======================================
# SCHEDULE ADDITIONAL JET DECORATIONS
#=======================================

from JetRecConfig.StandardJetMods import stdJetModifiers
from JetRecConfig.JetRecConfig import getModifier

bJVTTool = getModifier(AntiKt4EMPFlow, stdJetModifiers['bJVT'], stdJetModifiers['bJVT'].modspec)
jetm1Seq += CfgMgr.JetDecorationAlg('bJVTAlg', JetContainer='AntiKt4EMPFlowJets', Decorators=[bJVTTool])

#=======================================
# R = 0.4 track-jets (needed for Rtrk)
#=======================================
jetList = [AntiKt4PV0Track]

#=======================================
# SCHEDULE SMALL-R JETS WITH NO PT CUT
#=======================================
if DerivationFrameworkIsMonteCarlo:
    jetList += [AntiKt4EMPFlowNoPtCut, AntiKt4EMTopoLowPt]

#=======================================
# CSSK R = 0.4 EMPFlow and UFO jets
#=======================================
jetList += [AntiKt4EMPFlowCSSKNoPtCut, AntiKt4UFOCSSKNoPtCut]

addDAODJets(jetList,DerivationFrameworkJob)

#======================================= 
# UFO CSSK event shape
#======================================= 
from JetRecConfig.JetRecConfig import getConstitPJGAlg
from JetRecConfig.StandardJetConstits import stdConstitDic as cst
from JetRecConfig.JetInputConfig import buildEventShapeAlg
from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable

#Need to add non-standard jets explicitly
OutputJets["JETM1"] = ["AntiKt4PV0TrackJets","AntiKt4EMPFlowCSSKNoPtCutJets","AntiKt4UFOCSSKNoPtCutJets","AntiKt4EMPFlowNoPtCutJets","AntiKt4EMTopoLowPtJets"]

eventshapealg = buildEventShapeAlg(cst.UFOCSSK,'')
if not hasattr(DerivationFrameworkJob, eventshapealg.getName()):
  DerivationFrameworkJob += conf2toConfigurable(eventshapealg)

pjgufoccskneut = getConstitPJGAlg(cst.UFOCSSK, suffix='Neut')
if not hasattr(DerivationFrameworkJob, pjgufoccskneut.getName()):
  DerivationFrameworkJob += conf2toConfigurable(pjgufoccskneut)

eventshapeneutalg = buildEventShapeAlg(cst.UFOCSSK, '', suffix = 'Neut')
if not hasattr(DerivationFrameworkJob, eventshapeneutalg.getName()):
  DerivationFrameworkJob += conf2toConfigurable(eventshapeneutalg)

#=======================================
# More detailed truth information
#=======================================

if DerivationFrameworkIsMonteCarlo:
    from DerivationFrameworkMCTruth import MCTruthCommon
    MCTruthCommon.addBosonsAndDownstreamParticles(generations=4,rejectHadronChildren=True)
    MCTruthCommon.addTopQuarkAndDownstreamParticles(generations=4,rejectHadronChildren=True)

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM1SlimmingHelper = SlimmingHelper("JETM1SlimmingHelper")

JETM1SlimmingHelper.SmartCollections = ["Electrons", "Photons", "Muons", "PrimaryVertices",
                                        "InDetTrackParticles",
                                        "AntiKt4EMTopoJets","AntiKt4EMPFlowJets",
                                        "AntiKt10UFOCSSKJets",
                                        "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                        "BTagging_AntiKt4EMPFlow"]

JETM1SlimmingHelper.ExtraVariables  = ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                       "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                       "AntiKt4EMPFlowJets.GhostTower",
                                       "AntiKt4EMPFlowJets.passOnlyBJVT","AntiKt4EMPFlowJets.DFCommonJets_bJvt",
                                       "InDetTrackParticles.truthMatchProbability", 
                                       "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.zg.rg.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.EnergyPerSampling.GhostTrack",
                                       "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets.zg.rg",
                                       "AntiKt10UFOCSSKJets.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.EnergyPerSampling.GhostTrack",
                                       "TruthVertices.barcode.z"]

JETM1SlimmingHelper.AllVariables = [ "MuonSegments", "EventInfo",
                                     "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape",
                                     "CaloCalFwdTopoTowers"]

JETM1SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShape'] = 'xAOD::EventShape'
JETM1SlimmingHelper.AppendToDictionary['Kt4UFOCSSKEventShapeAux'] = 'xAOD::EventShapeAuxInfo'
JETM1SlimmingHelper.AppendToDictionary['Kt4UFOCSSKNeutEventShape'] = 'xAOD::EventShape'
JETM1SlimmingHelper.AppendToDictionary['Kt4UFOCSSKNeutEventShapeAux'] = 'xAOD::EventShapeAuxInfo'

if DerivationFrameworkIsMonteCarlo:
    JETM1SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticles'] = 'xAOD::TruthParticleContainer'
    JETM1SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayParticlesAux'] = 'xAOD::TruthParticleAuxContainer'
    JETM1SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVertices'] = 'xAOD::TruthVertexContainer'
    JETM1SlimmingHelper.AppendToDictionary['TruthTopQuarkWithDecayVerticesAux'] = 'xAOD::TruthVertexAuxContainer'
    JETM1SlimmingHelper.AppendToDictionary['TruthParticles'] = 'xAOD::TruthParticleContainer'
    JETM1SlimmingHelper.AppendToDictionary['TruthParticlesAux'] = 'xAOD::TruthParticleAuxContainer'

    JETM1SlimmingHelper.AllVariables += ["TruthMuons", "TruthElectrons", "TruthPhotons", "TruthTopQuarkWithDecayParticles", "TruthBosonsWithDecayParticles"]
    JETM1SlimmingHelper.AllVariables += ["TruthBosonsWithDecayVertices", "TruthTopQuarkWithDecayVertices"]
    JETM1SlimmingHelper.AllVariables += ["AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", "TruthParticles"]
    JETM1SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]

# Trigger content
JETM1SlimmingHelper.IncludeJetTriggerContent = True

# Add the jet containers to the stream
addJetOutputs(JETM1SlimmingHelper,
              ["JETM1"],
              JETM1SlimmingHelper.SmartCollections
)

JETM1SlimmingHelper.AppendContentToStream(JETM1Stream)
