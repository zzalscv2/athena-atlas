#====================================================================
# BPHY22.py

# B -> mu D*+
# It requires the reductionConf flag BPHY22 in Reco_tf.py
#====================================================================

# Set up common services and job object.
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

# data or simulation?
isSimulation = False
if globalflags.DataSource()=='geant4':
    isSimulation = True

print isSimulation


#====================================================================
# AUGMENTATION TOOLS
#====================================================================
## 1/ setup vertexing tools and services
include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY22_VertexTools = BPHYVertexTools("BPHY22")


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__AugOriginalCounts
BPHY22_AugOriginalCounts = DerivationFramework__AugOriginalCounts(
   name = "BPHY22_AugOriginalCounts",
   VertexContainer = "PrimaryVertices",
   TrackContainer = "InDetTrackParticles" )
ToolSvc += BPHY22_AugOriginalCounts

#===============================================================================================
#--------------------------------------------------------------------
# 1/ Select  mu pi
#--------------------------------------------------------------------
## a/ setup JpsiFinder tool
##    These are general tools independent of DerivationFramework that do the
##    actual vertex fitting and some pre-selection.
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY22MuPiFinder = Analysis__JpsiFinder(
    name                       = "BPHY22MuPiFinder",
    OutputLevel                = INFO,
    muAndMu                    = False,
    muAndTrack                 = True,  #need doTagAndProbe flag
    TrackAndTrack              = False,
    assumeDiMuons              = False,
    muonThresholdPt            = 2700,
    trackThresholdPt           = 250.0, # MeV
    invMassUpper               = 8200.0,
    invMassLower               = 200.0,
    Chi2Cut                    = 10.,
    oppChargesOnly             = False,
    allChargeCombinations      = True,
   # combOnly                   = False,
    atLeastOneComb             = False, # True by default
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY22_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY22_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY22_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY22_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY22_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False,
    doTagAndProbe              = True, #won't work with all/same charges combs
    forceTagAndProbe           = True) #force T&P to work with any charges combs

ToolSvc += BPHY22MuPiFinder
print      BPHY22MuPiFinder

#--------------------------------------------------------------------
## b/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool is the JpsiFinder mass window is wide enough.
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY22MuPiSelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY22MuPiSelectAndWrite",
    JpsiFinder             = BPHY22MuPiFinder,
    OutputVtxContainerName = "BPHY22MuPiCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED")

ToolSvc +=  BPHY22MuPiSelectAndWrite
print       BPHY22MuPiSelectAndWrite

#--------------------------------------------------------------------
## c/ augment and select mu pi candidates
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu
BPHY22_Select_MuPi = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY22_Select_MuPi",
    HypothesisName        = "MuPi",
    InputVtxContainerName = "BPHY22MuPiCandidates",
 #   TrkMasses             = [105.658, 139.571],
    VtxMassHypo           = 5279.64,
    MassMin               = 200.0,
    MassMax               = 8200.0,
    Chi2Max               = 200)
    #LxyMin                = -100, #default lowest of Double

ToolSvc += BPHY22_Select_MuPi
print      BPHY22_Select_MuPi
##
#===============================================================================================


#===============================================================================================
##
#--------------------------------------------------------------------
# 2/ Select K+K-, pi+K- and K+pi-      for D0, Ds, Dm
#--------------------------------------------------------------------
## a/ Setup the vertex fitter tools
BPHY22DiTrkFinder = Analysis__JpsiFinder(
    name                       = "BPHY22DiTrkFinder",
    OutputLevel                = INFO,
    muAndMu                    = False,
    muAndTrack                 = False,
    TrackAndTrack              = True,
    assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt           = 900,
    invMassUpper               = 2100.0,
    invMassLower               = 275,
    Chi2Cut                    = 20., #chi2
    oppChargesOnly             = True,
    atLeastOneComb             = False,
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY22_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY22_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY22_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY22_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY22_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False,
    track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
    track2Mass                 = 139.571)
  
ToolSvc += BPHY22DiTrkFinder
print      BPHY22DiTrkFinder

#--------------------------------------------------------------------
## b/ setup the vertex reconstruction "call" tool(s).
BPHY22DiTrkSelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY22DiTrkSelectAndWrite",
    JpsiFinder             = BPHY22DiTrkFinder,
    OutputVtxContainerName = "BPHY22DiTrkCandidates",
    PVContainerName        = "PrimaryVertices",
    #RefPVContainerName     = "BPHY22RefittedPrimaryVertices",#"SHOULDNOTBEUSED",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    #RefitPV                = True,
    #MaxPVrefit             = 100000,
    CheckCollections       = True,
    CheckVertexContainers  = ['BPHY22MuPiCandidates'])
  
ToolSvc += BPHY22DiTrkSelectAndWrite
print      BPHY22DiTrkSelectAndWrite

