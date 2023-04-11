#====================================================================
# HION13.py
# author: Xin Chen <xin.chen@cern.ch>
#====================================================================

# Set up common services and job object.
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkHI.HISkimmingTools import *
from DerivationFrameworkHI.HIDerivationFlags import HIDerivationFlags
from DerivationFrameworkHI.HIAugmentationTools import *

GetConditionsFromMetaData()
isSimulation = False
if HIDerivationFlags.isSimulation() or globalflags.DataSource()=='geant4':
    isSimulation = True

print '++++++++++++++++++++++++++++++++++ Start Checking Flags +++++++++++++++++++++++++++++++++++'
print HIDerivationFlags
print globalflags
print '+++++++++++++++++++++++++++++++++++ End Checking Flags +++++++++++++++++++++++++++++++++++'

# mass constants
Mumass = 105.658
Jpsimass = 3096.916
Psi2Smass = 3686.10
Upsimass = 9460.30

#====================================================================
# AUGMENTATION TOOLS
#====================================================================
# 1/ setup vertexing tools and services

include("DerivationFrameworkBPhys/configureVertexing.py")
HION13_VertexTools = BPHYVertexTools("HION13")

#--------------------------------------------------------------------
# 2/ Setup the vertex fitter tools (e.g. JpsiFinder, JpsiPlus1Track, etc).
#    These are general tools independent of DerivationFramework that do the
#    actual vertex fitting and some pre-selection.
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
HION13JpsiFinder = Analysis__JpsiFinder(
    name                        = "HION13JpsiFinder",
    OutputLevel                 = INFO,
    muAndMu                     = True,
    muAndTrack                  = False,
    TrackAndTrack               = False,
    assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt            = 2300.,
    invMassUpper                = 12500.0,
    invMassLower                = 2500.0,
    Chi2Cut                     = 200.,
    oppChargesOnly              = True,
    atLeastOneComb              = True,
    useCombinedMeasurement      = False, # Only takes effect if combOnly=True    
    muonCollectionKey           = "Muons",
    TrackParticleCollection     = "InDetTrackParticles",
    V0VertexFitterTool          = HION13_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                 = False,            # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool         = HION13_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool           = HION13_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool  = HION13_VertexTools.InDetConversionHelper,
    VertexPointEstimator        = HION13_VertexTools.VtxPointEstimator,
    useMCPCuts                  = False )
ToolSvc += HION13JpsiFinder

