#====================================================================
# BPHY23.py
# Contact: xin.chen@cern.ch
# In the following, Meson (Onium) refers to charged (neutral) hadrons
#====================================================================

# Set up common services and job object. 
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkHasTruth
isSimulation = DerivationFrameworkHasTruth

print isSimulation

#====================================================================
# AUGMENTATION TOOLS 
#====================================================================
## 1/ setup vertexing tools and services
include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY23_VertexTools = BPHYVertexTools("BPHY23")

# mass bounds and constants used in the following
X_hi = 141000.0

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
Upsi2S_lo = 9550.0
Upsi2S_hi = 10500.0
Ds_lo = 1660.0
Ds_hi = 2230.0

Mumass = 105.658
Pimass = 139.570
Kmass = 493.677
Kstarmass = 895.55
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

#--------------------------------------------------------------------
## 2/ Setup the vertex fitter tools (e.g. JpsiFinder, JpsiPlus1Track, etc).
##    These are general tools independent of DerivationFramework that do the 
##    actual vertex fitting and some pre-selection.

# invMass range covers phi, J/psi, psi(2S), Upsi(1S) and Upsi(2S)
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY23JpsiFinder = Analysis__JpsiFinder(
    name                        = "BPHY23JpsiFinder",
    OutputLevel                 = INFO,
    muAndMu                     = True,
    muAndTrack                  = False,
    TrackAndTrack               = False,
    assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt            = 2400.,
    invMassLower                = Phi_lo,
    invMassUpper                = Upsi2S_hi,
    Chi2Cut                     = 10.,
    oppChargesOnly	        = True,
    atLeastOneComb              = True,
    useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
    muonCollectionKey           = "Muons",
    TrackParticleCollection     = "InDetTrackParticles",
    V0VertexFitterTool          = BPHY23_VertexTools.TrkV0Fitter, # V0 vertex fitter
    useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool         = BPHY23_VertexTools.TrkVKalVrtFitter, # VKalVrt vertex fitter
    TrackSelectorTool           = BPHY23_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool  = BPHY23_VertexTools.InDetConversionHelper,
    VertexPointEstimator        = BPHY23_VertexTools.VtxPointEstimator,
    useMCPCuts                  = False )
ToolSvc += BPHY23JpsiFinder

#--------------------------------------------------------------------
## 3/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic 
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool if the JpsiFinder mass window is wide enough.

# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/src/Reco_mumu.cxx
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY23_Reco_mumu = DerivationFramework__Reco_mumu(
    name                   = "BPHY23_Reco_mumu",
    JpsiFinder             = BPHY23JpsiFinder,
    OutputVtxContainerName = "BPHY23OniaCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    DoVertexType           = 1)
ToolSvc += BPHY23_Reco_mumu

## 4/ setup a new vertexing tool (necessary due to use of mass constraint) 
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BPHY23VertexFit = Trk__TrkVKalVrtFitter(
    name                = "BPHY23VertexFit",
    Extrapolator        = BPHY23_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint  = False,  # use Perigee strategy
    MakeExtendedVertex  = True)
ToolSvc += BPHY23VertexFit

## 5/ setup the Jpsi+2 track finder
# https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/JpsiUpsilonTools/src/JpsiPlus2Tracks.cxx
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus2Tracks

# X(3872), Psi(2S) -> J/psi + pi pi
BPHY23PsiX3872_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY23PsiX3872_Jpsi2Trk",
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
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY23VertexFit,
    TrackSelectorTool		        = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY23PsiX3872_Jpsi2Trk

# Bs0 -> J/psi + K K
BPHY23Bs0_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY23Bs0_Jpsi2Trk",
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
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY23VertexFit,
    TrackSelectorTool		        = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY23Bs0_Jpsi2Trk

# B0 -> J/psi + K pi
BPHY23B0_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY23B0_Jpsi2Trk",
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
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY23VertexFit,
    TrackSelectorTool		        = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY23B0_Jpsi2Trk

# Upsi2S -> Upsi pi pi
BPHY23Upsi2S_Jpsi2Trk = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY23Upsi2S_Jpsi2Trk",
    kaonkaonHypothesis		        = False,
    pionpionHypothesis                  = True,
    kaonpionHypothesis                  = False,
    kaonprotonHypothesis                = False,
    trkThresholdPt			= 380.,
    trkMaxEta		 	        = 2.6,
    oppChargesOnly                      = False,
    JpsiMassLower                       = Upsi_lo,
    JpsiMassUpper                       = Upsi_hi,
    TrkQuadrupletMassLower              = Upsi2S_lo,
    TrkQuadrupletMassUpper              = Upsi2S_hi,
    Chi2Cut                             = 10.,
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY23VertexFit,
    TrackSelectorTool		        = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY23Upsi2S_Jpsi2Trk

from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus1Track

# Zc(3900)+ -> J/psi pi
BPHY23Zc3900_Jpsi1Trk = Analysis__JpsiPlus1Track(
    name                                = "BPHY23Zc3900_Jpsi1Trk",
    pionHypothesis                      = True,
    kaonHypothesis                      = False,
    trkThresholdPt                      = 380.,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkTrippletMassLower                = Zc_lo,
    TrkTrippletMassUpper                = Zc_hi,
    Chi2Cut                             = 10.0,
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY23VertexFit,
    TrackSelectorTool                   = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY23Zc3900_Jpsi1Trk

# B+ -> J/psi K
BPHY23Bpm_Jpsi1Trk = Analysis__JpsiPlus1Track(
    name                                = "BPHY23Bpm_Jpsi1Trk",
    pionHypothesis                      = False,
    kaonHypothesis                      = True,
    trkThresholdPt                      = 380.,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkTrippletMassLower                = B_lo,
    TrkTrippletMassUpper                = B_hi,
    Chi2Cut                             = 10.0,
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY23VertexFit,
    TrackSelectorTool                   = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY23Bpm_Jpsi1Trk

# D+, Ds+ -> phi pi
BPHY23DpmDs_Jpsi1Trk = Analysis__JpsiPlus1Track(
    name                                = "BPHY23DpmDs_Jpsi1Trk",
    pionHypothesis                      = True,
    kaonHypothesis                      = False,
    trkThresholdPt                      = 380.0,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = Phi_lo,
    JpsiMassUpper                       = Phi_hi,
    TrkTrippletMassLower                = Ds_lo,
    TrkTrippletMassUpper                = Ds_hi,
    Chi2Cut                             = 10.0,
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY23VertexFit,
    TrackSelectorTool                   = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY23DpmDs_Jpsi1Trk

## 6/ setup the combined augmentation/skimming tool
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrkTrk
BPHY23FourTrackReco_PsiX3872 = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY23FourTrackReco_PsiX3872",
    Jpsi2PlusTrackName       = BPHY23PsiX3872_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY23FourTrack_PsiX3872",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23FourTrackReco_PsiX3872

BPHY23FourTrackReco_Bs0 = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY23FourTrackReco_Bs0",
    Jpsi2PlusTrackName       = BPHY23Bs0_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY23FourTrack_Bs0",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23FourTrackReco_Bs0

BPHY23FourTrackReco_B0 = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY23FourTrackReco_B0",
    Jpsi2PlusTrackName       = BPHY23B0_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY23FourTrack_B0",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23FourTrackReco_B0

BPHY23FourTrackReco_Upsi2S = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY23FourTrackReco_Upsi2S",
    Jpsi2PlusTrackName       = BPHY23Upsi2S_Jpsi2Trk,
    OutputVtxContainerName   = "BPHY23FourTrack_Upsi2S",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23FourTrackReco_Upsi2S

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrk
BPHY23ThreeTrackReco_Zc3900 = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY23ThreeTrackReco_Zc3900",
    Jpsi1PlusTrackName       = BPHY23Zc3900_Jpsi1Trk,
    OutputVtxContainerName   = "BPHY23ThreeTrack_Zc3900",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23ThreeTrackReco_Zc3900

BPHY23ThreeTrackReco_Bpm = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY23ThreeTrackReco_Bpm",
    Jpsi1PlusTrackName       = BPHY23Bpm_Jpsi1Trk,
    OutputVtxContainerName   = "BPHY23ThreeTrack_Bpm",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23ThreeTrackReco_Bpm

BPHY23ThreeTrackReco_DpmDs = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY23ThreeTrackReco_DpmDs",
    Jpsi1PlusTrackName       = BPHY23DpmDs_Jpsi1Trk,
    OutputVtxContainerName   = "BPHY23ThreeTrack_DpmDs",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False,
    DoVertexType             = 0)
ToolSvc += BPHY23ThreeTrackReco_DpmDs

# revertex with mass constraints to reduce combinatorics
# Psi(2S) -> J/psi pi pi
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__ReVertex
BPHY23Rev_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Psi",
    InputVtxContainerName      = "BPHY23FourTrack_PsiX3872",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Psi")
ToolSvc += BPHY23Rev_Psi

# X(3872) -> J/psi pi pi
BPHY23Rev_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_X3872",
    InputVtxContainerName      = "BPHY23FourTrack_PsiX3872",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_X3872")
ToolSvc += BPHY23Rev_X3872

# Bs0 -> J/psi K K
BPHY23Rev_Bs0 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Bs0",
    InputVtxContainerName      = "BPHY23FourTrack_Bs0",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Bs0")
ToolSvc += BPHY23Rev_Bs0

# B0 -> J/psi K pi
BPHY23Rev_B0Kpi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_B0Kpi",
    InputVtxContainerName      = "BPHY23FourTrack_B0",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_B0Kpi")
ToolSvc += BPHY23Rev_B0Kpi

# B0 -> J/psi pi K
BPHY23Rev_B0piK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_B0piK",
    InputVtxContainerName      = "BPHY23FourTrack_B0",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Kmass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_B0piK")
ToolSvc += BPHY23Rev_B0piK

# Upsi2S -> Upsi1S pi pi
BPHY23Rev_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Upsi2S",
    InputVtxContainerName      = "BPHY23FourTrack_Upsi2S",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Upsi2S")
ToolSvc += BPHY23Rev_Upsi2S

# Zc3900 -> J/psi pi
BPHY23Rev_Zc3900 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Zc3900",
    InputVtxContainerName      = "BPHY23ThreeTrack_Zc3900",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 15.,
    BMassLower                 = Zc_lo,
    BMassUpper                 = Zc_hi,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Zc3900")
ToolSvc += BPHY23Rev_Zc3900

# Bpm -> J/psi K
BPHY23Rev_Bpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Bpm",
    InputVtxContainerName      = "BPHY23ThreeTrack_Bpm",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Bpm")
ToolSvc += BPHY23Rev_Bpm

# Ds -> phi pi
BPHY23Rev_Ds = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Ds",
    InputVtxContainerName      = "BPHY23ThreeTrack_DpmDs",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Ds")
ToolSvc += BPHY23Rev_Ds

# Dpm -> phi pi
BPHY23Rev_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev_Dpm",
    InputVtxContainerName      = "BPHY23ThreeTrack_DpmDs",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = False,
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 15.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Revtx_Dpm")
ToolSvc += BPHY23Rev_Dpm


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu

BPHY23Select_Phi               = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Phi",
    HypothesisName             = "Phi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Phi_lo,
    MassMax                    = Phi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Phi

BPHY23Select_Jpsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Jpsi",
    HypothesisName             = "Jpsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Jpsi_lo,
    MassMax                    = Jpsi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Jpsi

BPHY23Select_Psi               = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Psi",
    HypothesisName             = "Psi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Psi_lo,
    MassMax                    = Psi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Psi

BPHY23Select_Upsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Upsi",
    HypothesisName             = "Upsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Upsi_lo,
    MassMax                    = Upsi_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Upsi

BPHY23Select_Upsi2S            = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Upsi2S",
    HypothesisName             = "Upsi2S",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Upsi2S_lo,
    MassMax                    = Upsi2S_hi,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Upsi2S

########################
###  2 trks + 0 trk  ###
########################

list_2trk0trk_hypo = ["Psi2Phi0", "Psi2Jpsi0", "Psi2Psi0", "Psi2Upsi0", "Psi2Upsi2S0",
                      "X3872Phi0", "X3872Jpsi0", "X3872Psi0", "X3872Upsi0", "X3872Upsi2S0",
                      "Bs2KPhi0", "Bs2KJpsi0", "Bs2KPsi0", "Bs2KUpsi0", "Bs2KUpsi2S0",
                      "B0KpiPhi0", "B0KpiJpsi0", "B0KpiPsi0", "B0KpiUpsi0", "B0KpiUpsi2S0",
                      "B0piKPhi0", "B0piKJpsi0", "B0piKPsi0", "B0piKUpsi0", "B0piKUpsi2S0",
                      "Upsi2S2Phi0", "Upsi2S2Jpsi0", "Upsi2S2Psi0", "Upsi2S2Upsi0", "Upsi2S2Upsi2S0"]
list_2trk0trk_psiInput = ["BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi",
                          "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872",
                          "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0",
                          "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi",
                          "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK",
                          "BPHY23Revtx_Upsi2S", "BPHY23Revtx_Upsi2S", "BPHY23Revtx_Upsi2S", "BPHY23Revtx_Upsi2S", "BPHY23Revtx_Upsi2S"]
list_2trk0trk_jpsiHypo = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"]
list_2trk0trk_jpsiMass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                          Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                          Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                          Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                          Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                          Upsimass, Upsimass, Upsimass, Upsimass, Upsimass]
list_2trk0trk_psiMass = [Psi2Smass, Psi2Smass, Psi2Smass, Psi2Smass, Psi2Smass,
                         X3872mass, X3872mass, X3872mass, X3872mass, X3872mass,
                         Bs0mass, Bs0mass, Bs0mass, Bs0mass, Bs0mass,
                         B0mass, B0mass, B0mass, B0mass, B0mass,
                         B0mass, B0mass, B0mass, B0mass, B0mass,
                         Upsi2Smass, Upsi2Smass, Upsi2Smass, Upsi2Smass, Upsi2Smass]
list_2trk0trk_dau3Mass = [Pimass, Pimass, Pimass, Pimass, Pimass,
                         Pimass, Pimass, Pimass, Pimass, Pimass,
                         Kmass, Kmass, Kmass, Kmass, Kmass,
                         Kmass, Kmass, Kmass, Kmass, Kmass,
                         Pimass, Pimass, Pimass, Pimass, Pimass,
                         Pimass, Pimass, Pimass, Pimass, Pimass]
list_2trk0trk_dau4Mass = [Pimass, Pimass, Pimass, Pimass, Pimass,
                         Pimass, Pimass, Pimass, Pimass, Pimass,
                         Kmass, Kmass, Kmass, Kmass, Kmass,
                         Pimass, Pimass, Pimass, Pimass, Pimass,
                         Kmass, Kmass, Kmass, Kmass, Kmass,
                         Pimass, Pimass, Pimass, Pimass, Pimass]
list_2trk0trk_jpsi2Mass = [Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass]

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiPlusPsiCascade

list_2trk0trk_obj = []
for hypo in list_2trk0trk_hypo:
    list_2trk0trk_obj.append( DerivationFramework__JpsiPlusPsiCascade("BPHY23"+hypo) )

ToolSvc += list_2trk0trk_obj

for i in range(len(list_2trk0trk_obj)):
    list_2trk0trk_obj[i].HypothesisName           = list_2trk0trk_hypo[i]
    list_2trk0trk_obj[i].TrkVertexFitterTool      = BPHY23VertexFit
    list_2trk0trk_obj[i].JpsiVertices             = "BPHY23OniaCandidates"
    list_2trk0trk_obj[i].JpsiVtxHypoNames         = [list_2trk0trk_jpsiHypo[i]]
    list_2trk0trk_obj[i].PsiVertices              = list_2trk0trk_psiInput[i]
    list_2trk0trk_obj[i].NumberOfPsiDaughters     = 4
    list_2trk0trk_obj[i].MassLowerCut             = 0.
    list_2trk0trk_obj[i].MassUpperCut             = X_hi
    list_2trk0trk_obj[i].Chi2Cut                  = 30.
    list_2trk0trk_obj[i].MaxPsiCandidates         = 15
    list_2trk0trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trk0trk_obj[i].RefPVContainerName       = "BPHY23"+list_2trk0trk_hypo[i]+"RefPrimaryVertices"
    list_2trk0trk_obj[i].CascadeVertexCollections = ["BPHY23"+list_2trk0trk_hypo[i]+"CascadeVtx1","BPHY23"+list_2trk0trk_hypo[i]+"CascadeVtx2","BPHY23"+list_2trk0trk_hypo[i]+"CascadeVtx3"]
    list_2trk0trk_obj[i].RefitPV                  = True
    list_2trk0trk_obj[i].Vtx1Daug3MassHypo        = list_2trk0trk_dau3Mass[i]
    list_2trk0trk_obj[i].Vtx1Daug4MassHypo        = list_2trk0trk_dau4Mass[i]
    list_2trk0trk_obj[i].ApplyJpsiMassConstraint  = True
    list_2trk0trk_obj[i].JpsiMass                 = list_2trk0trk_jpsiMass[i]
    list_2trk0trk_obj[i].ApplyPsiMassConstraint   = True
    list_2trk0trk_obj[i].PsiMass                  = list_2trk0trk_psiMass[i]
    if list_2trk0trk_jpsi2Mass[i] != Phimass:
        list_2trk0trk_obj[i].ApplyJpsi2MassConstraint = True
        list_2trk0trk_obj[i].Jpsi2Mass                = list_2trk0trk_jpsi2Mass[i]

#######################
###  1 trk + 0 trk  ###
#######################

list_1trk0trk_hypo = ["Zc3900Phi0", "Zc3900Jpsi0", "Zc3900Psi0", "Zc3900Upsi0", "Zc3900Upsi2S0",
                      "BpmPhi0", "BpmJpsi0", "BpmPsi0", "BpmUpsi0", "BpmUpsi2S0",
                      "DsPhi0", "DsJpsi0", "DsPsi0", "DsUpsi0", "DsUpsi2S0",
                      "DpmPhi0", "DpmJpsi0", "DpmPsi0", "DpmUpsi0", "DpmUpsi2S0"]
list_1trk0trk_psiInput = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900",
                          "BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm",
                          "BPHY23Revtx_Ds", "BPHY23Revtx_Ds", "BPHY23Revtx_Ds", "BPHY23Revtx_Ds", "BPHY23Revtx_Ds",
                          "BPHY23Revtx_Dpm", "BPHY23Revtx_Dpm", "BPHY23Revtx_Dpm", "BPHY23Revtx_Dpm", "BPHY23Revtx_Dpm"]
list_1trk0trk_jpsiHypo = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S",
                          "Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"]
