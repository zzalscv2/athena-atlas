#====================================================================
# BPHY25.py
# Contact: xin.chen@cern.ch
#====================================================================

# Set up common services and job object. 
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *
isSimulation = DerivationFrameworkHasTruth

#====================================================================
# AUGMENTATION TOOLS 
#====================================================================
## 1/ setup vertexing tools and services
include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY25_VertexTools = BPHYVertexTools("BPHY25")

# mass limits (loose and tight versions) and constants used in the following
Jpsi_lo = 2600.0
Jpsi_hi = 3500.0
Zc_lo = 3650.0
Zc_hi = 4150.0
Psi_lo = 3350.0
Psi_hi = 4200.0
B_lo = 4850.0
B_hi = 5700.0
Kstar_lo = 640.0
Kstar_hi = 1140.0
Bs0_lo = 4950.0
Bs0_hi = 5800.0
Phi_lo = 770.0
Phi_hi = 1270.0
Upsi_lo = 8900.0
Upsi_hi = 9900.0
Ds_lo = 1660.0
Ds_hi = 2230.0
Ks_lo = 300.0
Ks_hi = 700.0
Ld_lo = 900.0
Ld_hi = 1350.0
Xi_lo = 1100.0
Xi_hi = 1550.0
Omg_lo = 1450.0
Omg_hi = 1900.0

Mumass = 105.658
Pimass = 139.570
Kmass = 493.677
Kstarmass = 895.55
Ks0mass = 497.611
Protonmass = 938.2721
Phimass = 1019.461
Dpmmass = 1869.66
Dspmmass = 1968.35
Jpsimass = 3096.916
Psi2Smass = 3686.10
X3872mass = 3871.65
Zcmass = 3887.1
Bpmmass = 5279.34
B0mass = 5279.66
Bs0mass = 5366.92
Upsimass = 9460.30
Upsi2Smass = 10023.26
Lambdamass = 1115.683
Ximass = 1321.71
Omegamass = 1672.45

#--------------------------------------------------------------------
## 2/ Setup the vertex fitter tools (e.g. JpsiFinder, JpsiPlus1Track, etc).
##    These are general tools independent of DerivationFramework that do the 
##    actual vertex fitting and some pre-selection.

# invMass range covers phi, J/psi, psi(2S), Upsi(1S) and Upsi(2S)
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY25JpsiFinder = Analysis__JpsiFinder(
    name                        = "BPHY25JpsiFinder",
    OutputLevel                 = INFO,
    muAndMu                     = True,
    muAndTrack                  = False,
    TrackAndTrack               = False,
    assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt            = 2400.,
    invMassLower                = Phi_lo,
    invMassUpper                = Upsi_hi,
    Chi2Cut                     = 10.,
    oppChargesOnly	        = True,
    atLeastOneComb              = True,
    useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
    muonCollectionKey           = "Muons",
    TrackParticleCollection     = "InDetTrackParticles",
    V0VertexFitterTool          = BPHY25_VertexTools.TrkV0Fitter, # V0 vertex fitter
    useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool         = BPHY25_VertexTools.TrkVKalVrtFitter, # VKalVrt vertex fitter
    TrackSelectorTool           = BPHY25_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool  = BPHY25_VertexTools.InDetConversionHelper,
    VertexPointEstimator        = BPHY25_VertexTools.VtxPointEstimator,
    useMCPCuts                  = False )
ToolSvc += BPHY25JpsiFinder

#--------------------------------------------------------------------
## 3/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic 
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool if the JpsiFinder mass window is wide enough.

# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/src/Reco_mumu.cxx
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY25_Reco_mumu = DerivationFramework__Reco_mumu(
    name                   = "BPHY25_Reco_mumu",
    JpsiFinder             = BPHY25JpsiFinder,
    OutputVtxContainerName = "BPHY25OniaCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    DoVertexType           = 1)
ToolSvc += BPHY25_Reco_mumu

## 4/ setup a new vertexing tool (necessary due to use of mass constraint) 
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BPHY25VertexFit = Trk__TrkVKalVrtFitter(
    name                = "BPHY25VertexFit",
    Extrapolator        = BPHY25_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint  = False,  # use Perigee strategy
    MakeExtendedVertex  = True)
ToolSvc += BPHY25VertexFit

## 5/ setup the Jpsi+2 track finder
# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/JpsiUpsilonTools/src/JpsiPlus2Tracks.cxx
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus2Tracks

# X(3872), Psi(2S) -> J/psi + pi pi
BPHY25PsiX3872_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY25PsiX3872_Jpsi2Trk",
    kaonkaonHypothesis		        = False,
    pionpionHypothesis                  = True,
    kaonpionHypothesis                  = False,
    kaonprotonHypothesis                = False,
    trkThresholdPt			= 380.,
    trkMaxEta		 	        = 2.6,
    oppChargesOnly                      = False,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkQuadrupletMassLower              = Psi_lo,
    TrkQuadrupletMassUpper              = Psi_hi,
    Chi2Cut                             = 10.,
    JpsiContainerKey                    = "BPHY25OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY25VertexFit,
    TrackSelectorTool		        = BPHY25_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY25PsiX3872_Jpsi2Trk

# Bs0 -> J/psi + K K
BPHY25Bs0_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY25Bs0_Jpsi2Trk",
    kaonkaonHypothesis		        = True,
    pionpionHypothesis                  = False,
    kaonpionHypothesis                  = False,
    kaonprotonHypothesis                = False,
    trkThresholdPt			= 380.,
    trkMaxEta		 	        = 2.6,
    oppChargesOnly                      = False,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkQuadrupletMassLower              = Bs0_lo,
    TrkQuadrupletMassUpper              = Bs0_hi,
    Chi2Cut                             = 10.,
    JpsiContainerKey                    = "BPHY25OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY25VertexFit,
    TrackSelectorTool		        = BPHY25_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY25Bs0_Jpsi2Trk

# B0 -> J/psi + K pi
BPHY25B0_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY25B0_Jpsi2Trk",
    kaonkaonHypothesis		        = False,
    pionpionHypothesis                  = False,
    kaonpionHypothesis                  = True,
    kaonprotonHypothesis                = False,
    trkThresholdPt			= 380.,
    trkMaxEta		 	        = 2.6,
    oppChargesOnly                      = False,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkQuadrupletMassLower              = B_lo,
    TrkQuadrupletMassUpper              = B_hi,
    Chi2Cut                             = 10.,
    JpsiContainerKey                    = "BPHY25OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY25VertexFit,
    TrackSelectorTool		        = BPHY25_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY25B0_Jpsi2Trk

from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus1Track

# Zc(3900)+ -> J/psi pi
BPHY25Zc3900_Jpsi1Trk = Analysis__JpsiPlus1Track(
    name                                = "BPHY25Zc3900_Jpsi1Trk",
    pionHypothesis                      = True,
    kaonHypothesis                      = False,
    trkThresholdPt                      = 380.,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkTrippletMassLower                = Zc_lo,
    TrkTrippletMassUpper                = Zc_hi,
    Chi2Cut                             = 10.0,
    JpsiContainerKey                    = "BPHY25OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY25VertexFit,
    TrackSelectorTool                   = BPHY25_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY25Zc3900_Jpsi1Trk

# B+ -> J/psi K
BPHY25Bpm_Jpsi1Trk = Analysis__JpsiPlus1Track(
    name                                = "BPHY25Bpm_Jpsi1Trk",
    pionHypothesis                      = False,
    kaonHypothesis                      = True,
    trkThresholdPt                      = 380.,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkTrippletMassLower                = B_lo,
    TrkTrippletMassUpper                = B_hi,
    Chi2Cut                             = 10.0,
    JpsiContainerKey                    = "BPHY25OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY25VertexFit,
    TrackSelectorTool                   = BPHY25_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY25Bpm_Jpsi1Trk

# D+, Ds+ -> phi pi
BPHY25DpmDs_Jpsi1Trk = Analysis__JpsiPlus1Track(
    name                                = "BPHY25DpmDs_Jpsi1Trk",
    pionHypothesis                      = True,
    kaonHypothesis                      = False,
    trkThresholdPt                      = 380.0,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = Phi_lo,
    JpsiMassUpper                       = Phi_hi,
    TrkTrippletMassLower                = Ds_lo,
    TrkTrippletMassUpper                = Ds_hi,
    Chi2Cut                             = 10.0,
    JpsiContainerKey                    = "BPHY25OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY25VertexFit,
    TrackSelectorTool                   = BPHY25_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY25DpmDs_Jpsi1Trk


## 6/ setup the combined augmentation/skimming tool
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrkTrk
BPHY25FourTrackReco_PsiX3872 = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY25FourTrackReco_PsiX3872",
    Jpsi2PlusTrackName       = BPHY25PsiX3872_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY25FourTrack_PsiX3872",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY25FourTrackReco_PsiX3872

BPHY25FourTrackReco_Bs0 = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY25FourTrackReco_Bs0",
    Jpsi2PlusTrackName       = BPHY25Bs0_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY25FourTrack_Bs0",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY25FourTrackReco_Bs0

BPHY25FourTrackReco_B0 = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY25FourTrackReco_B0",
    Jpsi2PlusTrackName       = BPHY25B0_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY25FourTrack_B0",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY25FourTrackReco_B0

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrk
BPHY25ThreeTrackReco_Zc3900 = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY25ThreeTrackReco_Zc3900",
    Jpsi1PlusTrackName       = BPHY25Zc3900_Jpsi1Trk,
    OutputVtxContainerName   = "BPHY25ThreeTrack_Zc3900",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY25ThreeTrackReco_Zc3900

BPHY25ThreeTrackReco_Bpm = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY25ThreeTrackReco_Bpm",
    Jpsi1PlusTrackName       = BPHY25Bpm_Jpsi1Trk,
    OutputVtxContainerName   = "BPHY25ThreeTrack_Bpm",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY25ThreeTrackReco_Bpm

BPHY25ThreeTrackReco_DpmDs = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY25ThreeTrackReco_DpmDs",
    Jpsi1PlusTrackName       = BPHY25DpmDs_Jpsi1Trk,
    OutputVtxContainerName   = "BPHY25ThreeTrack_DpmDs",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY25ThreeTrackReco_DpmDs

# revertex with mass constraints to reduce combinatorics
# Psi(2S) -> J/psi pi pi
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__ReVertex
BPHY25Rev_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_Psi",
    InputVtxContainerName      = "BPHY25FourTrack_PsiX3872",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_Psi")
ToolSvc += BPHY25Rev_Psi

# X(3872) -> J/psi pi pi
BPHY25Rev_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_X3872",
    InputVtxContainerName      = "BPHY25FourTrack_PsiX3872",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_X3872")
ToolSvc += BPHY25Rev_X3872

# Bs0 -> J/psi K K
BPHY25Rev_Bs0 = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_Bs0",
    InputVtxContainerName      = "BPHY25FourTrack_Bs0",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_Bs0")
ToolSvc += BPHY25Rev_Bs0

# B0 -> J/psi K pi
BPHY25Rev_B0Kpi = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_B0Kpi",
    InputVtxContainerName      = "BPHY25FourTrack_B0",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_B0Kpi")
ToolSvc += BPHY25Rev_B0Kpi

# B0 -> J/psi pi K
BPHY25Rev_B0piK = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_B0piK",
    InputVtxContainerName      = "BPHY25FourTrack_B0",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Kmass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_B0piK")
ToolSvc += BPHY25Rev_B0piK

# Zc3900 -> J/psi pi
BPHY25Rev_Zc3900 = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_Zc3900",
    InputVtxContainerName      = "BPHY25ThreeTrack_Zc3900",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 15.,
    BMassLower                 = Zc_lo,
    BMassUpper                 = Zc_hi,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_Zc3900")
ToolSvc += BPHY25Rev_Zc3900

# Bpm -> J/psi K
BPHY25Rev_Bpm = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_Bpm",
    InputVtxContainerName      = "BPHY25ThreeTrack_Bpm",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_Bpm")
ToolSvc += BPHY25Rev_Bpm

# Ds -> phi pi
BPHY25Rev_Ds = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_Ds",
    InputVtxContainerName      = "BPHY25ThreeTrack_DpmDs",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_Ds")
ToolSvc += BPHY25Rev_Ds

# Dpm -> phi pi
BPHY25Rev_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY25Rev_Dpm",
    InputVtxContainerName      = "BPHY25ThreeTrack_DpmDs",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY25VertexFit,
    OutputVtxContainerName     = "BPHY25Revtx_Dpm")
ToolSvc += BPHY25Rev_Dpm


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu

BPHY25Select_Phi               = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY25Select_Phi",
    HypothesisName             = "Phi",
    InputVtxContainerName      = "BPHY25OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Phi_lo,
    MassMax                    = Phi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY25Select_Phi

BPHY25Select_Jpsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY25Select_Jpsi",
    HypothesisName             = "Jpsi",
    InputVtxContainerName      = "BPHY25OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Jpsi_lo,
    MassMax                    = Jpsi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY25Select_Jpsi

BPHY25Select_Psi               = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY25Select_Psi",
    HypothesisName             = "Psi",
    InputVtxContainerName      = "BPHY25OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Psi_lo,
    MassMax                    = Psi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY25Select_Psi

BPHY25Select_Upsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY25Select_Upsi",
    HypothesisName             = "Upsi",
    InputVtxContainerName      = "BPHY25OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Upsi_lo,
    MassMax                    = Upsi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY25Select_Upsi

## V0 vertices
include("DerivationFrameworkBPhys/configureSimpleV0Finder.py")

BPHY25_V0FinderTools = BPHYV0FinderTools("BPHY25_V0FinderTools")
BPHY25_V0FinderTools.V0FinderTool.doSimpleV0          = True
BPHY25_V0FinderTools.V0FinderTool.useV0Fitter         = True
BPHY25_V0FinderTools.V0FinderTool.useorigin           = True
BPHY25_V0FinderTools.V0FinderTool.AddSameSign         = False
BPHY25_V0FinderTools.V0FinderTool.trkSelPV            = True
BPHY25_V0FinderTools.V0FinderTool.useVertexCollection = False
BPHY25_V0FinderTools.V0FinderTool.d0_cut              = 3.
BPHY25_V0FinderTools.V0FinderTool.vert_lxy_sig        = 5
BPHY25_V0FinderTools.V0FinderTool.vert_a0xy_cut       = 1.0e9
BPHY25_V0FinderTools.V0FinderTool.vert_a0z_cut        = 1.0e9
BPHY25_V0FinderTools.InDetV0VxTrackSelector.maxTrtD0           = 100.
BPHY25_V0FinderTools.InDetV0VxTrackSelector.significanceD0_Si  = 3.
BPHY25_V0FinderTools.InDetV0VxTrackSelector.significanceD0_Trt = 3.
BPHY25_V0FinderTools.InDetV0VxTrackSelector.significanceZ0_Trt = 3. 

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_V0Finder
BPHY25_RecoV0Finder = DerivationFramework__Reco_V0Finder(
    name                   = "BPHY25_RecoV0Finder",
    CheckVertexContainers  = ["BPHY25OniaCandidates"],
    V0FinderTool           = BPHY25_V0FinderTools.V0FinderTool,
    V0ContainerName        = "BPHY25V0Candidates")
ToolSvc += BPHY25_RecoV0Finder


########################
###   2 trks + Ks0   ###
########################

list_2trkKs_hypo = ["Psi2Ks", "X3872Ks", "Bs2KKs", "B0KpiKs", "B0piKKs"]
list_2trkKs_jxInput = ["BPHY25Revtx_Psi", "BPHY25Revtx_X3872", "BPHY25Revtx_Bs0", "BPHY25Revtx_B0Kpi", "BPHY25Revtx_B0piK"]
list_2trkKs_jxMass = [Psi2Smass, X3872mass, Bs0mass, B0mass, B0mass]
list_2trkKs_jpsiMass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass]
list_2trkKs_jxDau3Mass = [Pimass, Pimass, Kmass, Kmass, Pimass]
list_2trkKs_jxDau4Mass = [Pimass, Pimass, Kmass, Pimass, Kmass]

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiXPlusDisplaced

list_2trkKs_obj = []
for hypo in list_2trkKs_hypo:
    list_2trkKs_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_2trkKs_obj

for i in range(len(list_2trkKs_obj)):
    list_2trkKs_obj[i].JXVertices               = list_2trkKs_jxInput[i]
    if i == 0:
        list_2trkKs_obj[i].V0Vertices            = "BPHY25V0Candidates"
        list_2trkKs_obj[i].RefitV0               = True
        list_2trkKs_obj[i].OutoutV0VtxCollection = "KsCollection"
        list_2trkKs_obj[i].V0MassLowerCut        = Ks_lo
        list_2trkKs_obj[i].V0MassUpperCut        = Ks_hi
    else:
        list_2trkKs_obj[i].V0Vertices            = "KsCollection"
        list_2trkKs_obj[i].RefitV0               = False
    list_2trkKs_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkKs_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkKs_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2trkKs_hypo[i]+"_CascadeVtx3"]
    list_2trkKs_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trkKs_obj[i].V0Hypothesis             = "Ks"
    list_2trkKs_obj[i].MassCutGamma             = 100.
    list_2trkKs_obj[i].Chi2CutGamma             = 5.
    list_2trkKs_obj[i].LxyV0Cut                 = 25.
    list_2trkKs_obj[i].HypothesisName           = list_2trkKs_hypo[i]
    list_2trkKs_obj[i].NumberOfJXDaughters      = 4
    list_2trkKs_obj[i].JXDaug1MassHypo          = Mumass
    list_2trkKs_obj[i].JXDaug2MassHypo          = Mumass
    list_2trkKs_obj[i].JXDaug3MassHypo          = list_2trkKs_jxDau3Mass[i]
    list_2trkKs_obj[i].JXDaug4MassHypo          = list_2trkKs_jxDau4Mass[i]
    list_2trkKs_obj[i].NumberOfDisVDaughters    = 2
    list_2trkKs_obj[i].JXMass                   = list_2trkKs_jxMass[i]
    list_2trkKs_obj[i].JpsiMass                 = list_2trkKs_jpsiMass[i]
    list_2trkKs_obj[i].V0Mass                   = Ks0mass
    list_2trkKs_obj[i].ApplyJXMassConstraint    = True
    list_2trkKs_obj[i].ApplyJpsiMassConstraint  = True
    list_2trkKs_obj[i].ApplyV0MassConstraint    = True
    list_2trkKs_obj[i].Chi2CutV0                = 10.
    list_2trkKs_obj[i].Chi2Cut                  = 8.
    list_2trkKs_obj[i].Trackd0Cut               = 3.
    list_2trkKs_obj[i].MaxJXCandidates          = 15
    list_2trkKs_obj[i].MaxV0Candidates          = 15
    list_2trkKs_obj[i].RefitPV                  = True
    list_2trkKs_obj[i].MaxnPV                   = 20
    list_2trkKs_obj[i].RefPVContainerName       = "BPHY25_"+list_2trkKs_hypo[i]+"_RefPrimaryVertices"
    list_2trkKs_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_2trkKs_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_2trkKs_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_2trkKs_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
### 2 trks + Lambda  ###
########################

list_2trkLd_hypo = ["Psi2Ld", "X3872Ld", "Bs2KLd", "B0KpiLd", "B0piKLd"]
list_2trkLd_jxInput = ["BPHY25Revtx_Psi", "BPHY25Revtx_X3872", "BPHY25Revtx_Bs0", "BPHY25Revtx_B0Kpi", "BPHY25Revtx_B0piK"]
list_2trkLd_jxMass = [Psi2Smass, X3872mass, Bs0mass, B0mass, B0mass]
list_2trkLd_jpsiMass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass]
list_2trkLd_jxDau3Mass = [Pimass, Pimass, Kmass, Kmass, Pimass]
list_2trkLd_jxDau4Mass = [Pimass, Pimass, Kmass, Pimass, Kmass]