#--------------------------------------------------------------------
# 3/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
#    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic
#    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
#    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
#    Reco tool is the JpsiFinder mass window is wide enough.

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
HION13_Reco_mumu = DerivationFramework__Reco_mumu(
    name                   = "HION13_Reco_mumu",
    JpsiFinder             = HION13JpsiFinder,
    OutputVtxContainerName = "HION13OniaCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    DoVertexType           = 1)
ToolSvc += HION13_Reco_mumu

#--------------------------------------------------------------------
# 4/ setup the vertex selection and augmentation tool(s). These tools decorate the vertices with
#    variables that depend on the vertex mass hypothesis, e.g. invariant mass, proper decay time, etc.
#    Property HypothesisName is used as a prefix for these decorations.
#    They also perform tighter selection, flagging the vertecis that passed. The flag is a Char_t branch
#    named "passed_"+HypothesisName. It is used later by the "SelectEvent" and "Thin_vtxTrk" tools
#    to determine which events and candidates should be kept in the output stream.
#    Multiple instances of the Select_* tools can be used on a single input collection as long as they
#    use different "HypothesisName" flags.

from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
HION13VertexFit = Trk__TrkVKalVrtFitter(
    name                = "HION13VertexFit",
    Extrapolator        = HION13_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint  = False,
    MakeExtendedVertex  = True)
ToolSvc += HION13VertexFit

# 6/ setup the Jpsi+2 track finder
# if (HIDerivationFlags.isPPb() or HIDerivationFlags.isPP()):
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus2Tracks
HION13JpsiPlus2Tracks = Analysis__JpsiPlus2Tracks(
    name                        = "HION13JpsiPlus2Tracks",
    OutputLevel                 = INFO,
    kaonkaonHypothesis          = False,
    pionpionHypothesis          = False,
    kaonpionHypothesis          = False,
    ManualMassHypo              = [ Mumass, Mumass, Mumass, Mumass ],
    trkThresholdPt              = 2300.0,
    trkMaxEta                   = 2.6,
    oppChargesOnly              = False,
    DiTrackMassUpper            = 12500.,
    DiTrackMassLower            = 2500.,
    TrkQuadrupletMassUpper      = 25000.,
    TrkQuadrupletMassLower      = 0.,
    Chi2Cut                     = 200.0,
    JpsiContainerKey            = "HION13OniaCandidates",
    TrackParticleCollection     = "InDetTrackParticles",
    MuonsUsedInJpsi		= "Muons",
    ExcludeJpsiMuonsOnly        = True,
    RequireNMuonTracks          = 2,
    TrkVertexFitterTool		= HION13VertexFit,
    TrackSelectorTool		= HION13_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		= False)

ToolSvc += HION13JpsiPlus2Tracks

# 7/ setup the combined augmentation/skimming tool
# if (HIDerivationFlags.isPPb() or HIDerivationFlags.isPP()):
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrkTrk
HION13FourTrackReco = DerivationFramework__Reco_dimuTrkTrk(
    name                   = "HION13FourTrackReco",
    Jpsi2PlusTrackName     = HION13JpsiPlus2Tracks,
    OutputVtxContainerName = "HION13FourTrack",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "HION13RefittedPrimaryVertices",
    RefitPV                = True,
    MaxPVrefit             = 10,
    DoVertexType           = 7)
ToolSvc += HION13FourTrackReco


#====================================================================
# Revertex with mass constraint
#====================================================================

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__ReVertex
HION13_Revertex_2mu = DerivationFramework__ReVertex(
    name                       = "HION13_Revertex_2mu",
    InputVtxContainerName      = "HION13FourTrack",
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "HION13RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 100.,
    TrkVertexFitterTool	       = HION13VertexFit,
    OutputVtxContainerName     = "HION13TwoMuon")
ToolSvc += HION13_Revertex_2mu

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu
HION13_Select_TwoMuon          = DerivationFramework__Select_onia2mumu(
    name                       = "HION13_Select_TwoMuon",
    HypothesisName             = "TwoMuons",
    InputVtxContainerName      = "HION13TwoMuon",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = 2500.,
    MassMax                    = 3600.,
    DoVertexType               = 0)
ToolSvc += HION13_Select_TwoMuon

HION13_Revertex_2trk           = DerivationFramework__ReVertex(
    name                       = "HION13_Revertex_2trk",
    InputVtxContainerName      = "HION13FourTrack",
    TrackIndices               = [ 2, 3 ],
    RefitPV                    = True,
    RefPVContainerName         = "HION13RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 100.,
    TrkVertexFitterTool	       = HION13VertexFit,
    OutputVtxContainerName     = "HION13TwoTrack")
ToolSvc += HION13_Revertex_2trk

HION13_Select_TwoTrack         = DerivationFramework__Select_onia2mumu(
    name                       = "HION13_Select_TwoTrack",
    HypothesisName             = "TwoTracks",
    InputVtxContainerName      = "HION13TwoTrack",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = 2500.,
    MassMax                    = 3600.,
    DoVertexType               = 0)
ToolSvc += HION13_Select_TwoTrack


HION13_Revertex_2muHi          = DerivationFramework__ReVertex(
    name                       = "HION13_Revertex_2muHi",
    InputVtxContainerName      = "HION13FourTrack",
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "HION13RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 100.,
    TrkVertexFitterTool	       = HION13VertexFit,
    OutputVtxContainerName     = "HION13TwoMuonHi")
ToolSvc += HION13_Revertex_2muHi

HION13_Select_TwoMuonHi        = DerivationFramework__Select_onia2mumu(
    name                       = "HION13_Select_TwoMuonHi",
    HypothesisName             = "TwoMuonsHi",
    InputVtxContainerName      = "HION13TwoMuonHi",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = 8500.,
    MassMax                    = 11000.,
    DoVertexType               = 0)
ToolSvc += HION13_Select_TwoMuonHi

HION13_Revertex_2trkHi         = DerivationFramework__ReVertex(
    name                       = "HION13_Revertex_2trkHi",
    InputVtxContainerName      = "HION13FourTrack",
    TrackIndices               = [ 2, 3 ],
    RefitPV                    = True,
    RefPVContainerName         = "HION13RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 100.,
    TrkVertexFitterTool	       = HION13VertexFit,
    OutputVtxContainerName     = "HION13TwoTrackHi")
ToolSvc += HION13_Revertex_2trkHi

HION13_Select_TwoTrackHi       = DerivationFramework__Select_onia2mumu(
    name                       = "HION13_Select_TwoTrackHi",
    HypothesisName             = "TwoTracksHi",
    InputVtxContainerName      = "HION13TwoTrackHi",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = 8500.,
    MassMax                    = 11000.,
    DoVertexType               = 0)
ToolSvc += HION13_Select_TwoTrackHi


HION13_Revertex_2muMed         = DerivationFramework__ReVertex(
    name                       = "HION13_Revertex_2muMed",
    InputVtxContainerName      = "HION13FourTrack",
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "HION13RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 100.,
    TrkVertexFitterTool	       = HION13VertexFit,
    OutputVtxContainerName     = "HION13TwoMuonMed")
ToolSvc += HION13_Revertex_2muMed

HION13_Select_TwoMuonMed       = DerivationFramework__Select_onia2mumu(
    name                       = "HION13_Select_TwoMuonMed",
    HypothesisName             = "TwoMuonsMed",
    InputVtxContainerName      = "HION13TwoMuonMed",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = 3300.0,
    MassMax                    = 4500.0,
    DoVertexType               = 0)
ToolSvc += HION13_Select_TwoMuonMed

HION13_Revertex_2trkMed        = DerivationFramework__ReVertex(
    name                       = "HION13_Revertex_2trkMed",
    InputVtxContainerName      = "HION13FourTrack",
    TrackIndices               = [ 2, 3 ],
    RefitPV                    = True,
    RefPVContainerName         = "HION13RefittedPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 100.,
    TrkVertexFitterTool	       = HION13VertexFit,
    OutputVtxContainerName     = "HION13TwoTrackMed")
ToolSvc += HION13_Revertex_2trkMed

HION13_Select_TwoTrackMed      = DerivationFramework__Select_onia2mumu(
    name                       = "HION13_Select_TwoTrackMed",
    HypothesisName             = "TwoTracksMed",
    InputVtxContainerName      = "HION13TwoTrackMed",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = 3300.,
    MassMax                    = 4500.,
    DoVertexType               = 0)
ToolSvc += HION13_Select_TwoTrackMed


#--------------------------------------------------------------------
# 8/ select the event. We only want to keep events that contain certain vertices which passed certain selection.
expression = "( count(HION13TwoMuon.passed_TwoMuons) + count(HION13TwoTrack.passed_TwoTracks) > 1 || count(HION13TwoMuonMed.passed_TwoMuonsMed) + count(HION13TwoTrackMed.passed_TwoTracksMed) > 1 || count(HION13TwoMuon.passed_TwoMuons) + count(HION13TwoTrackMed.passed_TwoTracksMed) > 1 || count(HION13TwoMuonMed.passed_TwoMuonsMed) + count(HION13TwoTrack.passed_TwoTracks) > 1 || count(HION13TwoMuonHi.passed_TwoMuonsHi) + count(HION13TwoTrackHi.passed_TwoTracksHi) > 0 )"

MBTriggers = [
    "L1_ZDC_A_C_VTE50_PEB",
    "L1_TE50_VTE600.0ETA49_PEB",
    "L1_TE600.0ETA49_PEB",
    "HLT_mb_sptrk_L1ZDC_A_C_VTE50",
    "HLT_hi_hipeb_L1TE50_VTE600",
    "HLT_hi_hipeb_L1TE600",
    "HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50",
    "HLT_mb_sptrk_hipeb_L1ZDC_A_C_VTE50",
    "HLT_noalg_mb_L1TE50",
    "HLT_mb_sptrk",
    "HLT_mb_mbts_L1MBTS_1",
    "HLT_mb_sptrk_L1MBTS_1" ]

MuonTriggers = [ ]
if HIDerivationFlags.isPP():
    MuonTriggers = [
        "HLT_mu4",
        "HLT_mu4_nomucomb",
        "HLT_mu4_bJpsi_Trkloose",
        "HLT_mu6",
        "HLT_mu8",
        "HLT_mu10",
        "HLT_mu14",
        "HLT_2mu4",
        "HLT_2mu4_nomucomb",
        "HLT_mu4_mu2noL1_L1MU4",
        "HLT_mu6_mu2noL1_L1MU6" ]
elif HIDerivationFlags.isPPb():
    MuonTriggers = [
        "HLT_mu4",
        "HLT_mu6",
        "HLT_mu8",
        "HLT_2mu4",
        "HLT_mu4_bJpsi_Trkloose",
        "HLT_mu4_mb_sp1200_pusup200_trk60_hmt_L1MU4",
        "HLT_mu4_mb_sp2400_pusup500_trk120_hmt_L1MU4_TE50",
        "HLT_mu4_mb_sp2800_pusup600_trk140_hmt_L1MU4_TE50",
        "HLT_mu4_mb_sp3500_pusup800_trk180_hmt_L1MU4_TE120" ]
else:
    MuonTriggers = [
        "HLT_mu3",
        "HLT_mu4",
        "HLT_mu6",
        "HLT_mu8",
        "HLT_mu4noL1",
        "HLT_2mu3",
        "HLT_2mu4",
        "HLT_2mu4_nomucomb",
        "HLT_mu4_mu4noL1" ]

triggers  = MBTriggers + MuonTriggers
triggers  = '(' + ' || '.join(triggers) + ')'
req_total = triggers + '&&('+expression+')'
if isSimulation:
    triggers  = ""
    req_total = '('+expression+')'

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
HION13_SelectEvent = DerivationFramework__xAODStringSkimmingTool(
    name = "HION13_SelectEvent",
    expression = req_total)
ToolSvc += HION13_SelectEvent
print      HION13_SelectEvent

print '++++++++++++++++++++++++++++++++++ Start printing event selection +++++++++++++++++++++++++++++++++++'
print req_total
print '++++++++++++++++++++++++++++++++++ End printing event selection +++++++++++++++++++++++++++++++++++'

# The name of the kernel (HION13Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel(
    "HION13Kernel",
    AugmentationTools = [HION13_Reco_mumu, HION13FourTrackReco,HION13_Revertex_2mu,HION13_Select_TwoMuon,HION13_Revertex_2trk,HION13_Select_TwoTrack,HION13_Revertex_2muHi,HION13_Select_TwoMuonHi,HION13_Revertex_2trkHi,HION13_Select_TwoTrackHi,HION13_Revertex_2muMed,HION13_Select_TwoMuonMed,HION13_Revertex_2trkMed,HION13_Select_TwoTrackMed],
    SkimmingTools     = [HION13_SelectEvent] )