list_1trk0trk_jpsiMass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                          Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                          Phimass, Phimass, Phimass, Phimass, Phimass,
                          Phimass, Phimass, Phimass, Phimass, Phimass]
list_1trk0trk_psiMass = [Zcmass, Zcmass, Zcmass, Zcmass, Zcmass,
                         Bpmmass, Bpmmass, Bpmmass, Bpmmass, Bpmmass,
                         Dspmmass, Dspmmass, Dspmmass, Dspmmass, Dspmmass,
                         Dpmmass, Dpmmass, Dpmmass, Dpmmass, Dpmmass]
list_1trk0trk_dau3Mass = [Pimass, Pimass, Pimass, Pimass, Pimass,
                         Kmass, Kmass, Kmass, Kmass, Kmass,
                         Pimass, Pimass, Pimass, Pimass, Pimass,
                         Pimass, Pimass, Pimass, Pimass, Pimass]
list_1trk0trk_jpsi2Mass = [Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass,
                           Phimass, Jpsimass, Psi2Smass, Upsimass, Upsi2Smass]

list_1trk0trk_obj = []
for hypo in list_1trk0trk_hypo:
    list_1trk0trk_obj.append( DerivationFramework__JpsiPlusPsiCascade("BPHY23"+hypo) )

ToolSvc += list_1trk0trk_obj