list_2trkLd_obj = []
for hypo in list_2trkLd_hypo:
    list_2trkLd_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_2trkLd_obj

for i in range(len(list_2trkLd_obj)):
    list_2trkLd_obj[i].JXVertices               = list_2trkLd_jxInput[i]
    if i == 0:
        list_2trkLd_obj[i].V0Vertices            = "BPHY25V0Candidates"
        list_2trkLd_obj[i].RefitV0               = True
        list_2trkLd_obj[i].OutoutV0VtxCollection = "LambdaCollection"
        list_2trkLd_obj[i].V0MassLowerCut        = Ld_lo
        list_2trkLd_obj[i].V0MassUpperCut        = Ld_hi
    else:
        list_2trkLd_obj[i].V0Vertices            = "LambdaCollection"
        list_2trkLd_obj[i].RefitV0               = False
    list_2trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2trkLd_hypo[i]+"_CascadeVtx3"]
    list_2trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trkLd_obj[i].V0Hypothesis             = "Lambda"
    list_2trkLd_obj[i].MassCutGamma             = 10.
    list_2trkLd_obj[i].Chi2CutGamma             = 5.
    list_2trkLd_obj[i].LxyV0Cut                 = 25.
    list_2trkLd_obj[i].HypothesisName           = list_2trkLd_hypo[i]
    list_2trkLd_obj[i].NumberOfJXDaughters      = 4
    list_2trkLd_obj[i].JXDaug1MassHypo          = Mumass
    list_2trkLd_obj[i].JXDaug2MassHypo          = Mumass
    list_2trkLd_obj[i].JXDaug3MassHypo          = list_2trkLd_jxDau3Mass[i]
    list_2trkLd_obj[i].JXDaug4MassHypo          = list_2trkLd_jxDau4Mass[i]
    list_2trkLd_obj[i].NumberOfDisVDaughters    = 2
    list_2trkLd_obj[i].JXMass                   = list_2trkLd_jxMass[i]
    list_2trkLd_obj[i].JpsiMass                 = list_2trkLd_jpsiMass[i]
    list_2trkLd_obj[i].V0Mass                   = Lambdamass
    list_2trkLd_obj[i].ApplyJXMassConstraint    = True
    list_2trkLd_obj[i].ApplyJpsiMassConstraint  = True
    list_2trkLd_obj[i].ApplyV0MassConstraint    = True
    list_2trkLd_obj[i].Chi2CutV0                = 10.
    list_2trkLd_obj[i].Chi2Cut                  = 8.
    list_2trkLd_obj[i].Trackd0Cut               = 3.
    list_2trkLd_obj[i].MaxJXCandidates          = 15
    list_2trkLd_obj[i].MaxV0Candidates          = 15
    list_2trkLd_obj[i].RefitPV                  = True
    list_2trkLd_obj[i].MaxnPV                   = 20
    list_2trkLd_obj[i].RefPVContainerName       = "BPHY25_"+list_2trkLd_hypo[i]+"_RefPrimaryVertices"
    list_2trkLd_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_2trkLd_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_2trkLd_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_2trkLd_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###    2 trks + Xi   ###
########################

list_2trkXi_hypo = ["Psi2Xi", "X3872Xi", "Bs2KXi", "B0KpiXi", "B0piKXi"]
list_2trkXi_jxInput = ["BPHY25Revtx_Psi", "BPHY25Revtx_X3872", "BPHY25Revtx_Bs0", "BPHY25Revtx_B0Kpi", "BPHY25Revtx_B0piK"]
list_2trkXi_jxMass = [Psi2Smass, X3872mass, Bs0mass, B0mass, B0mass]
list_2trkXi_jpsiMass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass]
list_2trkXi_jxDau3Mass = [Pimass, Pimass, Kmass, Kmass, Pimass]
list_2trkXi_jxDau4Mass = [Pimass, Pimass, Kmass, Pimass, Kmass]