#====================================================================
# SET UP STREAM 
#====================================================================
streamName = derivationFlags.WriteDAOD_HION13Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_HION13Stream )
HION13Stream = MSMgr.NewPoolRootStream( streamName, fileName )
HION13Stream.AcceptAlgs(["HION13Kernel"])

#====================================================================
# Slimming
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
HION13SlimmingHelper = SlimmingHelper("HION13SlimmingHelper")
# Needed for trigger objects
HION13SlimmingHelper.IncludeMuonTriggerContent = True

# primary vertices
HION13SlimmingHelper.AllVariables  = ["Muons"]
HION13SlimmingHelper.AllVariables += [ "PrimaryVertices",
                                       "InDetTrackParticles",
                                       "CombinedMuonTrackParticles",
                                       "ExtrapolatedMuonTrackParticles",
                                       "HIEventShape",
                                       "ZdcSums" ]
HION13SlimmingHelper.AllVariables += ["MuonSegments"]

# HIGlobalVars = ['CaloSums', 'ZdcModules', 'ZdcTriggerTowers',
#                 'MBTSForwardEventInfo', 'MBTSModules']
HION13SlimmingHelper.AllVariables += HIGlobalVars

if isSimulation:
    HION13SlimmingHelper.AllVariables += ["TruthEvents", "TruthParticles", "TruthVertices", "MuonTruthParticles"]

HION13SlimmingHelper.StaticContent += ["xAOD::VertexContainer#%s" % HION13FourTrackReco.OutputVtxContainerName]
# we have to disable vxTrackAtVertex branch since it is not xAOD compatible
HION13SlimmingHelper.StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % HION13FourTrackReco.OutputVtxContainerName]

revertex_tools = [HION13_Revertex_2mu, HION13_Revertex_2trk, HION13_Revertex_2muHi, HION13_Revertex_2trkHi, HION13_Revertex_2muMed, HION13_Revertex_2trkMed]

for rev_tool in revertex_tools:
    HION13SlimmingHelper.StaticContent += ["xAOD::VertexContainer#%s" % rev_tool.OutputVtxContainerName]
    HION13SlimmingHelper.StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % rev_tool.OutputVtxContainerName]

HION13SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
HION13SlimmingHelper.AppendContentToStream(HION13Stream)
