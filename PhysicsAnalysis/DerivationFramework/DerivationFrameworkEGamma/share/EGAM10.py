#********************************************************************
# EGAM10.py
# Inclusive photon reduction - for e/gamma photon studies
# reductionConf flag EGAM10 in Reco_tf.py
# Migrated from r21 STDM2 format
#********************************************************************

from DerivationFrameworkCore.DerivationFrameworkMaster import buildFileName
from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkIsMonteCarlo, DerivationFrameworkJob
from DerivationFrameworkPhys import PhysCommon
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkEGamma.EGAM10ExtraContent import *
from DerivationFrameworkEGamma.TriggerContent import *

from DerivationFrameworkEGamma import EGammaIso
pflowIsoVar,densityList,densityDict = EGammaIso.makeEGammaCommonIso()

MenuType = 'Run3'
if ConfigFlags.Trigger.EDMVersion == 2: 
    MenuType = 'Run2'

#====================================================================
# read common DFEGamma settings from egammaDFFlags
#====================================================================
from DerivationFrameworkEGamma.egammaDFFlags import jobproperties
jobproperties.egammaDFFlags.print_JobProperties("full")


#====================================================================
# Set up sequence for this format and add to the top sequence
#====================================================================
EGAM10Sequence = CfgMgr.AthSequencer("EGAM10Sequence")
DerivationFrameworkJob += EGAM10Sequence


#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_EGAM10Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_EGAM10Stream )
EGAM10Stream = MSMgr.NewPoolRootStream( streamName, fileName )
# Only events that pass the filters listed below are written out.
# Name must match that of the kernel above
# AcceptAlgs  = logical OR of filters
# RequireAlgs = logical AND of filters
EGAM10Stream.AcceptAlgs(["EGAM10Kernel"])


### Thinning and augmentation tools lists
augmentationTools = []
thinningTools=[]


#====================================================================
# SET UP AUGMENTATIONS
#====================================================================


#====================================================================
# Max Cell sum decoration tool
#====================================================================
from DerivationFrameworkCalo.DerivationFrameworkCaloConf import DerivationFramework__MaxCellDecorator
EGAM10_MaxCellDecoratorTool = DerivationFramework__MaxCellDecorator( name = "EGAM10_MaxCellDecoratorTool",
                                                                     SGKey_electrons = "Electrons",
                                                                     SGKey_photons   = "Photons")
ToolSvc += EGAM10_MaxCellDecoratorTool
augmentationTools += [EGAM10_MaxCellDecoratorTool]


#====================================================================
# PhotonVertexSelectionWrapper decoration tool - needs PhotonPointing tool
#====================================================================
from PhotonVertexSelection.PhotonVertexSelectionConf import CP__PhotonPointingTool
from RecExConfig.RecFlags  import rec

EGAM10_PhotonPointingTool = CP__PhotonPointingTool(name = "EGAM10_PhotonPointingTool",
                                                   isSimulation = DerivationFrameworkIsMonteCarlo)
ToolSvc += EGAM10_PhotonPointingTool

from DerivationFrameworkEGamma.DerivationFrameworkEGammaConf import DerivationFramework__PhotonVertexSelectionWrapper
EGAM10_PhotonVertexSelectionWrapper = DerivationFramework__PhotonVertexSelectionWrapper(name = "EGAM10_PhotonVertexSelectionWrapper",
                                                                                        PhotonPointingTool = EGAM10_PhotonPointingTool,
                                                                                        DecorationPrefix = "EGAM10",
                                                                                        PhotonContainer = "Photons",
                                                                                        VertexContainer = "PrimaryVertices")
ToolSvc += EGAM10_PhotonVertexSelectionWrapper
augmentationTools += [EGAM10_PhotonVertexSelectionWrapper]


#====================================================================
# SET UP THINNING
#====================================================================

electronRequirements = '(Electrons.pt > 15*GeV) && (abs(Electrons.eta) < 2.5) && (Electrons.DFCommonElectronsLHLoose)'
photonRequirements = '(DFCommonPhotons_et >= 15*GeV) && (abs(DFCommonPhotons_eta) < 2.5)' # && (Photons.DFCommonPhotonsLoose)'

# All Track within a cone DeltaR=0.6 around Electrons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
EGAM10_ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name = "EGAM10_ElectronTPThinningTool",
                                                                                StreamName              = streamName,
                                                                                SGKey                  = "Electrons",
                                                                                GSFTrackParticlesKey   = "GSFTrackParticles",
                                                                                InDetTrackParticlesKey = "InDetTrackParticles",
                                                                                SelectionString        = electronRequirements,
                                                                                BestMatchOnly          = True,
                                                                                ConeSize               = 0.6)
ToolSvc += EGAM10_ElectronTPThinningTool
thinningTools.append(EGAM10_ElectronTPThinningTool)

# Track associated to all Electrons for ambiguity resolver tool
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
EGAM10_ElectronTPThinningToolAR = DerivationFramework__EgammaTrackParticleThinning(name = "EGAM10_ElectronTPThinningToolAR",
                                                                                   StreamName              = streamName,
                                                                                   SGKey                  = "Electrons",
                                                                                   GSFTrackParticlesKey   = "GSFTrackParticles",
                                                                                   InDetTrackParticlesKey = "InDetTrackParticles",
                                                                                   BestMatchOnly          = True)
ToolSvc += EGAM10_ElectronTPThinningToolAR
thinningTools.append(EGAM10_ElectronTPThinningToolAR)