#--------------------------------------------------------------------
## c/ augment and select D0 candidates
BPHY22_Select_D0 = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY22_Select_D0",
    HypothesisName        = "D0",
    InputVtxContainerName = "BPHY22DiTrkCandidates",
    TrkMasses             = [139.571, 493.677],
    VtxMassHypo           = 1864.83,
    MassMin               = 1864.83-200,
    MassMax               = 1864.83+200,
    #LxyMin                = 0.0,#0.15,
    Chi2Max               = 50)

ToolSvc += BPHY22_Select_D0
print      BPHY22_Select_D0
##
#--------------------------------------------------------------------
## d/ augment and select D0bar candidates
BPHY22_Select_D0b = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY22_Select_D0b",
    HypothesisName        = "D0b",
    InputVtxContainerName = "BPHY22DiTrkCandidates",
    TrkMasses             = [493.677, 139.571],
    VtxMassHypo           = 1864.83,
    MassMin               = 1864.83-200,
    MassMax               = 1864.83+200,
    #LxyMin                = 0.0,
    Chi2Max               = 50)

ToolSvc += BPHY22_Select_D0b
print      BPHY22_Select_D0b
#==============================================================================================

#===============================================================================================

#--------------------------------------------------------------------
# 3/ select B -> mu pi D*
#--------------------------------------------------------------------
## a/ setup the cascade vertexing tool
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BMuDstVertexFit = Trk__TrkVKalVrtFitter(
    name                 = "BMuDstVertexFit",
    Extrapolator         = BPHY22_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint   = False,
    CascadeCnstPrecision = 1e-6,
    MakeExtendedVertex   = True)

ToolSvc += BMuDstVertexFit
print      BMuDstVertexFit

#--------------------------------------------------------------------
## b/ setup Jpsi D*+ finder
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__MuPlusDpstCascade
BPHY22MuDpst = DerivationFramework__MuPlusDpstCascade(
    name                     = "BPHY22MuDpst",
    HypothesisName           = "B",
    TrkVertexFitterTool      = BMuDstVertexFit,
    TrkVertexFitterToolAdd     = BPHY22_VertexTools.TrkVKalVrtFitter,

    DxHypothesis             = 421, # MC PID for D0
    ApplyD0MassConstraint    = True,
    MuPiMassLowerCut         = 200.,
    MuPiMassUpperCut         = 8200.,
    D0MassLowerCut           = 1864.83 - 200.,
    D0MassUpperCut           = 1864.83 + 200.,
    DstMassLowerCut          = 2010.26 - 300.,
    DstMassUpperCut          = 2010.26 + 300.,
    DstMassUpperCutAft       = 2010.26 + 55., #mass cut after cascade fit old 25.
    MassLowerCut             = 0.,
    MassUpperCut             = 12500.,
    Chi2Cut                  = 5, #chi2/ndf
    RefitPV                  = True,
    RefPVContainerName       = "BPHY22RefittedPrimaryVertices",
    MuPiVertices             = "BPHY22MuPiCandidates",
    CascadeVertexCollections = ["BMuDpstCascadeSV2", "BMuDpstCascadeSV1"],
    AdditionalCascadeVertexCollections = ["BMuDpstTrkCascadeSV2", "BMuDpstTrkCascadeSV1"],
    D0Vertices               = "BPHY22DiTrkCandidates",
    DoVertexType             = 15 )

ToolSvc += BPHY22MuDpst
print      BPHY22MuDpst
#===============================================================================================


#--------------------------------------------------------------------

CascadeCollections = []
CascadeCollections += BPHY22MuDpst.CascadeVertexCollections
CascadeCollections += BPHY22MuDpst.AdditionalCascadeVertexCollections

#--------------------------------------------------------------------
## 6/ select the event. We only want to keep events that
##    contain certain vertices which passed certain selection.
if not isSimulation: #Only Skim Data
   from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
   BPHY22_SelectBMuDxEvent = DerivationFramework__xAODStringSkimmingTool(
     name = "BPHY22_SelectBMuDxEvent",
     expression = "(count(BMuDpstCascadeSV1.x > -999)) > 0")

   ToolSvc += BPHY22_SelectBMuDxEvent
   print      BPHY22_SelectBMuDxEvent

   #====================================================================
   # Make event selection based on an OR of the input skimming tools
   #====================================================================
      
   from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationOR
   BPHY22SkimmingOR = CfgMgr.DerivationFramework__FilterCombinationOR(
       "BPHY22SkimmingOR",
       FilterList = [BPHY22_SelectBMuDxEvent] )
   ToolSvc += BPHY22SkimmingOR
   print      BPHY22SkimmingOR

#--------------------------------------------------------------------
##7/ track and vertex thinning. We want to remove all reconstructed secondary vertices
##    which hasn't passed any of the selections defined by (Select_*) tools.
##    We also want to keep only tracks which are associates with either muons or any of the
##    vertices that passed the selection. Multiple thinning tools can perform the
##    selection. The final thinning decision is based OR of all the decisions (by default,
##    although it can be changed by the JO).

