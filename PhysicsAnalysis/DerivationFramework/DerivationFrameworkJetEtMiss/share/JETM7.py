#====================================================================
# JETM7.py
# reductionConf flag JETM7 in Reco_tf.py
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkInDet.InDetCommon import *

# Include TRUTH3 containers
if DerivationFrameworkHasTruth:
    from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
    addStandardTruthContents()

#====================================================================
# SKIMMING TOOL
#====================================================================

jetSelection = '(count(AntiKt4EMPFlowJets.pt > 10.*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.5) >= 1)'


from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
JETM7OfflineSkimmingTool = DerivationFramework__xAODStringSkimmingTool(name = "JETM7OfflineSkimmingTool",
                                                                    expression = jetSelection)
ToolSvc += JETM7OfflineSkimmingTool


#=======================================
# CREATE PRIVATE SEQUENCE
#=======================================

jetm7Seq = CfgMgr.AthSequencer("JETM7Sequence")
DerivationFrameworkJob += jetm7Seq

#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM7Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM7Stream )
JETM7Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM7Stream.AcceptAlgs(["JETM7Kernel"])

#=======================================
# ESTABLISH THE THINNING HELPER
#=======================================
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
JETM7ThinningHelper = ThinningHelper( "JETM7ThinningHelper" )
JETM7ThinningHelper.TriggerChains = 'HLT_.*' # Thin TrigNavigation
JETM7ThinningHelper.AppendToStream( JETM7Stream )


#====================================================================
# THINNING TOOLS
#====================================================================
thinningTools = []

# TrackParticles associated with Muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
JETM7MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name             = "JETM7MuonTPThinningTool",
                                                                   ThinningService         = JETM7ThinningHelper.ThinningSvc(),
                                                                   MuonKey                 = "Muons",
                                                                   InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM7MuonTPThinningTool
thinningTools.append(JETM7MuonTPThinningTool)

# TrackParticles associated with electrons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
JETM7ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name                  = "JETM7ElectronTPThinningTool",
                                                                              ThinningService         = JETM7ThinningHelper.ThinningSvc(),
                                                                              SGKey                   = "Electrons",
                                                                              InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM7ElectronTPThinningTool
thinningTools.append(JETM7ElectronTPThinningTool)