for i in range(len(list_1trk0trk_obj)):
    list_1trk0trk_obj[i].HypothesisName           = list_1trk0trk_hypo[i]
    list_1trk0trk_obj[i].TrkVertexFitterTool      = BPHY23VertexFit
    list_1trk0trk_obj[i].JpsiVertices             = "BPHY23OniaCandidates"
    list_1trk0trk_obj[i].JpsiVtxHypoNames         = [list_1trk0trk_jpsiHypo[i]]
    list_1trk0trk_obj[i].PsiVertices              = list_1trk0trk_psiInput[i]
    list_1trk0trk_obj[i].NumberOfPsiDaughters     = 3
    list_1trk0trk_obj[i].MassLowerCut             = 0.
    list_1trk0trk_obj[i].MassUpperCut             = X_hi
    list_1trk0trk_obj[i].Chi2Cut                  = 30.
    list_1trk0trk_obj[i].MaxPsiCandidates         = 15
    list_1trk0trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_1trk0trk_obj[i].RefPVContainerName       = "BPHY23"+list_1trk0trk_hypo[i]+"RefPrimaryVertices"
    list_1trk0trk_obj[i].CascadeVertexCollections = ["BPHY23"+list_1trk0trk_hypo[i]+"CascadeVtx1","BPHY23"+list_1trk0trk_hypo[i]+"CascadeVtx2","BPHY23"+list_1trk0trk_hypo[i]+"CascadeVtx3"]
    list_1trk0trk_obj[i].RefitPV                  = True
    list_1trk0trk_obj[i].Vtx1Daug3MassHypo        = list_1trk0trk_dau3Mass[i]
    if list_1trk0trk_jpsiMass[i] != Phimass:
        list_1trk0trk_obj[i].ApplyJpsiMassConstraint  = True
        list_1trk0trk_obj[i].JpsiMass                 = list_1trk0trk_jpsiMass[i]
    if list_1trk0trk_psiMass[i] != Zcmass:
        list_1trk0trk_obj[i].ApplyPsiMassConstraint   = True
        list_1trk0trk_obj[i].PsiMass                  = list_1trk0trk_psiMass[i]
    if list_1trk0trk_jpsi2Mass[i] != Phimass:
        list_1trk0trk_obj[i].ApplyJpsi2MassConstraint = True
        list_1trk0trk_obj[i].Jpsi2Mass                = list_1trk0trk_jpsi2Mass[i]

#######################
###  1 trk + 1 trk  ###
#######################

list_1trk1trk_hypo = ["Zc3900Zc3900", "Zc3900Bpm", "Zc3900Ds", "Zc3900Dpm",
                      "BpmBpm", "BpmDs", "BpmDpm",
                      "DsDs", "DsDpm",
                      "DpmDpm"]
list_1trk1trk_psi1Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900",
                           "BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm",
                           "BPHY23Revtx_Ds", "BPHY23Revtx_Ds",
                           "BPHY23Revtx_Dpm"]
list_1trk1trk_psi2Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Dpm"]
list_1trk1trk_jpsi1Mass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass,
                           Phimass, Phimass,
                           Phimass]