list_2trkXi_obj = []
for hypo in list_2trkXi_hypo:
    list_2trkXi_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_2trkXi_obj

for i in range(len(list_2trkXi_obj)):
    list_2trkXi_obj[i].JXVertices               = list_2trkXi_jxInput[i]
    list_2trkXi_obj[i].V0Vertices               = "LambdaCollection"
    list_2trkXi_obj[i].RefitV0                  = False
    if i == 0:
        list_2trkXi_obj[i].OutoutDisVtxCollection = "XiCollection"
        list_2trkXi_obj[i].DisplacedMassLowerCut  = Xi_lo
        list_2trkXi_obj[i].DisplacedMassUpperCut  = Xi_hi
        list_2trkXi_obj[i].Chi2CutDisV            = 10.
    else:
        list_2trkXi_obj[i].DisplacedVertices      = "XiCollection"
    list_2trkXi_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkXi_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkXi_hypo[i]+"_CascadeVtx2_sub","BPHY25_"+list_2trkXi_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2trkXi_hypo[i]+"_CascadeVtx3"]
    list_2trkXi_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trkXi_obj[i].V0Hypothesis             = "Lambda"
    list_2trkXi_obj[i].MassCutGamma             = 10.
    list_2trkXi_obj[i].Chi2CutGamma             = 5.
    list_2trkXi_obj[i].LxyV0Cut                 = 25.
    list_2trkXi_obj[i].LxyDisVtxCut             = 25.
    list_2trkXi_obj[i].HypothesisName           = list_2trkXi_hypo[i]
    list_2trkXi_obj[i].NumberOfJXDaughters      = 4
    list_2trkXi_obj[i].JXDaug1MassHypo          = Mumass
    list_2trkXi_obj[i].JXDaug2MassHypo          = Mumass
    list_2trkXi_obj[i].JXDaug3MassHypo          = list_2trkXi_jxDau3Mass[i]
    list_2trkXi_obj[i].JXDaug4MassHypo          = list_2trkXi_jxDau4Mass[i]
    list_2trkXi_obj[i].NumberOfDisVDaughters    = 3
    list_2trkXi_obj[i].DisVDaug3MassHypo        = Pimass
    list_2trkXi_obj[i].JXMass                   = list_2trkXi_jxMass[i]
    list_2trkXi_obj[i].JpsiMass                 = list_2trkXi_jpsiMass[i]
    list_2trkXi_obj[i].V0Mass                   = Lambdamass
    list_2trkXi_obj[i].DisVtxMass               = Ximass
    list_2trkXi_obj[i].ApplyJXMassConstraint    = True
    list_2trkXi_obj[i].ApplyJpsiMassConstraint  = True
    list_2trkXi_obj[i].ApplyV0MassConstraint    = True
    list_2trkXi_obj[i].ApplyDisVMassConstraint  = True
    list_2trkXi_obj[i].Chi2CutV0                = 10.
    list_2trkXi_obj[i].Chi2Cut                  = 8.
    list_2trkXi_obj[i].Trackd0Cut               = 3.
    list_2trkXi_obj[i].MaxJXCandidates          = 15
    list_2trkXi_obj[i].MaxV0Candidates          = 15
    list_2trkXi_obj[i].MaxDisVCandidates        = 15
    list_2trkXi_obj[i].RefitPV                  = True
    list_2trkXi_obj[i].MaxnPV                   = 20
    list_2trkXi_obj[i].RefPVContainerName       = "BPHY25_"+list_2trkXi_hypo[i]+"_RefPrimaryVertices"
    list_2trkXi_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_2trkXi_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_2trkXi_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_2trkXi_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###   2 trks + Omg   ###
########################

list_2trkOmg_hypo = ["Psi2Omg", "X3872Omg", "Bs2KOmg", "B0KpiOmg", "B0piKOmg"]
list_2trkOmg_jxInput = ["BPHY25Revtx_Psi", "BPHY25Revtx_X3872", "BPHY25Revtx_Bs0", "BPHY25Revtx_B0Kpi", "BPHY25Revtx_B0piK"]
list_2trkOmg_jxMass = [Psi2Smass, X3872mass, Bs0mass, B0mass, B0mass]
list_2trkOmg_jpsiMass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass]
list_2trkOmg_jxDau3Mass = [Pimass, Pimass, Kmass, Kmass, Pimass]
list_2trkOmg_jxDau4Mass = [Pimass, Pimass, Kmass, Pimass, Kmass]

list_2trkOmg_obj = []
for hypo in list_2trkOmg_hypo:
    list_2trkOmg_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_2trkOmg_obj

for i in range(len(list_2trkOmg_obj)):
    list_2trkOmg_obj[i].JXVertices               = list_2trkOmg_jxInput[i]
    list_2trkOmg_obj[i].V0Vertices               = "LambdaCollection"
    list_2trkOmg_obj[i].RefitV0                  = False
    if i == 0:
        list_2trkOmg_obj[i].OutoutDisVtxCollection = "OmegaCollection"
        list_2trkOmg_obj[i].DisplacedMassLowerCut  = Omg_lo
        list_2trkOmg_obj[i].DisplacedMassUpperCut  = Omg_hi
        list_2trkOmg_obj[i].Chi2CutDisV            = 10.
    else:
        list_2trkOmg_obj[i].DisplacedVertices      = "OmegaCollection"
    list_2trkOmg_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeVtx2_sub","BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeVtx3"]
    list_2trkOmg_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trkOmg_obj[i].V0Hypothesis             = "Lambda"
    list_2trkOmg_obj[i].MassCutGamma             = 10.
    list_2trkOmg_obj[i].Chi2CutGamma             = 5.
    list_2trkOmg_obj[i].LxyV0Cut                 = 25.
    list_2trkOmg_obj[i].LxyDisVtxCut             = 25.
    list_2trkOmg_obj[i].HypothesisName           = list_2trkOmg_hypo[i]
    list_2trkOmg_obj[i].NumberOfJXDaughters      = 4
    list_2trkOmg_obj[i].JXDaug1MassHypo          = Mumass
    list_2trkOmg_obj[i].JXDaug2MassHypo          = Mumass
    list_2trkOmg_obj[i].JXDaug3MassHypo          = list_2trkOmg_jxDau3Mass[i]
    list_2trkOmg_obj[i].JXDaug4MassHypo          = list_2trkOmg_jxDau4Mass[i]
    list_2trkOmg_obj[i].NumberOfDisVDaughters    = 3
    list_2trkOmg_obj[i].DisVDaug3MassHypo        = Kmass
    list_2trkOmg_obj[i].JXMass                   = list_2trkOmg_jxMass[i]
    list_2trkOmg_obj[i].JpsiMass                 = list_2trkOmg_jpsiMass[i]
    list_2trkOmg_obj[i].V0Mass                   = Lambdamass
    list_2trkOmg_obj[i].DisVtxMass               = Omegamass
    list_2trkOmg_obj[i].ApplyJXMassConstraint    = True
    list_2trkOmg_obj[i].ApplyJpsiMassConstraint  = True
    list_2trkOmg_obj[i].ApplyV0MassConstraint    = True
    list_2trkOmg_obj[i].ApplyDisVMassConstraint  = True
    list_2trkOmg_obj[i].Chi2CutV0                = 10.
    list_2trkOmg_obj[i].Chi2Cut                  = 8.
    list_2trkOmg_obj[i].Trackd0Cut               = 3.
    list_2trkOmg_obj[i].MaxJXCandidates          = 15
    list_2trkOmg_obj[i].MaxV0Candidates          = 15
    list_2trkOmg_obj[i].MaxDisVCandidates        = 15
    list_2trkOmg_obj[i].RefitPV                  = True
    list_2trkOmg_obj[i].MaxnPV                   = 20
    list_2trkOmg_obj[i].RefPVContainerName       = "BPHY25_"+list_2trkOmg_hypo[i]+"_RefPrimaryVertices"
    list_2trkOmg_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_2trkOmg_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_2trkOmg_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_2trkOmg_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###   1 trk + Ks0    ###