## TrackParticles associated with small-R jets
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
JETM7Akt4PFlowJetTPThinningTool = DerivationFramework__JetTrackParticleThinning( name          = "JETM7Akt4PFlowByVertexJetTPThinningTool",
                                                                                 ThinningService         = JETM7ThinningHelper.ThinningSvc(),
                                                                                 JetKey                  = "AntiKt4EMPFlowByVertexJets",
                                                                                 SelectionString         = "(AntiKt4EMPFlowByVertexJets.pt > 7*GeV && AntiKt4EMPFlowByVertexJets.Jvt > 0.4)",
                                                                                 InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += JETM7Akt4PFlowJetTPThinningTool
thinningTools.append(JETM7Akt4PFlowJetTPThinningTool)

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__GenericObjectThinning

# Store EMPFlowByVertexJets with JVT > 0.4. This will result in jets extending up to about 2.6 in |eta|
JETM7Akt4PFlowByVertexJetThinningTool = DerivationFramework__GenericObjectThinning( name             = "JETM7Akt4PFlowByVertexJetThinningTool",
                                                                             ThinningService  = JETM7ThinningHelper.ThinningSvc(),
                                                                             ContainerName    = "AntiKt4EMPFlowByVertexJets",
                                                                             SelectionString  = "(AntiKt4EMPFlowByVertexJets.Jvt > 0.4)",
                                                                             ApplyAnd         = False)

# Store regular EMPFlowJets for |eta| > 2.4 to retain forward jets with a PV0 interpretation
JETM7Akt4PFlowJetThinningTool = DerivationFramework__GenericObjectThinning( name             = "JETM7Akt4PFlowJetThinningTool",
                                                                             ThinningService  = JETM7ThinningHelper.ThinningSvc(),
                                                                             ContainerName    = "AntiKt4EMPFlowJets",
                                                                             SelectionString  = "(abs(AntiKt4EMPFlowJets.eta) > 2.4)",
                                                                             ApplyAnd         = False)

ToolSvc += JETM7Akt4PFlowByVertexJetThinningTool
ToolSvc += JETM7Akt4PFlowJetThinningTool

thinningTools.append(JETM7Akt4PFlowByVertexJetThinningTool)
thinningTools.append(JETM7Akt4PFlowJetThinningTool)

JETM7MuonThinningTool = DerivationFramework__GenericObjectThinning( name             = "JETM7MuonThinningTool",
                                                                     ThinningService  = JETM7ThinningHelper.ThinningSvc(),
                                                                     ContainerName    = "Muons",
                                                                     SelectionString  = "(Muons.pt > 5*GeV)",
                                                                     ApplyAnd         = False)

ToolSvc += JETM7MuonThinningTool
thinningTools.append(JETM7MuonThinningTool)

JETM7ElectronThinningTool = DerivationFramework__GenericObjectThinning( name             = "JETM7ElectronThinningTool",
                                                                         ThinningService  = JETM7ThinningHelper.ThinningSvc(),
                                                                         ContainerName    = "Electrons",
                                                                         SelectionString  = "(Electrons.pt > 5*GeV)",
                                                                         ApplyAnd         = False)

ToolSvc += JETM7ElectronThinningTool
thinningTools.append(JETM7ElectronThinningTool)

JETM7PhotonThinningTool = DerivationFramework__GenericObjectThinning( name             = "JETM7PhotonThinningTool",
                                                                       ThinningService  = JETM7ThinningHelper.ThinningSvc(),
                                                                       ContainerName    = "Photons",
                                                                       SelectionString  = "(Photons.pt > 5*GeV)",
                                                                       ApplyAnd         = False)

ToolSvc += JETM7PhotonThinningTool
thinningTools.append(JETM7PhotonThinningTool)


# TrackParticles failing quality cuts
# Dangerous to use: calculated with respect to nominal PV!! - Pt cut should be vertex-agnostic
thinning_expression = "InDetTrackParticles.pt > 1*GeV" 

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
JETM7TPThinningTool = DerivationFramework__TrackParticleThinning( name                    = "JETM7TPThinningTool",
                                                                   ThinningService         = JETM7ThinningHelper.ThinningSvc(),
                                                                   SelectionString         = thinning_expression,
                                                                   InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                   ApplyAnd                = True)
ToolSvc += JETM7TPThinningTool
thinningTools.append(JETM7TPThinningTool)

#=======================================
# Augmentation tools
#=======================================

augmentationTools = []


#================================================
# DECORATE TRUTH JETS WITH SOME TRUTH PROPERTIES
#================================================
if DerivationFrameworkHasTruth:
    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthJetDecorationTool
    DFCommonTruthJetsDecorationTool = DerivationFramework__TruthJetDecorationTool(name="DFCommonTruthJetsDecorationTool",
                                                              JetCollection = "AntiKt4TruthJets")
    ToolSvc += DFCommonTruthJetsDecorationTool
    augmentationTools.append(DFCommonTruthJetsDecorationTool)


# Add alternative rho definitions
#from DerivationFrameworkJetEtMiss.JetCommon import addCHSPFlowObjects
#addCHSPFlowObjects()
from DerivationFrameworkJetEtMiss.JetCommon import defineEDAlg
jetm7Seq += defineEDAlg(R=0.4, inputtype="EMPFlowNeut")



OutputJets["JETM7"] = []

#=======================================
# RESTORE AOD-REDUCED JET COLLECTIONS
#=======================================
reducedJetList = ["AntiKt2PV0TrackJets",
                  "AntiKt4PV0TrackJets",
                  "AntiKt4TruthJets"]
replaceAODReducedJets(reducedJetList,jetm7Seq,"JETM7")

#=======================================
# SCHEDULE SMALL-R JETS WITH LOW PT CUT
#=======================================

#addAntiKt4NoPtCutJets(jetm7Seq,"JETM7")
addAntiKt4ByVertexJets(jetm7Seq,"JETM7", 7000)

if DerivationFrameworkHasTruth:
    ## Add GhostTruthAssociation information ##
    addJetPtAssociation(jetalg="AntiKt4EMPFlowByVertex", truthjetalg="AntiKt4TruthJets", sequence=jetm7Seq, algname="JetPtAssociationAlgNoPtCut")

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm7Seq += CfgMgr.DerivationFramework__DerivationKernel("JETM7Kernel" ,
                                                         AugmentationTools = augmentationTools,
                                                         #SkimmingTools = [JETM7ORTool],
                                                         SkimmingTools = [JETM7OfflineSkimmingTool],
                                                         ThinningTools = thinningTools)

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM7SlimmingHelper = SlimmingHelper("JETM7SlimmingHelper")
JETM7SlimmingHelper.AppendToDictionary = {
    "Kt4EMPFlowNeutEventShape": "xAOD::EventShape",
    "Kt4EMPFlowNeutEventShapeAux": "xAOD::AuxInfoBase"
}

JETM7SlimmingHelper.SmartCollections = ["Electrons", "Photons", "Muons", "PrimaryVertices", "AntiKt4EMPFlowJets"]


JETM7SlimmingHelper.AllVariables = [ "MuonSegments", "TruthVertices", "TruthEvents", "AntiKt4TruthJets", "AntiKt4EMPFlowJets", 
                                      "Kt4EMTopoOriginEventShape","Kt4LCTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape", "AntiKt4EMPFlowByVertexJets"]

# Trigger content
JETM7SlimmingHelper.IncludeJetTriggerContent = True
JETM7SlimmingHelper.IncludeEtMissTriggerContent = True
JETM7SlimmingHelper.IncludeEGammaTriggerContent = True
JETM7SlimmingHelper.IncludeMuonTriggerContent = True
JETM7SlimmingHelper.IncludeTauTriggerContent = True
JETM7SlimmingHelper.IncludeBPhysTriggerContent = True

# Add the jet containers to the stream
addJetOutputs(JETM7SlimmingHelper,["JETM7"], [], 
              [
               "AntiKt4TruthWZJets", "AntiKt2PV0TrackJets", "AntiKt4PV0TrackJets"
               ]# veto list,
              )

if DerivationFrameworkHasTruth:
    JETM7SlimmingHelper.AllVariables += ["TruthMuons", "TruthElectrons", "TruthPhotons", "TruthEvents"]
    for truthc in ["TruthTop", "TruthBoson"]:
        JETM7SlimmingHelper.StaticContent.append("xAOD::TruthParticleContainer#"+truthc)
        JETM7SlimmingHelper.StaticContent.append("xAOD::TruthParticleAuxContainer#"+truthc+"Aux.")


JETM7SlimmingHelper.AppendContentToStream(JETM7Stream)