list_1trk1trk_psi1Mass = [Zcmass, Zcmass, Zcmass, Zcmass,
                          Bpmmass, Bpmmass, Bpmmass,
                          Dspmmass, Dspmmass,
                          Dpmmass]
list_1trk1trk_p1dau3Mass = [Pimass, Pimass, Pimass, Pimass,
                           Kmass, Kmass, Kmass,
                           Pimass, Pimass,
                           Pimass]
list_1trk1trk_jpsi2Mass = [Jpsimass, Jpsimass, Phimass, Phimass,
                           Jpsimass, Phimass, Phimass,
                           Phimass, Phimass,
                           Phimass]
list_1trk1trk_psi2Mass = [Zcmass, Bpmmass, Dspmmass, Dpmmass,
                          Bpmmass, Dspmmass, Dpmmass,
                          Dspmmass, Dpmmass,
                          Dpmmass]
list_1trk1trk_p2dau3Mass = [Pimass, Kmass, Pimass, Pimass,
                           Kmass, Pimass, Pimass,
                           Pimass, Pimass,
                           Pimass]

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__PsiPlusPsiCascade

list_1trk1trk_obj = []
for hypo in list_1trk1trk_hypo:
    list_1trk1trk_obj.append( DerivationFramework__PsiPlusPsiCascade("BPHY23"+hypo) )

ToolSvc += list_1trk1trk_obj

for i in range(len(list_1trk1trk_obj)):
    list_1trk1trk_obj[i].HypothesisName           = list_1trk1trk_hypo[i]
    list_1trk1trk_obj[i].TrkVertexFitterTool      = BPHY23VertexFit
    list_1trk1trk_obj[i].Psi1Vertices             = list_1trk1trk_psi1Input[i]
    list_1trk1trk_obj[i].Psi2Vertices             = list_1trk1trk_psi2Input[i]
    list_1trk1trk_obj[i].NumberOfPsi1Daughters    = 3
    list_1trk1trk_obj[i].NumberOfPsi2Daughters    = 3
    list_1trk1trk_obj[i].MassLowerCut             = 0.
    list_1trk1trk_obj[i].MassUpperCut             = X_hi
    list_1trk1trk_obj[i].Chi2CutPsi1              = 12.
    list_1trk1trk_obj[i].Chi2CutPsi2              = 12.
    list_1trk1trk_obj[i].Chi2Cut                  = 30.
    list_1trk1trk_obj[i].MaxPsi1Candidates        = 15
    list_1trk1trk_obj[i].MaxPsi2Candidates        = 15
    list_1trk1trk_obj[i].RemoveDuplicatePairs     = True
    list_1trk1trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_1trk1trk_obj[i].RefPVContainerName       = "BPHY23"+list_1trk1trk_hypo[i]+"RefPrimaryVertices"
    list_1trk1trk_obj[i].CascadeVertexCollections = ["BPHY23"+list_1trk1trk_hypo[i]+"CascadeVtx1","BPHY23"+list_1trk1trk_hypo[i]+"CascadeVtx2","BPHY23"+list_1trk1trk_hypo[i]+"CascadeVtx3"]
    list_1trk1trk_obj[i].RefitPV                  = True
    list_1trk1trk_obj[i].Vtx1Daug3MassHypo        = list_1trk1trk_p1dau3Mass[i]
    list_1trk1trk_obj[i].Vtx2Daug3MassHypo        = list_1trk1trk_p2dau3Mass[i]
    if list_1trk1trk_jpsi1Mass[i] != Phimass:
        list_1trk1trk_obj[i].ApplyJpsi1MassConstraint = True
        list_1trk1trk_obj[i].Jpsi1Mass                = list_1trk1trk_jpsi1Mass[i]
    if list_1trk1trk_psi1Mass[i] != Zcmass:
        list_1trk1trk_obj[i].ApplyPsi1MassConstraint  = True
        list_1trk1trk_obj[i].Psi1Mass                 = list_1trk1trk_psi1Mass[i]
    if list_1trk1trk_jpsi2Mass[i] != Phimass:
        list_1trk1trk_obj[i].ApplyJpsi2MassConstraint = True
        list_1trk1trk_obj[i].Jpsi2Mass                = list_1trk1trk_jpsi2Mass[i]
    if list_1trk1trk_psi2Mass[i] != Zcmass:
        list_1trk1trk_obj[i].ApplyPsi2MassConstraint  = True
        list_1trk1trk_obj[i].Psi2Mass                 = list_1trk1trk_psi2Mass[i]

########################
###  2 trks + 1 trk  ###
########################

list_2trk1trk_hypo = ["Psi2Zc3900", "Psi2Bpm", "Psi2Ds", "Psi2Dpm",
                      "X3872Zc3900", "X3872Bpm", "X3872Ds", "X3872Dpm",
                      "Bs2KZc3900", "Bs2KBpm", "Bs2KDs", "Bs2KDpm",
                      "B0KpiZc3900", "B0KpiBpm", "B0KpiDs", "B0KpiDpm",
                      "B0piKZc3900", "B0piKBpm", "B0piKDs", "B0piKDpm",
                      "Upsi2S2Zc3900", "Upsi2S2Bpm", "Upsi2S2Ds", "Upsi2S2Dpm"]
list_2trk1trk_psi1Input = ["BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi",
                           "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872",
                           "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0",
                           "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi",
                           "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK",
                           "BPHY23Revtx_Upsi2S", "BPHY23Revtx_Upsi2S", "BPHY23Revtx_Upsi2S", "BPHY23Revtx_Upsi2S"]
list_2trk1trk_psi2Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm",
                           "BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm", "BPHY23Revtx_Ds", "BPHY23Revtx_Dpm"]