# Tracks associated with Photons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
EGAM10_PhotonTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name = "EGAM10_PhotonTPThinningTool",
                                                                               StreamName              = streamName,
                                                                               SGKey                  = "Photons",
                                                                               GSFTrackParticlesKey   = "GSFTrackParticles",
                                                                               InDetTrackParticlesKey = "InDetTrackParticles",
                                                                               SelectionString        = photonRequirements,
                                                                               BestMatchOnly          = False,
                                                                               ConeSize               = 0.6)
ToolSvc += EGAM10_PhotonTPThinningTool
thinningTools.append(EGAM10_PhotonTPThinningTool)

# Possibility to thin CaloCalTopoClusters (UE/PU iso corrections not recomputable then)
# see https://indico.cern.ch/event/532191/contributions/2167754/attachments/1273075/1887639/ArthurLesage_ASGMeeting_20160513.pdf, S6-7


#====================================================================
# Setup the skimming criteria
#====================================================================

#====================================================================
# SKIMMING TOOL - OFFLINE
#====================================================================
photonSelection = '(count(' + photonRequirements + ') >= 1)'

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
EGAM10_OfflineSkimmingTool = DerivationFramework__xAODStringSkimmingTool( name = "EGAM10_OfflineSkimmingTool",
                                                                          expression = photonSelection)
ToolSvc += EGAM10_OfflineSkimmingTool
print("EGAM10 offline skimming tool:", EGAM10_OfflineSkimmingTool)


#====================================================================
# SKIMMING TOOL - trigger-based selection
#====================================================================
allTriggers = singlePhotonTriggers[MenuType] + diPhotonTriggers[MenuType] + triPhotonTriggers[MenuType] + noalgTriggers[MenuType]
#remove duplicates
allTriggers = list(set(allTriggers))

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
EGAM10_TriggerSkimmingTool = DerivationFramework__TriggerSkimmingTool(   name = "EGAM10_TriggerSkimmingTool", TriggerListOR = allTriggers)
print('EGAM10 list of triggers used for skimming:')
for trig in allTriggers: print(trig)

# can't use trigger API (https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerAPI) because we also need prescaled triggers
# so we cannot use trig_g   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
# which gives only the unprescaled ones
# nor trig_g   = TriggerAPI.getActive(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
# which seems to return only single photon triggers but not multi-photon ones

ToolSvc += EGAM10_TriggerSkimmingTool
print("EGAM10 trigger skimming tool:", EGAM10_TriggerSkimmingTool)

#
# Make AND of trigger-based and offline-based selections
#
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
EGAM10_SkimmingTool = DerivationFramework__FilterCombinationAND(name="EGAM10_SkimmingTool", FilterList=[EGAM10_OfflineSkimmingTool,EGAM10_TriggerSkimmingTool] )
ToolSvc+=EGAM10_SkimmingTool


#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM
#=======================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
print("EGAM10 skimming tools: ", [EGAM10_SkimmingTool])
print("EGAM10 thinning tools: ", thinningTools)
print("EGAM10 augmentation tools: ", augmentationTools)
EGAM10Sequence += CfgMgr.DerivationFramework__DerivationKernel("EGAM10Kernel",
                                                               SkimmingTools = [EGAM10_SkimmingTool],
                                                               AugmentationTools = augmentationTools,
                                                               ThinningTools = thinningTools
                                                               )


#====================================================================
# JET/MET
#====================================================================
from DerivationFrameworkJetEtMiss.JetCommon import addDAODJets
from JetRecConfig.StandardSmallRJets import AntiKt4Truth,AntiKt4TruthDressedWZ,AntiKt4PV0Track
jetList = [AntiKt4Truth,AntiKt4TruthDressedWZ,AntiKt4PV0Track]
addDAODJets(jetList, EGAM10Sequence)


#====================================================================
# SET UP SLIMMING
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
EGAM10SlimmingHelper = SlimmingHelper("EGAM10SlimmingHelper")
EGAM10SlimmingHelper.SmartCollections = ["Electrons",
                                         "Photons",
                                         "AntiKt4EMPFlowJets",
                                         "InDetTrackParticles",
                                         "PrimaryVertices"
]
EGAM10SlimmingHelper.IncludeEGammaTriggerContent = True

# Extra variables
EGAM10SlimmingHelper.ExtraVariables = ExtraVariablesElectrons + ExtraVariablesPhotons + ExtraVariablesVtx + ExtraVariablesTrk + ExtraVariablesJets + ExtraVariablesEventShape

EGAM10SlimmingHelper.AppendToDictionary.update(densityDict)
EGAM10SlimmingHelper.ExtraVariables += densityList + [f'Photons{pflowIsoVar}']

EGAM10SlimmingHelper.AllVariables += ExtraContainers

# Add event info
if jobproperties.egammaDFFlags.doEGammaEventInfoSlimming:
    EGAM10SlimmingHelper.SmartCollections.append("EventInfo")
else:
    EGAM10SlimmingHelper.AllVariables += ["EventInfo"]

# add detailed photon shower shape variables
from DerivationFrameworkEGamma.PhotonsCPDetailedContent import *
EGAM10SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

# additional truth-level variables
if DerivationFrameworkIsMonteCarlo:
    EGAM10SlimmingHelper.ExtraVariables += ExtraVariablesElectronsTruth+ExtraVariablesPhotonsTruth
    EGAM10SlimmingHelper.AllVariables   += ExtraContainersTruth+ExtraContainersTruthPhotons
    EGAM10SlimmingHelper.AllVariables   += ["TruthIsoCentralEventShape", "TruthIsoForwardEventShape"]
    EGAM10SlimmingHelper.AppendToDictionary.update(ExtraDictionary)

# This line must come after we have finished configuring EGAM1SlimmingHelper
EGAM10SlimmingHelper.AppendContentToStream(EGAM10Stream)