########################

list_1trkKs_hypo = ["Zc3900Ks", "BpmKs", "DsKs", "DpmKs"]
list_1trkKs_jxInput = ["BPHY25Revtx_Zc3900", "BPHY25Revtx_Bpm", "BPHY25Revtx_Ds", "BPHY25Revtx_Dpm"]
list_1trkKs_jxMass = [Zcmass, Bpmmass, Dspmmass, Dpmmass]
list_1trkKs_jpsiMass = [Jpsimass, Jpsimass, Phimass, Phimass]
list_1trkKs_jxDau3Mass = [Pimass, Kmass, Pimass, Pimass]

list_1trkKs_obj = []
for hypo in list_1trkKs_hypo:
    list_1trkKs_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_1trkKs_obj

for i in range(len(list_1trkKs_obj)):
    list_1trkKs_obj[i].JXVertices               = list_1trkKs_jxInput[i]
    list_1trkKs_obj[i].V0Vertices               = "KsCollection"
    list_1trkKs_obj[i].RefitV0                  = False
    list_1trkKs_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkKs_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkKs_hypo[i]+"_CascadeVtx2","BPHY25_"+list_1trkKs_hypo[i]+"_CascadeVtx3"]
    list_1trkKs_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_1trkKs_obj[i].V0Hypothesis             = "Ks"
    list_1trkKs_obj[i].MassCutGamma             = 100.
    list_1trkKs_obj[i].Chi2CutGamma             = 5.
    list_1trkKs_obj[i].LxyV0Cut                 = 25.
    list_1trkKs_obj[i].HypothesisName           = list_1trkKs_hypo[i]
    list_1trkKs_obj[i].NumberOfJXDaughters      = 3
    list_1trkKs_obj[i].JXDaug1MassHypo          = Mumass
    list_1trkKs_obj[i].JXDaug2MassHypo          = Mumass
    list_1trkKs_obj[i].JXDaug3MassHypo          = list_1trkKs_jxDau3Mass[i]
    list_1trkKs_obj[i].NumberOfDisVDaughters    = 2
    list_1trkKs_obj[i].JXMass                   = list_1trkKs_jxMass[i]
    list_1trkKs_obj[i].JpsiMass                 = list_1trkKs_jpsiMass[i]
    list_1trkKs_obj[i].V0Mass                   = Ks0mass
    list_1trkKs_obj[i].ApplyJXMassConstraint    = True
    list_1trkKs_obj[i].ApplyJpsiMassConstraint  = True
    list_1trkKs_obj[i].ApplyV0MassConstraint    = True
    list_1trkKs_obj[i].Chi2CutV0                = 10.
    list_1trkKs_obj[i].Chi2Cut                  = 8.
    list_1trkKs_obj[i].Trackd0Cut               = 3.
    list_1trkKs_obj[i].MaxJXCandidates          = 15
    list_1trkKs_obj[i].MaxV0Candidates          = 15
    list_1trkKs_obj[i].RefitPV                  = True
    list_1trkKs_obj[i].MaxnPV                   = 20
    list_1trkKs_obj[i].RefPVContainerName       = "BPHY25_"+list_1trkKs_hypo[i]+"_RefPrimaryVertices"
    list_1trkKs_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_1trkKs_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_1trkKs_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_1trkKs_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###  1 trk + Lambda  ###
########################

list_1trkLd_hypo = ["Zc3900Ld", "BpmLd", "DsLd", "DpmLd"]
list_1trkLd_jxInput = ["BPHY25Revtx_Zc3900", "BPHY25Revtx_Bpm", "BPHY25Revtx_Ds", "BPHY25Revtx_Dpm"]
list_1trkLd_jxMass = [Zcmass, Bpmmass, Dspmmass, Dpmmass]
list_1trkLd_jpsiMass = [Jpsimass, Jpsimass, Phimass, Phimass]
list_1trkLd_jxDau3Mass = [Pimass, Kmass, Pimass, Pimass]

list_1trkLd_obj = []
for hypo in list_1trkLd_hypo:
    list_1trkLd_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_1trkLd_obj

for i in range(len(list_1trkLd_obj)):
    list_1trkLd_obj[i].JXVertices               = list_1trkLd_jxInput[i]
    list_1trkLd_obj[i].V0Vertices               = "LambdaCollection"
    list_1trkLd_obj[i].RefitV0                  = False
    list_1trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_1trkLd_hypo[i]+"_CascadeVtx3"]
    list_1trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_1trkLd_obj[i].V0Hypothesis             = "Lambda"
    list_1trkLd_obj[i].MassCutGamma             = 10.
    list_1trkLd_obj[i].Chi2CutGamma             = 5.
    list_1trkLd_obj[i].LxyV0Cut                 = 25.
    list_1trkLd_obj[i].HypothesisName           = list_1trkLd_hypo[i]
    list_1trkLd_obj[i].NumberOfJXDaughters      = 3
    list_1trkLd_obj[i].JXDaug1MassHypo          = Mumass
    list_1trkLd_obj[i].JXDaug2MassHypo          = Mumass
    list_1trkLd_obj[i].JXDaug3MassHypo          = list_1trkLd_jxDau3Mass[i]
    list_1trkLd_obj[i].NumberOfDisVDaughters    = 2
    list_1trkLd_obj[i].JXMass                   = list_1trkLd_jxMass[i]
    list_1trkLd_obj[i].JpsiMass                 = list_1trkLd_jpsiMass[i]
    list_1trkLd_obj[i].V0Mass                   = Lambdamass
    list_1trkLd_obj[i].ApplyJXMassConstraint    = True
    list_1trkLd_obj[i].ApplyJpsiMassConstraint  = True
    list_1trkLd_obj[i].ApplyV0MassConstraint    = True
    list_1trkLd_obj[i].Chi2CutV0                = 10.
    list_1trkLd_obj[i].Chi2Cut                  = 8.
    list_1trkLd_obj[i].Trackd0Cut               = 3.
    list_1trkLd_obj[i].MaxJXCandidates          = 15
    list_1trkLd_obj[i].MaxV0Candidates          = 15
    list_1trkLd_obj[i].RefitPV                  = True
    list_1trkLd_obj[i].MaxnPV                   = 20
    list_1trkLd_obj[i].RefPVContainerName       = "BPHY25_"+list_1trkLd_hypo[i]+"_RefPrimaryVertices"
    list_1trkLd_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_1trkLd_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_1trkLd_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_1trkLd_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###    1 trk + Xi    ###
########################

list_1trkXi_hypo = ["Zc3900Xi", "BpmXi", "DsXi", "DpmXi"]
list_1trkXi_jxInput = ["BPHY25Revtx_Zc3900", "BPHY25Revtx_Bpm", "BPHY25Revtx_Ds", "BPHY25Revtx_Dpm"]
list_1trkXi_jxMass = [Zcmass, Bpmmass, Dspmmass, Dpmmass]
list_1trkXi_jpsiMass = [Jpsimass, Jpsimass, Phimass, Phimass]
list_1trkXi_jxDau3Mass = [Pimass, Kmass, Pimass, Pimass]

list_1trkXi_obj = []
for hypo in list_1trkXi_hypo:
    list_1trkXi_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_1trkXi_obj