list_2trk1trk_jpsi1Mass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Upsimass, Upsimass, Upsimass, Upsimass]
list_2trk1trk_psi1Mass = [Psi2Smass, Psi2Smass, Psi2Smass, Psi2Smass,
                          X3872mass, X3872mass, X3872mass, X3872mass,
                          Bs0mass, Bs0mass, Bs0mass, Bs0mass,
                          B0mass, B0mass, B0mass, B0mass,
                          B0mass, B0mass, B0mass, B0mass,
                          Upsi2Smass, Upsi2Smass, Upsi2Smass, Upsi2Smass]
list_2trk1trk_p1dau3Mass = [Pimass, Pimass, Pimass, Pimass,
                            Pimass, Pimass, Pimass, Pimass,
                            Kmass, Kmass, Kmass, Kmass,
                            Kmass, Kmass, Kmass, Kmass,
                            Pimass, Pimass, Pimass, Pimass,
                            Pimass, Pimass, Pimass, Pimass]
list_2trk1trk_p1dau4Mass = [Pimass, Pimass, Pimass, Pimass,
                            Pimass, Pimass, Pimass, Pimass,
                            Kmass, Kmass, Kmass, Kmass,
                            Pimass, Pimass, Pimass, Pimass,
                            Kmass, Kmass, Kmass, Kmass,
                            Pimass, Pimass, Pimass, Pimass]
list_2trk1trk_jpsi2Mass = [Jpsimass, Jpsimass, Phimass, Phimass,
                           Jpsimass, Jpsimass, Phimass, Phimass,
                           Jpsimass, Jpsimass, Phimass, Phimass,
                           Jpsimass, Jpsimass, Phimass, Phimass,
                           Jpsimass, Jpsimass, Phimass, Phimass,
                           Jpsimass, Jpsimass, Phimass, Phimass]
list_2trk1trk_psi2Mass = [Zcmass, Bpmmass, Dspmmass, Dpmmass,
                          Zcmass, Bpmmass, Dspmmass, Dpmmass,
                          Zcmass, Bpmmass, Dspmmass, Dpmmass,
                          Zcmass, Bpmmass, Dspmmass, Dpmmass,
                          Zcmass, Bpmmass, Dspmmass, Dpmmass,
                          Zcmass, Bpmmass, Dspmmass, Dpmmass]
list_2trk1trk_p2dau3Mass = [Pimass, Kmass, Pimass, Pimass,
                            Pimass, Kmass, Pimass, Pimass,
                            Pimass, Kmass, Pimass, Pimass,
                            Pimass, Kmass, Pimass, Pimass,
                            Pimass, Kmass, Pimass, Pimass,
                            Pimass, Kmass, Pimass, Pimass]

list_2trk1trk_obj = []
for hypo in list_2trk1trk_hypo:
    list_2trk1trk_obj.append( DerivationFramework__PsiPlusPsiCascade("BPHY23"+hypo) )

ToolSvc += list_2trk1trk_obj

for i in range(len(list_2trk1trk_obj)):
    list_2trk1trk_obj[i].HypothesisName           = list_2trk1trk_hypo[i]
    list_2trk1trk_obj[i].TrkVertexFitterTool      = BPHY23VertexFit
    list_2trk1trk_obj[i].Psi1Vertices             = list_2trk1trk_psi1Input[i]
    list_2trk1trk_obj[i].Psi2Vertices             = list_2trk1trk_psi2Input[i]
    list_2trk1trk_obj[i].NumberOfPsi1Daughters    = 4
    list_2trk1trk_obj[i].NumberOfPsi2Daughters    = 3
    list_2trk1trk_obj[i].MassLowerCut             = 0.
    list_2trk1trk_obj[i].MassUpperCut             = X_hi
    list_2trk1trk_obj[i].Chi2CutPsi1              = 10.
    list_2trk1trk_obj[i].Chi2CutPsi2              = 12.
    list_2trk1trk_obj[i].Chi2Cut                  = 30.
    list_2trk1trk_obj[i].MaxPsi1Candidates        = 15
    list_2trk1trk_obj[i].MaxPsi2Candidates        = 15
    list_2trk1trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trk1trk_obj[i].RefPVContainerName       = "BPHY23"+list_2trk1trk_hypo[i]+"RefPrimaryVertices"
    list_2trk1trk_obj[i].CascadeVertexCollections = ["BPHY23"+list_2trk1trk_hypo[i]+"CascadeVtx1","BPHY23"+list_2trk1trk_hypo[i]+"CascadeVtx2","BPHY23"+list_2trk1trk_hypo[i]+"CascadeVtx3"]
    list_2trk1trk_obj[i].RefitPV                  = True
    list_2trk1trk_obj[i].Vtx1Daug3MassHypo        = list_2trk1trk_p1dau3Mass[i]
    list_2trk1trk_obj[i].Vtx1Daug4MassHypo        = list_2trk1trk_p1dau4Mass[i]
    list_2trk1trk_obj[i].Vtx2Daug3MassHypo        = list_2trk1trk_p2dau3Mass[i]
    list_2trk1trk_obj[i].ApplyJpsi1MassConstraint = True
    list_2trk1trk_obj[i].Jpsi1Mass                = list_2trk1trk_jpsi1Mass[i]
    list_2trk1trk_obj[i].ApplyPsi1MassConstraint  = True
    list_2trk1trk_obj[i].Psi1Mass                 = list_2trk1trk_psi1Mass[i]
    if list_2trk1trk_jpsi2Mass[i] != Phimass:
        list_2trk1trk_obj[i].ApplyJpsi2MassConstraint = True
        list_2trk1trk_obj[i].Jpsi2Mass                = list_2trk1trk_jpsi2Mass[i]
    if list_2trk1trk_psi2Mass[i] != Zcmass:
        list_2trk1trk_obj[i].ApplyPsi2MassConstraint  = True
        list_2trk1trk_obj[i].Psi2Mass                 = list_2trk1trk_psi2Mass[i]

#########################
###  2 trks + 2 trks  ###
#########################