## a) thining out vertices that didn't pass any selection and idetifying tracks associated with
##    selected vertices. The "VertexContainerNames" is a list of the vertex containers, and "PassFlags"
##    contains all pass flags for Select_* tools that must be satisfied. The vertex is kept if it
##    satisfies any of the listed selections.
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Thin_vtxTrk
BPHY22_thinningTool_Tracks = DerivationFramework__Thin_vtxTrk(
    name                       = "BPHY22_thinningTool_Tracks",
    ThinningService            = "BPHY22ThinningSvc",
    TrackParticleContainerName = "InDetTrackParticles",
    VertexContainerNames       = ["BMuDpstCascadeSV1", "BMuDpstCascadeSV2"],
    PassFlags                  = ["passed_B"])

ToolSvc += BPHY22_thinningTool_Tracks
print      BPHY22_thinningTool_Tracks

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__BPhysPVThinningTool
BPHY22_thinningTool_PV = DerivationFramework__BPhysPVThinningTool(
    name                 = "BPHY22_thinningTool_PV",
    ThinningService      = "BPHY22ThinningSvc",
    CandidateCollections = ["BMuDpstCascadeSV1", "BMuDpstCascadeSV2"],
    KeepPVTracks         = True)

ToolSvc += BPHY22_thinningTool_PV
print      BPHY22_thinningTool_PV

## b) thinning out tracks that are not attached to muons. The final thinning decision is based on the OR operation
##    between decision from this and the previous tools.
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
BPHY22MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(
    name                   = "BPHY22MuonTPThinningTool",
    ThinningService        = "BPHY22ThinningSvc",
    MuonKey                = "Muons",
    InDetTrackParticlesKey = "InDetTrackParticles")

ToolSvc += BPHY22MuonTPThinningTool
print      BPHY22MuonTPThinningTool

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS
#====================================================================

thiningCollection = []
print thiningCollection

# The name of the kernel (BPHY22Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel(
    "BPHY22Kernel",
    AugmentationTools = [BPHY22MuPiSelectAndWrite,
                         BPHY22DiTrkSelectAndWrite,
                         BPHY22MuDpst,
                         BPHY22_AugOriginalCounts],
    #Only skim if not MC
    SkimmingTools     = [BPHY22SkimmingOR] if not isSimulation else [],
    ThinningTools     = thiningCollection
    )

#====================================================================
# SET UP STREAM
#====================================================================
streamName   = derivationFlags.WriteDAOD_BPHY22Stream.StreamName
fileName     = buildFileName( derivationFlags.WriteDAOD_BPHY22Stream )
BPHY22Stream  = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY22Stream.AcceptAlgs(["BPHY22Kernel"])

# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()

BPHY22ThinningSvc = createThinningSvc( svcName="BPHY22ThinningSvc", outStreams=[evtStream] )
svcMgr += BPHY22ThinningSvc


#====================================================================
# Slimming
#====================================================================
# Added by ASC
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY22SlimmingHelper = SlimmingHelper("BPHY22SlimmingHelper")
AllVariables  = []
StaticContent = []

# Needed for trigger objects
BPHY22SlimmingHelper.IncludeMuonTriggerContent  = TRUE
BPHY22SlimmingHelper.IncludeBPhysTriggerContent = TRUE

## primary vertices
AllVariables  += ["PrimaryVertices"]
StaticContent += ["xAOD::VertexContainer#BPHY22RefittedPrimaryVertices"]
StaticContent += ["xAOD::VertexAuxContainer#BPHY22RefittedPrimaryVerticesAux."]

## ID track particles
AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are store in InDetTrackParticles collection)
AllVariables += ["CombinedMuonTrackParticles"]
AllVariables += ["ExtrapolatedMuonTrackParticles"]

## muon container
AllVariables += ["Muons"]

## Jpsi candidates
StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY22MuPiSelectAndWrite.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY22MuPiSelectAndWrite.OutputVtxContainerName]


## K+K-, Kpi, D0/D0bar candidates
#StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY22DiTrkSelectAndWrite.OutputVtxContainerName]
#StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY22DiTrkSelectAndWrite.OutputVtxContainerName]


## B+>mu D_(s)+/-, mu D*+/- and mu Lambda_c+/- candidates
for cascades in CascadeCollections:
   StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]

# Tagging information (in addition to that already requested by usual algorithms)
AllVariables += ["MuonSpectrometerTrackParticles" ]

# Added by ASC
# Truth information for MC only
if isSimulation:
    AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

AllVariables = list(set(AllVariables)) # remove duplicates

BPHY22SlimmingHelper.AllVariables = AllVariables
BPHY22SlimmingHelper.StaticContent = StaticContent
BPHY22SlimmingHelper.SmartCollections = []

BPHY22SlimmingHelper.AppendContentToStream(BPHY22Stream)


#====================================================================
# END OF BPHY22.py
#====================================================================