for i in range(len(list_1trkXi_obj)):
    list_1trkXi_obj[i].JXVertices               = list_1trkXi_jxInput[i]
    list_1trkXi_obj[i].V0Vertices               = "LambdaCollection"
    list_1trkXi_obj[i].RefitV0                  = False
    list_1trkXi_obj[i].DisplacedVertices        = "XiCollection"
    list_1trkXi_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx2_sub","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx2","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx3"]
    list_1trkXi_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_1trkXi_obj[i].V0Hypothesis             = "Lambda"
    list_1trkXi_obj[i].MassCutGamma             = 10.
    list_1trkXi_obj[i].Chi2CutGamma             = 5.
    list_1trkXi_obj[i].LxyV0Cut                 = 25.
    list_1trkXi_obj[i].LxyDisVtxCut             = 25.
    list_1trkXi_obj[i].HypothesisName           = list_1trkXi_hypo[i]
    list_1trkXi_obj[i].NumberOfJXDaughters      = 3
    list_1trkXi_obj[i].JXDaug1MassHypo          = Mumass
    list_1trkXi_obj[i].JXDaug2MassHypo          = Mumass
    list_1trkXi_obj[i].JXDaug3MassHypo          = list_1trkXi_jxDau3Mass[i]
    list_1trkXi_obj[i].NumberOfDisVDaughters    = 3
    list_1trkXi_obj[i].DisVDaug3MassHypo        = Pimass
    list_1trkXi_obj[i].JXMass                   = list_1trkXi_jxMass[i]
    list_1trkXi_obj[i].JpsiMass                 = list_1trkXi_jpsiMass[i]
    list_1trkXi_obj[i].V0Mass                   = Lambdamass
    list_1trkXi_obj[i].DisVtxMass               = Ximass
    list_1trkXi_obj[i].ApplyJXMassConstraint    = True
    list_1trkXi_obj[i].ApplyJpsiMassConstraint  = True
    list_1trkXi_obj[i].ApplyV0MassConstraint    = True
    list_1trkXi_obj[i].ApplyDisVMassConstraint  = True
    list_1trkXi_obj[i].Chi2CutV0                = 10.
    list_1trkXi_obj[i].Chi2Cut                  = 8.
    list_1trkXi_obj[i].Trackd0Cut               = 3.
    list_1trkXi_obj[i].MaxJXCandidates          = 15
    list_1trkXi_obj[i].MaxV0Candidates          = 15
    list_1trkXi_obj[i].MaxDisVCandidates        = 15
    list_1trkXi_obj[i].RefitPV                  = True
    list_1trkXi_obj[i].MaxnPV                   = 20
    list_1trkXi_obj[i].RefPVContainerName       = "BPHY25_"+list_1trkXi_hypo[i]+"_RefPrimaryVertices"
    list_1trkXi_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_1trkXi_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_1trkXi_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_1trkXi_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###   1 trk + Omg    ###
########################

list_1trkOmg_hypo = ["Zc3900Omg", "BpmOmg", "DsOmg", "DpmOmg"]
list_1trkOmg_jxInput = ["BPHY25Revtx_Zc3900", "BPHY25Revtx_Bpm", "BPHY25Revtx_Ds", "BPHY25Revtx_Dpm"]
list_1trkOmg_jxMass = [Zcmass, Bpmmass, Dspmmass, Dpmmass]
list_1trkOmg_jpsiMass = [Jpsimass, Jpsimass, Phimass, Phimass]
list_1trkOmg_jxDau3Mass = [Pimass, Kmass, Pimass, Pimass]

list_1trkOmg_obj = []
for hypo in list_1trkOmg_hypo:
    list_1trkOmg_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_1trkOmg_obj

for i in range(len(list_1trkOmg_obj)):
    list_1trkOmg_obj[i].JXVertices               = list_1trkOmg_jxInput[i]
    list_1trkOmg_obj[i].V0Vertices               = "LambdaCollection"
    list_1trkOmg_obj[i].RefitV0                  = False
    list_1trkOmg_obj[i].DisplacedVertices        = "OmegaCollection"
    list_1trkOmg_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx2_sub","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx2","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx3"]
    list_1trkOmg_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_1trkOmg_obj[i].V0Hypothesis             = "Lambda"
    list_1trkOmg_obj[i].MassCutGamma             = 10.
    list_1trkOmg_obj[i].Chi2CutGamma             = 5.
    list_1trkOmg_obj[i].LxyV0Cut                 = 25.
    list_1trkOmg_obj[i].LxyDisVtxCut             = 25.
    list_1trkOmg_obj[i].HypothesisName           = list_1trkOmg_hypo[i]
    list_1trkOmg_obj[i].NumberOfJXDaughters      = 3
    list_1trkOmg_obj[i].JXDaug1MassHypo          = Mumass
    list_1trkOmg_obj[i].JXDaug2MassHypo          = Mumass
    list_1trkOmg_obj[i].JXDaug3MassHypo          = list_1trkOmg_jxDau3Mass[i]
    list_1trkOmg_obj[i].NumberOfDisVDaughters    = 3
    list_1trkOmg_obj[i].DisVDaug3MassHypo        = Kmass
    list_1trkOmg_obj[i].JXMass                   = list_1trkOmg_jxMass[i]
    list_1trkOmg_obj[i].JpsiMass                 = list_1trkOmg_jpsiMass[i]
    list_1trkOmg_obj[i].V0Mass                   = Lambdamass
    list_1trkOmg_obj[i].DisVtxMass               = Omegamass
    list_1trkOmg_obj[i].ApplyJXMassConstraint    = True
    list_1trkOmg_obj[i].ApplyJpsiMassConstraint  = True
    list_1trkOmg_obj[i].ApplyV0MassConstraint    = True
    list_1trkOmg_obj[i].ApplyDisVMassConstraint  = True
    list_1trkOmg_obj[i].Chi2CutV0                = 10.
    list_1trkOmg_obj[i].Chi2Cut                  = 8.
    list_1trkOmg_obj[i].Trackd0Cut               = 3.
    list_1trkOmg_obj[i].MaxJXCandidates          = 15
    list_1trkOmg_obj[i].MaxV0Candidates          = 15
    list_1trkOmg_obj[i].MaxDisVCandidates        = 15
    list_1trkOmg_obj[i].RefitPV                  = True
    list_1trkOmg_obj[i].MaxnPV                   = 20
    list_1trkOmg_obj[i].RefPVContainerName       = "BPHY25_"+list_1trkOmg_hypo[i]+"_RefPrimaryVertices"
    list_1trkOmg_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_1trkOmg_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_1trkOmg_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_1trkOmg_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###   0 trk + Ks0    ###
########################

list_0trkKs_hypo = ["PhiKs", "JpsiKs", "Psi0Ks", "UpsiKs"]
list_0trkKs_jxHypo = ["Phi", "Jpsi", "Psi", "Upsi"]
list_0trkKs_jxMass = [Phimass, Jpsimass, Psi2Smass, Upsimass]

list_0trkKs_obj = []
for hypo in list_0trkKs_hypo:
    list_0trkKs_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_0trkKs_obj