list_2trk2trk_hypo = ["Psi2Psi2", "Psi2X3872", "Psi2Bs2K", "Psi2B0Kpi", "Psi2B0piK", "Psi2Upsi2S2",
                      "X3872X3872", "X3872Bs2K", "X3872B0Kpi", "X3872B0piK", "X3872Upsi2S2",
                      "Bs2KBs2K", "Bs2KB0Kpi", "Bs2KB0piK", "Bs2KUpsi2S2",
                      "B0KpiB0Kpi", "B0KpiB0piK", "B0KpiUpsi2S2",
                      "B0piKB0piK", "B0piKUpsi2S2",
                      "Upsi2S2Upsi2S2"]
list_2trk2trk_psi1Input = ["BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi", "BPHY23Revtx_Psi",
                           "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872",
                           "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0",
                           "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi",
                           "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK",
                           "BPHY23Revtx_Upsi2S"]
list_2trk2trk_psi2Input = ["BPHY23Revtx_Psi", "BPHY23Revtx_X3872", "BPHY23Revtx_Bs0", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK", "BPHY23Revtx_Upsi2S",
                           "BPHY23Revtx_X3872", "BPHY23Revtx_Bs0", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK", "BPHY23Revtx_Upsi2S",
                           "BPHY23Revtx_Bs0", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK", "BPHY23Revtx_Upsi2S",
                           "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK", "BPHY23Revtx_Upsi2S",
                           "BPHY23Revtx_B0piK", "BPHY23Revtx_Upsi2S",
                           "BPHY23Revtx_Upsi2S"]
list_2trk2trk_jpsi1Mass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass, Jpsimass,
                           Jpsimass, Jpsimass,
                           Upsimass]
list_2trk2trk_psi1Mass = [Psi2Smass, Psi2Smass, Psi2Smass, Psi2Smass, Psi2Smass, Psi2Smass,
                          X3872mass, X3872mass, X3872mass, X3872mass, X3872mass,
                          Bs0mass, Bs0mass, Bs0mass, Bs0mass,
                          B0mass, B0mass, B0mass,
                          B0mass, B0mass,
                          Upsi2Smass]
list_2trk2trk_p1dau3Mass = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass,
                            Pimass, Pimass, Pimass, Pimass, Pimass,
                            Kmass, Kmass, Kmass, Kmass,
                            Kmass, Kmass, Kmass,
                            Pimass, Pimass,
                            Pimass]
list_2trk2trk_p1dau4Mass = [Pimass, Pimass, Pimass, Pimass, Pimass, Pimass,
                            Pimass, Pimass, Pimass, Pimass, Pimass,
                            Kmass, Kmass, Kmass, Kmass,
                            Pimass, Pimass, Pimass,
                            Kmass, Kmass,
                            Pimass]
list_2trk2trk_jpsi2Mass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass, Upsimass,
                           Jpsimass, Jpsimass, Jpsimass, Jpsimass, Upsimass,
                           Jpsimass, Jpsimass, Jpsimass, Upsimass,
                           Jpsimass, Jpsimass, Upsimass,
                           Jpsimass, Upsimass,
                           Upsimass]
list_2trk2trk_psi2Mass = [Psi2Smass, X3872mass, Bs0mass, B0mass, B0mass, Upsi2Smass,
                          X3872mass, Bs0mass, B0mass, B0mass, Upsi2Smass,
                          Bs0mass, B0mass, B0mass, Upsi2Smass,
                          B0mass, B0mass, Upsi2Smass,
                          B0mass, Upsi2Smass,
                          Upsi2Smass]
list_2trk2trk_p2dau3Mass = [Pimass, Pimass, Kmass, Kmass, Pimass, Pimass,
                            Pimass, Kmass, Kmass, Pimass, Pimass,
                            Kmass, Kmass, Pimass, Pimass,
                            Kmass, Pimass, Pimass,
                            Pimass, Pimass,
                            Pimass]
list_2trk2trk_p2dau4Mass = [Pimass, Pimass, Kmass, Pimass, Kmass,  Pimass,
                            Pimass, Kmass, Pimass, Kmass,  Pimass,
                            Kmass, Pimass, Kmass,  Pimass,
                            Pimass, Kmass, Pimass,
                            Kmass, Pimass,
                            Pimass]

list_2trk2trk_obj = []
for hypo in list_2trk2trk_hypo:
    list_2trk2trk_obj.append( DerivationFramework__PsiPlusPsiCascade("BPHY23"+hypo) )

ToolSvc += list_2trk2trk_obj

for i in range(len(list_2trk2trk_obj)):
    list_2trk2trk_obj[i].HypothesisName           = list_2trk2trk_hypo[i]
    list_2trk2trk_obj[i].TrkVertexFitterTool      = BPHY23VertexFit
    list_2trk2trk_obj[i].Psi1Vertices             = list_2trk2trk_psi1Input[i]
    list_2trk2trk_obj[i].Psi2Vertices             = list_2trk2trk_psi2Input[i]
    list_2trk2trk_obj[i].NumberOfPsi1Daughters    = 4
    list_2trk2trk_obj[i].NumberOfPsi2Daughters    = 4
    list_2trk2trk_obj[i].MassLowerCut             = 0.
    list_2trk2trk_obj[i].MassUpperCut             = X_hi
    list_2trk2trk_obj[i].Chi2CutPsi1              = 10.
    list_2trk2trk_obj[i].Chi2CutPsi2              = 10.
    list_2trk2trk_obj[i].Chi2Cut                  = 30.
    list_2trk2trk_obj[i].MaxPsi1Candidates        = 15
    list_2trk2trk_obj[i].MaxPsi2Candidates        = 15
    list_2trk2trk_obj[i].RemoveDuplicatePairs     = True
    list_2trk2trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2trk2trk_obj[i].RefPVContainerName       = "BPHY23"+list_2trk2trk_hypo[i]+"RefPrimaryVertices"
    list_2trk2trk_obj[i].CascadeVertexCollections = ["BPHY23"+list_2trk2trk_hypo[i]+"CascadeVtx1","BPHY23"+list_2trk2trk_hypo[i]+"CascadeVtx2","BPHY23"+list_2trk2trk_hypo[i]+"CascadeVtx3"]
    list_2trk2trk_obj[i].RefitPV                  = True
    list_2trk2trk_obj[i].Vtx1Daug3MassHypo        = list_2trk2trk_p1dau3Mass[i]
    list_2trk2trk_obj[i].Vtx1Daug4MassHypo        = list_2trk2trk_p1dau4Mass[i]
    list_2trk2trk_obj[i].Vtx2Daug3MassHypo        = list_2trk2trk_p2dau3Mass[i]
    list_2trk2trk_obj[i].Vtx2Daug4MassHypo        = list_2trk2trk_p2dau4Mass[i]
    list_2trk2trk_obj[i].ApplyJpsi1MassConstraint = True
    list_2trk2trk_obj[i].Jpsi1Mass                = list_2trk2trk_jpsi1Mass[i]
    list_2trk2trk_obj[i].ApplyPsi1MassConstraint  = True
    list_2trk2trk_obj[i].Psi1Mass                 = list_2trk2trk_psi1Mass[i]
    list_2trk2trk_obj[i].ApplyJpsi2MassConstraint = True
    list_2trk2trk_obj[i].Jpsi2Mass                = list_2trk2trk_jpsi2Mass[i]
    list_2trk2trk_obj[i].ApplyPsi2MassConstraint  = True
    list_2trk2trk_obj[i].Psi2Mass                 = list_2trk2trk_psi2Mass[i]


list_all_obj = list_2trk0trk_obj + list_1trk0trk_obj + list_1trk1trk_obj + list_2trk1trk_obj + list_2trk2trk_obj

CascadeCollections = []
RefPVContainers = []
RefPVAuxContainers = []
expression = "("

for obj in list_all_obj:
    CascadeCollections += obj.CascadeVertexCollections
    RefPVContainers += ["xAOD::VertexContainer#BPHY23" + obj.HypothesisName + "RefPrimaryVertices"]
    RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23" + obj.HypothesisName + "RefPrimaryVerticesAux."]
    expression += "count(BPHY23" + obj.HypothesisName + "CascadeVtx3.passed_" + obj.HypothesisName + ")"
    if list_all_obj.index(obj) != len(list_all_obj)-1:
        expression += "+"

expression += ") > 0"

#--------------------------------------------------------------------
## 7/ select the event. We only want to keep events that contain certain vertices which passed certain selection.
##    This is specified by the "SelectionExpression" property, which contains the expression in the following format:
##
##       "ContainerName.passed_HypoName > count"
##
##    where "ContainerName" is output container from some Reco_* tool, "HypoName" is the hypothesis name setup in some "Select_*"
##    tool and "count" is the number of candidates passing the selection you want to keep. 

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY23_SelectEvent = DerivationFramework__xAODStringSkimmingTool(name = "BPHY23_SelectEvent", expression = expression)

ToolSvc += BPHY23_SelectEvent

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================
## IMPORTANT bit. Don't forget to pass the tools to the DerivationKernel! If you don't do that, they will not be 
## be executed!

# The name of the kernel (BPHY23Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
augmentation_tools = [BPHY23_Reco_mumu, BPHY23FourTrackReco_PsiX3872, BPHY23FourTrackReco_Bs0, BPHY23FourTrackReco_B0, BPHY23FourTrackReco_Upsi2S, BPHY23ThreeTrackReco_Zc3900, BPHY23ThreeTrackReco_Bpm, BPHY23ThreeTrackReco_DpmDs, BPHY23Rev_Psi, BPHY23Rev_X3872, BPHY23Rev_Bs0, BPHY23Rev_B0Kpi, BPHY23Rev_B0piK, BPHY23Rev_Upsi2S, BPHY23Rev_Zc3900, BPHY23Rev_Bpm, BPHY23Rev_Ds, BPHY23Rev_Dpm, BPHY23Select_Phi, BPHY23Select_Jpsi, BPHY23Select_Psi, BPHY23Select_Upsi, BPHY23Select_Upsi2S] + list_all_obj

DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel(
    "BPHY23Kernel",
    AugmentationTools = augmentation_tools,
    SkimmingTools     = [BPHY23_SelectEvent]
)

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_BPHY23Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_BPHY23Stream )
BPHY23Stream = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY23Stream.AcceptAlgs(["BPHY23Kernel"])
# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="BPHY23ThinningSvc", outStreams=[evtStream] )

#====================================================================
# Slimming 
#====================================================================

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY23SlimmingHelper = SlimmingHelper("BPHY23SlimmingHelper")
BPHY23_AllVariables = []
BPHY23_StaticContent = []

# Needed for trigger objects
BPHY23SlimmingHelper.IncludeMuonTriggerContent = True
BPHY23SlimmingHelper.IncludeBPhysTriggerContent = True

## primary vertices
BPHY23_AllVariables += ["PrimaryVertices"]
BPHY23_StaticContent += RefPVContainers
BPHY23_StaticContent += RefPVAuxContainers

## ID track particles
BPHY23_AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles 
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are stored in InDetTrackParticles collection)
BPHY23_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

## muon container
BPHY23_AllVariables += ["Muons", "MuonSegments"]

## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
for cascade in CascadeCollections:
    BPHY23_StaticContent += ["xAOD::VertexContainer#%s" % cascade]
    BPHY23_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascade]

# Truth information for MC only
if isSimulation:
    BPHY23_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

BPHY23SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
BPHY23SlimmingHelper.AllVariables = BPHY23_AllVariables
BPHY23SlimmingHelper.StaticContent = BPHY23_StaticContent
BPHY23SlimmingHelper.AppendContentToStream(BPHY23Stream)