for i in range(len(list_0trkKs_obj)):
    list_0trkKs_obj[i].JXVertices               = "BPHY25OniaCandidates"
    list_0trkKs_obj[i].V0Vertices               = "KsCollection"
    list_0trkKs_obj[i].RefitV0                  = False
    list_0trkKs_obj[i].JXVtxHypoNames           = [list_0trkKs_jxHypo[i]]
    list_0trkKs_obj[i].CascadeVertexCollections = ["BPHY25_"+list_0trkKs_hypo[i]+"_CascadeVtx1","BPHY25_"+list_0trkKs_hypo[i]+"_CascadeVtx2","BPHY25_"+list_0trkKs_hypo[i]+"_CascadeVtx3"]
    list_0trkKs_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_0trkKs_obj[i].V0Hypothesis             = "Ks"
    list_0trkKs_obj[i].MassCutGamma             = 100.
    list_0trkKs_obj[i].Chi2CutGamma             = 5.
    list_0trkKs_obj[i].LxyV0Cut                 = 25.
    list_0trkKs_obj[i].HypothesisName           = list_0trkKs_hypo[i]
    list_0trkKs_obj[i].NumberOfJXDaughters      = 2
    list_0trkKs_obj[i].JXDaug1MassHypo          = Mumass
    list_0trkKs_obj[i].JXDaug2MassHypo          = Mumass
    list_0trkKs_obj[i].NumberOfDisVDaughters    = 2
    list_0trkKs_obj[i].JXMass                   = list_0trkKs_jxMass[i]
    list_0trkKs_obj[i].V0Mass                   = Ks0mass
    list_0trkKs_obj[i].ApplyJXMassConstraint    = True
    list_0trkKs_obj[i].ApplyV0MassConstraint    = True
    list_0trkKs_obj[i].Chi2CutV0                = 10.
    list_0trkKs_obj[i].Chi2Cut                  = 8.
    list_0trkKs_obj[i].Trackd0Cut               = 3.
    list_0trkKs_obj[i].MaxJXCandidates          = 15
    list_0trkKs_obj[i].MaxV0Candidates          = 15
    list_0trkKs_obj[i].RefitPV                  = True
    list_0trkKs_obj[i].MaxnPV                   = 20
    list_0trkKs_obj[i].RefPVContainerName       = "BPHY25_"+list_0trkKs_hypo[i]+"_RefPrimaryVertices"
    list_0trkKs_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_0trkKs_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_0trkKs_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_0trkKs_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###  0 trk + Lambda  ###
########################

list_0trkLd_hypo = ["PhiLd", "JpsiLd", "Psi0Ld", "UpsiLd"]
list_0trkLd_jxHypo = ["Phi", "Jpsi", "Psi", "Upsi"]
list_0trkLd_jxMass = [Phimass, Jpsimass, Psi2Smass, Upsimass]

list_0trkLd_obj = []
for hypo in list_0trkLd_hypo:
    list_0trkLd_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_0trkLd_obj

for i in range(len(list_0trkLd_obj)):
    list_0trkLd_obj[i].JXVertices               = "BPHY25OniaCandidates"
    list_0trkLd_obj[i].V0Vertices               = "LambdaCollection"
    list_0trkLd_obj[i].RefitV0                  = False
    list_0trkLd_obj[i].JXVtxHypoNames           = [list_0trkLd_jxHypo[i]]
    list_0trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_0trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_0trkLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_0trkLd_hypo[i]+"_CascadeVtx3"]
    list_0trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_0trkLd_obj[i].V0Hypothesis             = "Lambda"
    list_0trkLd_obj[i].MassCutGamma             = 10.
    list_0trkLd_obj[i].Chi2CutGamma             = 5.
    list_0trkLd_obj[i].LxyV0Cut                 = 25.
    list_0trkLd_obj[i].HypothesisName           = list_0trkLd_hypo[i]
    list_0trkLd_obj[i].NumberOfJXDaughters      = 2
    list_0trkLd_obj[i].JXDaug1MassHypo          = Mumass
    list_0trkLd_obj[i].JXDaug2MassHypo          = Mumass
    list_0trkLd_obj[i].NumberOfDisVDaughters    = 2
    list_0trkLd_obj[i].JXMass                   = list_0trkLd_jxMass[i]
    list_0trkLd_obj[i].V0Mass                   = Lambdamass
    list_0trkLd_obj[i].ApplyJXMassConstraint    = True
    list_0trkLd_obj[i].ApplyV0MassConstraint    = True
    list_0trkLd_obj[i].Chi2CutV0                = 10.
    list_0trkLd_obj[i].Chi2Cut                  = 8.
    list_0trkLd_obj[i].Trackd0Cut               = 3.
    list_0trkLd_obj[i].MaxJXCandidates          = 15
    list_0trkLd_obj[i].MaxV0Candidates          = 15
    list_0trkLd_obj[i].RefitPV                  = True
    list_0trkLd_obj[i].MaxnPV                   = 20
    list_0trkLd_obj[i].RefPVContainerName       = "BPHY25_"+list_0trkLd_hypo[i]+"_RefPrimaryVertices"
    list_0trkLd_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_0trkLd_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_0trkLd_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_0trkLd_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###    0 trk + Xi    ###
########################

list_0trkXi_hypo = ["PhiXi", "JpsiXi", "Psi0Xi", "UpsiXi"]
list_0trkXi_jxHypo = ["Phi", "Jpsi", "Psi", "Upsi"]
list_0trkXi_jxMass = [Phimass, Jpsimass, Psi2Smass, Upsimass]

list_0trkXi_obj = []
for hypo in list_0trkXi_hypo:
    list_0trkXi_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_0trkXi_obj

for i in range(len(list_0trkXi_obj)):
    list_0trkXi_obj[i].JXVertices               = "BPHY25OniaCandidates"
    list_0trkXi_obj[i].V0Vertices               = "LambdaCollection"
    list_0trkXi_obj[i].RefitV0                  = False
    list_0trkXi_obj[i].DisplacedVertices        = "XiCollection"
    list_0trkXi_obj[i].JXVtxHypoNames           = [list_0trkXi_jxHypo[i]]
    list_0trkXi_obj[i].CascadeVertexCollections = ["BPHY25_"+list_0trkXi_hypo[i]+"_CascadeVtx1","BPHY25_"+list_0trkXi_hypo[i]+"_CascadeVtx2_sub","BPHY25_"+list_0trkXi_hypo[i]+"_CascadeVtx2","BPHY25_"+list_0trkXi_hypo[i]+"_CascadeVtx3"]
    list_0trkXi_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_0trkXi_obj[i].V0Hypothesis             = "Lambda"
    list_0trkXi_obj[i].MassCutGamma             = 10.
    list_0trkXi_obj[i].Chi2CutGamma             = 5.
    list_0trkXi_obj[i].LxyV0Cut                 = 25.
    list_0trkXi_obj[i].LxyDisVtxCut             = 25.
    list_0trkXi_obj[i].HypothesisName           = list_0trkXi_hypo[i]
    list_0trkXi_obj[i].NumberOfJXDaughters      = 2
    list_0trkXi_obj[i].JXDaug1MassHypo          = Mumass
    list_0trkXi_obj[i].JXDaug2MassHypo          = Mumass
    list_0trkXi_obj[i].NumberOfDisVDaughters    = 3
    list_0trkXi_obj[i].DisVDaug3MassHypo        = Pimass
    list_0trkXi_obj[i].JXMass                   = list_0trkXi_jxMass[i]
    list_0trkXi_obj[i].V0Mass                   = Lambdamass
    list_0trkXi_obj[i].DisVtxMass               = Ximass
    list_0trkXi_obj[i].ApplyJXMassConstraint    = True
    list_0trkXi_obj[i].ApplyV0MassConstraint    = True
    list_0trkXi_obj[i].ApplyDisVMassConstraint  = True
    list_0trkXi_obj[i].Chi2CutV0                = 10.
    list_0trkXi_obj[i].Chi2Cut                  = 8.
    list_0trkXi_obj[i].Trackd0Cut               = 3.
    list_0trkXi_obj[i].MaxJXCandidates          = 15
    list_0trkXi_obj[i].MaxV0Candidates          = 15
    list_0trkXi_obj[i].MaxDisVCandidates        = 15
    list_0trkXi_obj[i].RefitPV                  = True
    list_0trkXi_obj[i].MaxnPV                   = 20
    list_0trkXi_obj[i].RefPVContainerName       = "BPHY25_"+list_0trkXi_hypo[i]+"_RefPrimaryVertices"
    list_0trkXi_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_0trkXi_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_0trkXi_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_0trkXi_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector

########################
###   0 trk + Omg    ###
########################

list_0trkOmg_hypo = ["PhiOmg", "JpsiOmg", "Psi0Omg", "UpsiOmg"]
list_0trkOmg_jxHypo = ["Phi", "Jpsi", "Psi", "Upsi"]
list_0trkOmg_jxMass = [Phimass, Jpsimass, Psi2Smass, Upsimass]

list_0trkOmg_obj = []
for hypo in list_0trkOmg_hypo:
    list_0trkOmg_obj.append( DerivationFramework__JpsiXPlusDisplaced("BPHY25_"+hypo) )

ToolSvc += list_0trkOmg_obj

for i in range(len(list_0trkOmg_obj)):
    list_0trkOmg_obj[i].JXVertices               = "BPHY25OniaCandidates"
    list_0trkOmg_obj[i].V0Vertices               = "LambdaCollection"
    list_0trkOmg_obj[i].RefitV0                  = False
    list_0trkOmg_obj[i].DisplacedVertices        = "OmegaCollection"
    list_0trkOmg_obj[i].JXVtxHypoNames           = [list_0trkOmg_jxHypo[i]]
    list_0trkOmg_obj[i].CascadeVertexCollections = ["BPHY25_"+list_0trkOmg_hypo[i]+"_CascadeVtx1","BPHY25_"+list_0trkOmg_hypo[i]+"_CascadeVtx2_sub","BPHY25_"+list_0trkOmg_hypo[i]+"_CascadeVtx2","BPHY25_"+list_0trkOmg_hypo[i]+"_CascadeVtx3"]
    list_0trkOmg_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_0trkOmg_obj[i].V0Hypothesis             = "Lambda"
    list_0trkOmg_obj[i].MassCutGamma             = 10.
    list_0trkOmg_obj[i].Chi2CutGamma             = 5.
    list_0trkOmg_obj[i].LxyV0Cut                 = 25.
    list_0trkOmg_obj[i].LxyDisVtxCut             = 25.
    list_0trkOmg_obj[i].HypothesisName           = list_0trkOmg_hypo[i]
    list_0trkOmg_obj[i].NumberOfJXDaughters      = 2
    list_0trkOmg_obj[i].JXDaug1MassHypo          = Mumass
    list_0trkOmg_obj[i].JXDaug2MassHypo          = Mumass
    list_0trkOmg_obj[i].NumberOfDisVDaughters    = 3
    list_0trkOmg_obj[i].DisVDaug3MassHypo        = Kmass
    list_0trkOmg_obj[i].JXMass                   = list_0trkOmg_jxMass[i]
    list_0trkOmg_obj[i].V0Mass                   = Lambdamass
    list_0trkOmg_obj[i].DisVtxMass               = Omegamass
    list_0trkOmg_obj[i].ApplyJXMassConstraint    = True
    list_0trkOmg_obj[i].ApplyV0MassConstraint    = True
    list_0trkOmg_obj[i].ApplyDisVMassConstraint  = True
    list_0trkOmg_obj[i].Chi2CutV0                = 10.
    list_0trkOmg_obj[i].Chi2Cut                  = 8.
    list_0trkOmg_obj[i].Trackd0Cut               = 3.
    list_0trkOmg_obj[i].MaxJXCandidates          = 15
    list_0trkOmg_obj[i].MaxV0Candidates          = 15
    list_0trkOmg_obj[i].MaxDisVCandidates        = 15
    list_0trkOmg_obj[i].RefitPV                  = True
    list_0trkOmg_obj[i].MaxnPV                   = 20
    list_0trkOmg_obj[i].RefPVContainerName       = "BPHY25_"+list_0trkOmg_hypo[i]+"_RefPrimaryVertices"
    list_0trkOmg_obj[i].TrkVertexFitterTool      = BPHY25VertexFit
    list_0trkOmg_obj[i].V0VertexFitterTool       = BPHY25_V0FinderTools.BPhysV0Fitter
    list_0trkOmg_obj[i].GammaFitterTool          = BPHY25_V0FinderTools.BPhysGammaFitter
    list_0trkOmg_obj[i].TrackSelectorTool        = BPHY25_V0FinderTools.InDetV0VxTrackSelector


list_all_obj = list_2trkKs_obj + list_2trkLd_obj + list_2trkXi_obj + list_2trkOmg_obj + list_1trkKs_obj + list_1trkLd_obj + list_1trkXi_obj + list_1trkOmg_obj + list_0trkKs_obj + list_0trkLd_obj + list_0trkXi_obj + list_0trkOmg_obj

CascadeCollections = []
RefPVContainers = []
RefPVAuxContainers = []
TheExpression = "("

for obj in list_all_obj:
    CascadeCollections += obj.CascadeVertexCollections
    RefPVContainers += ["xAOD::VertexContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVertices"]
    RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
    TheExpression += "count(BPHY25_" + obj.HypothesisName + "_CascadeVtx3.passed_" + obj.HypothesisName + ")"
    if list_all_obj.index(obj) != len(list_all_obj)-1:
        TheExpression += "+"

TheExpression += ") > 0"

#--------------------------------------------------------------------
## 7/ select the event. We only want to keep events that contain certain vertices which passed certain selection.
##    This is specified by the "SelectionExpression" property, which contains the expression in the following format:
##
##       "ContainerName.passed_HypoName > count"
##
##    where "ContainerName" is output container from some Reco_* tool, "HypoName" is the hypothesis name setup in some "Select_*"
##    tool and "count" is the number of candidates passing the selection you want to keep. 

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY25_SelectEvent = DerivationFramework__xAODStringSkimmingTool(name = "BPHY25_SelectEvent", expression = TheExpression)

ToolSvc += BPHY25_SelectEvent

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================
## IMPORTANT bit. Don't forget to pass the tools to the DerivationKernel! If you don't do that, they will not be 
## be executed!

# The name of the kernel (BPHY25Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
augmentation_tools = [BPHY25_Reco_mumu, BPHY25FourTrackReco_PsiX3872, BPHY25FourTrackReco_Bs0, BPHY25FourTrackReco_B0, BPHY25ThreeTrackReco_Zc3900, BPHY25ThreeTrackReco_Bpm, BPHY25ThreeTrackReco_DpmDs, BPHY25Rev_Psi, BPHY25Rev_X3872, BPHY25Rev_Bs0, BPHY25Rev_B0Kpi, BPHY25Rev_B0piK, BPHY25Rev_Zc3900, BPHY25Rev_Bpm, BPHY25Rev_Ds, BPHY25Rev_Dpm, BPHY25Select_Phi, BPHY25Select_Jpsi, BPHY25Select_Psi, BPHY25Select_Upsi, BPHY25_RecoV0Finder] + list_all_obj

DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel(
    "BPHY25Kernel",
    AugmentationTools = augmentation_tools,
    SkimmingTools     = [BPHY25_SelectEvent]
)

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_BPHY25Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_BPHY25Stream )
BPHY25Stream = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY25Stream.AcceptAlgs(["BPHY25Kernel"])
# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="BPHY25ThinningSvc", outStreams=[evtStream] )

#====================================================================
# Slimming 
#====================================================================

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY25SlimmingHelper = SlimmingHelper("BPHY25SlimmingHelper")
BPHY25_AllVariables = []
BPHY25_StaticContent = []

# Needed for trigger objects
BPHY25SlimmingHelper.IncludeMuonTriggerContent = True
BPHY25SlimmingHelper.IncludeBPhysTriggerContent = True

## primary vertices
BPHY25_AllVariables += ["PrimaryVertices"]
#BPHY25_StaticContent += RefPVContainers
#BPHY25_StaticContent += RefPVAuxContainers

## ID track particles
BPHY25_AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles 
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are stored in InDetTrackParticles collection)
BPHY25_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

## muon container
BPHY25_AllVariables += ["Muons", "MuonSegments"]

## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
for cascade in CascadeCollections:
    BPHY25_StaticContent += ["xAOD::VertexContainer#%s" % cascade]
    BPHY25_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascade]

# Truth information for MC only
if isSimulation:
    BPHY25_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

BPHY25SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
BPHY25SlimmingHelper.AllVariables = BPHY25_AllVariables
BPHY25SlimmingHelper.StaticContent = BPHY25_StaticContent
BPHY25SlimmingHelper.AppendContentToStream(BPHY25Stream)
