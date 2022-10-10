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

# mass limits (loose and tight versions) and constants used in the following
Jpsi_lo = 2500.0
Jpsi_hi = 3600.0
Zc_lo = 3500.0
Zc_hi = 4300.0
Psi_lo = 3200.0
Psi_hi = 4300.0
B_lo = 4800.0
B_hi = 5750.0
Kstar_lo = 590.0
Kstar_hi = 1190.0
Bs0_lo = 4900.0
Bs0_hi = 5850.0
Phi_lo = 720.0
Phi_hi = 1320.0
Upsi_lo = 8850.0
Upsi_hi = 9950.0
Upsi2S_lo = 9500.0
Upsi2S_hi = 10600.0
Ds_lo = 1600.0
Ds_hi = 2300.0

Jpsi_lo2 = 2600.0
Jpsi_hi2 = 3500.0
Zc_lo2 = 3600.0
Zc_hi2 = 4200.0
Psi_lo2 = 3300.0
Psi_hi2 = 4200.0
B_lo2 = 4850.0
B_hi2 = 5700.0
Kstar_lo2 = 640.0
Kstar_hi2 = 1140.0
Bs0_lo2 = 4950.0
Bs0_hi2 = 5800.0
Phi_lo2 = 770.0
Phi_hi2 = 1270.0
Upsi_lo2 = 8900.0
Upsi_hi2 = 9900.0
Upsi2S_lo2 = 9550.0
Upsi2S_hi2 = 10550.0
Ds_lo2 = 1650.0
Ds_hi2 = 2250.0

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

# B0, X(3872), Psi(2S), Bs -> J/psi + trk trk
BPHY23JpsiPlus2Tracks = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY23JpsiPlus2Tracks",
    kaonkaonHypothesis		        = True,
    pionpionHypothesis                  = True,
    kaonpionHypothesis                  = True,
    kaonprotonHypothesis                = False,
    trkThresholdPt			= 380.,
    trkMaxEta		 	        = 2.6,
    oppChargesOnly                      = False,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkQuadrupletMassLower              = Psi_lo,
    TrkQuadrupletMassUpper              = Bs0_hi,
    Chi2Cut                             = 10.,
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi			= "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool		        = BPHY23VertexFit,
    TrackSelectorTool		        = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint		        = False)
ToolSvc += BPHY23JpsiPlus2Tracks

# Upsi2S -> Upsi pi pi
BPHY23UpsiPlus2Tracks = Analysis__JpsiPlus2Tracks(
    name                                = "BPHY23UpsiPlus2Tracks",
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
ToolSvc += BPHY23UpsiPlus2Tracks

from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus1Track

# Zc(3900)+, B+ -> J/psi trk
BPHY23JpsiPlus1Track = Analysis__JpsiPlus1Track(
    name                                = "BPHY23JpsiPlus1Track",
    pionHypothesis                      = True,
    kaonHypothesis                      = True,
    trkThresholdPt                      = 380.,
    trkMaxEta                           = 2.6,
    JpsiMassLower                       = Jpsi_lo,
    JpsiMassUpper                       = Jpsi_hi,
    TrkTrippletMassLower                = Zc_lo,
    TrkTrippletMassUpper                = B_hi,
    Chi2Cut                             = 10.0,
    JpsiContainerKey                    = "BPHY23OniaCandidates",
    TrackParticleCollection             = "InDetTrackParticles",
    MuonsUsedInJpsi                     = "Muons",
    ExcludeJpsiMuonsOnly                = True,
    TrkVertexFitterTool                 = BPHY23VertexFit,
    TrackSelectorTool                   = BPHY23_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint                   = False)
ToolSvc += BPHY23JpsiPlus1Track

# D+, Ds+ -> phi pi
BPHY23PhiPlus1Track = Analysis__JpsiPlus1Track(
    name                                = "BPHY23PhiPlus1Track",
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
ToolSvc += BPHY23PhiPlus1Track

## 6/ setup the combined augmentation/skimming tool
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrkTrk
BPHY23FourTrackReco_JpsiTrkTrk = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY23FourTrackReco_JpsiTrkTrk",
    Jpsi2PlusTrackName       = BPHY23JpsiPlus2Tracks,
    OutputVtxContainerName   = "BPHY23FourTrack_JpsiTrkTrk",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False)
ToolSvc += BPHY23FourTrackReco_JpsiTrkTrk

BPHY23FourTrackReco_UpsiTrkTrk = DerivationFramework__Reco_dimuTrkTrk(
    name                     = "BPHY23FourTrackReco_UpsiTrkTrk",
    Jpsi2PlusTrackName       = BPHY23UpsiPlus2Tracks,
    OutputVtxContainerName   = "BPHY23FourTrack_UpsiTrkTrk",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False)
ToolSvc += BPHY23FourTrackReco_UpsiTrkTrk

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrk
BPHY23ThreeTrackReco_JpsiTrk = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY23ThreeTrackReco_JpsiTrk",
    Jpsi1PlusTrackName       = BPHY23JpsiPlus1Track,
    OutputVtxContainerName   = "BPHY23ThreeTrack_JpsiTrk",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False)
ToolSvc += BPHY23ThreeTrackReco_JpsiTrk

BPHY23ThreeTrackReco_PhiTrk = DerivationFramework__Reco_dimuTrk(
    name                     = "BPHY23ThreeTrackReco_PhiTrk",
    Jpsi1PlusTrackName       = BPHY23PhiPlus1Track,
    OutputVtxContainerName   = "BPHY23ThreeTrack_PhiTrk",
    PVContainerName          = "PrimaryVertices",
    RefitPV                  = False)
ToolSvc += BPHY23ThreeTrackReco_PhiTrk


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu
BPHY23Select_Phi               = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Phi",
    HypothesisName             = "Phi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Phi_lo2,
    MassMax                    = Phi_hi2,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Phi

BPHY23Select_Jpsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Jpsi",
    HypothesisName             = "Jpsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Jpsi_lo2,
    MassMax                    = Jpsi_hi2,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Jpsi

BPHY23Select_Psi               = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Psi",
    HypothesisName             = "Psi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Psi_lo2,
    MassMax                    = Psi_hi2,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Psi

BPHY23Select_Upsi              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Upsi",
    HypothesisName             = "Upsi",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Upsi_lo2,
    MassMax                    = Upsi_hi2,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Upsi

BPHY23Select_Upsi2S            = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_Upsi2S",
    HypothesisName             = "Upsi2S",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = Upsi2S_lo2,
    MassMax                    = Upsi2S_hi2,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_Upsi2S

BPHY23Select_MuMu              = DerivationFramework__Select_onia2mumu(
    name                       = "BPHY23Select_MuMu",
    HypothesisName             = "MuMu",
    InputVtxContainerName      = "BPHY23OniaCandidates",
    TrkMasses                  = [Mumass, Mumass],
    MassMin                    = 5000.,
    MassMax                    = 9300.,
    DoVertexType               = 0)
ToolSvc += BPHY23Select_MuMu


########################
###  2 trk + 0 trks  ###
########################

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiPlusPsiCascade
#--------------------------------------------------------------------
# set up Onium (-> Jpsi pi pi) + Onium (-> mu mu) finder, for X(3872)/Psi(2S)
#--------------------------------------------------------------------
BPHY23Jpsi2PiOnium = DerivationFramework__JpsiPlusPsiCascade(
    name                     = "BPHY23Jpsi2PiOnium",
    HypothesisName           = "Jpsi2PiOnium",
    TrkVertexFitterTool      = BPHY23VertexFit,
    JpsiVertices             = "BPHY23OniaCandidates",
    JpsiVtxHypoNames         = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"],
    PsiVertices              = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsiDaughters     = 4,
    PsiMassLowerCut          = Psi_lo,
    PsiMassUpperCut          = Psi_hi,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23Jpsi2PiOniumRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23Jpsi2PiOniumCascadeVtx1", "BPHY23Jpsi2PiOniumCascadeVtx2", "BPHY23Jpsi2PiOniumCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsiMassConstraint  = True,
    JpsiMass                 = Jpsimass)
ToolSvc += BPHY23Jpsi2PiOnium

#--------------------------------------------------------------------
# set up Onium (-> Jpsi K pi) + Onium (-> mu mu) finder, for B0
#--------------------------------------------------------------------
BPHY23JpsiKPiOnium = DerivationFramework__JpsiPlusPsiCascade(
    name                       = "BPHY23JpsiKPiOnium",
    HypothesisName             = "JpsiKPiOnium",
    TrkVertexFitterTool        = BPHY23VertexFit,
    JpsiVertices               = "BPHY23OniaCandidates",
    JpsiVtxHypoNames           = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"],
    PsiVertices                = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsiDaughters       = 4,
    Vtx1Daug3MassHypo          = Kmass,
    PsiMassLowerCut            = B_lo,
    PsiMassUpperCut            = B_hi,
    DiTrackMassLower           = Kstar_lo,
    DiTrackMassUpper           = Kstar_hi,
    MassLowerCut               = 0.,
    MassUpperCut               = 31000.,
    Chi2Cut                    = 50.,
    VxPrimaryCandidateName     = "PrimaryVertices",
    RefPVContainerName         = "BPHY23JpsiKPiOniumRefPrimaryVertices",
    CascadeVertexCollections   = [ "BPHY23JpsiKPiOniumCascadeVtx1", "BPHY23JpsiKPiOniumCascadeVtx2", "BPHY23JpsiKPiOniumCascadeVtx3" ],
    RefitPV                    = True,
    MaxnPV                     = 2000,
    DoVertexType               = 7,
    ApplyJpsiMassConstraint    = True,
    JpsiMass                   = Jpsimass)
ToolSvc += BPHY23JpsiKPiOnium

#--------------------------------------------------------------------
# set up Onium (-> Jpsi K K) + Onium (-> mu mu) finder, for Bs0
#--------------------------------------------------------------------
BPHY23JpsiKKOnium = DerivationFramework__JpsiPlusPsiCascade(
    name                       = "BPHY23JpsiKKOnium",
    HypothesisName             = "JpsiKKOnium",
    TrkVertexFitterTool        = BPHY23VertexFit,
    JpsiVertices               = "BPHY23OniaCandidates",
    JpsiVtxHypoNames           = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"],
    PsiVertices                = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsiDaughters       = 4,
    Vtx1Daug3MassHypo          = Kmass,
    Vtx1Daug4MassHypo          = Kmass,
    PsiMassLowerCut            = Bs0_lo,
    PsiMassUpperCut            = Bs0_hi,
    DiTrackMassLower           = Phi_lo,
    DiTrackMassUpper           = Phi_hi,
    MassLowerCut               = 0.,
    MassUpperCut               = 31000.,
    Chi2Cut                    = 50.,
    VxPrimaryCandidateName     = "PrimaryVertices",
    RefPVContainerName         = "BPHY23JpsiKKOniumRefPrimaryVertices",
    CascadeVertexCollections   = [ "BPHY23JpsiKKOniumCascadeVtx1", "BPHY23JpsiKKOniumCascadeVtx2", "BPHY23JpsiKKOniumCascadeVtx3" ],
    RefitPV                    = True,
    MaxnPV                     = 2000,
    DoVertexType               = 7,
    ApplyJpsiMassConstraint    = True,
    JpsiMass                   = Jpsimass)
ToolSvc += BPHY23JpsiKKOnium

#--------------------------------------------------------------------
# set up Onium (-> Upsi pi pi) + Onium (-> mu mu) finder, for Upsi2S
#--------------------------------------------------------------------
BPHY23Upsi2PiOnium = DerivationFramework__JpsiPlusPsiCascade(
    name                     = "BPHY23Upsi2PiOnium",
    HypothesisName           = "Upsi2PiOnium",
    TrkVertexFitterTool      = BPHY23VertexFit,
    JpsiVertices             = "BPHY23OniaCandidates",
    JpsiVtxHypoNames         = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S", "MuMu"],
    PsiVertices              = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsiDaughters     = 4,
    PsiMassLowerCut          = Upsi2S_lo,
    PsiMassUpperCut          = Upsi2S_hi,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23Upsi2PiOniumRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23Upsi2PiOniumCascadeVtx1", "BPHY23Upsi2PiOniumCascadeVtx2", "BPHY23Upsi2PiOniumCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsiMassConstraint  = True,
    JpsiMass                 = Upsimass)
ToolSvc += BPHY23Upsi2PiOnium


#######################
###  1 trk + 0 trk  ###
#######################

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Onium (-> mu mu) finder, for Zc(3900)
#--------------------------------------------------------------------
BPHY23JpsiPiOnium = DerivationFramework__JpsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiOnium",
    HypothesisName           = "JpsiPiOnium",
    TrkVertexFitterTool      = BPHY23VertexFit,
    JpsiVertices             = "BPHY23OniaCandidates",
    JpsiVtxHypoNames         = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"],
    PsiVertices              = "BPHY23ThreeTrack_JpsiTrk",
    NumberOfPsiDaughters     = 3,
    PsiMassLowerCut          = Zc_lo,
    PsiMassUpperCut          = Zc_hi,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiOniumRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiOniumCascadeVtx1", "BPHY23JpsiPiOniumCascadeVtx2", "BPHY23JpsiPiOniumCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsiMassConstraint  = True,
    JpsiMass                 = Jpsimass)
ToolSvc += BPHY23JpsiPiOnium

#--------------------------------------------------------------------
# set up Meson (-> Jpsi K) + Onium (-> mu mu) finder, for B+
#--------------------------------------------------------------------
BPHY23JpsiKOnium = DerivationFramework__JpsiPlusPsiCascade(
    name                     = "BPHY23JpsiKOnium",
    HypothesisName           = "JpsiKOnium",
    TrkVertexFitterTool      = BPHY23VertexFit,
    JpsiVertices             = "BPHY23OniaCandidates",
    JpsiVtxHypoNames         = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"],
    PsiVertices              = "BPHY23ThreeTrack_JpsiTrk",
    NumberOfPsiDaughters     = 3,
    Vtx1Daug3MassHypo        = Kmass,
    PsiMassLowerCut          = B_lo,
    PsiMassUpperCut          = B_hi,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKOniumRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKOniumCascadeVtx1", "BPHY23JpsiKOniumCascadeVtx2", "BPHY23JpsiKOniumCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsiMassConstraint  = True,
    JpsiMass                 = Jpsimass)
ToolSvc += BPHY23JpsiKOnium

#--------------------------------------------------------------------
# set up Meson (-> phi pi) + Onium (-> mu mu) finder, for D+, Ds+
#--------------------------------------------------------------------
BPHY23PhiPiOnium = DerivationFramework__JpsiPlusPsiCascade(
    name                     = "BPHY23PhiPiOnium",
    HypothesisName           = "PhiPiOnium",
    TrkVertexFitterTool      = BPHY23VertexFit,
    JpsiVertices             = "BPHY23OniaCandidates",
    JpsiVtxHypoNames         = ["Phi", "Jpsi", "Psi", "Upsi", "Upsi2S"],
    PsiVertices              = "BPHY23ThreeTrack_PhiTrk",
    NumberOfPsiDaughters     = 3,
    PsiMassLowerCut          = Ds_lo,
    PsiMassUpperCut          = Ds_hi,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23PhiPiOniumRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23PhiPiOniumCascadeVtx1", "BPHY23PhiPiOniumCascadeVtx2", "BPHY23PhiPiOniumCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7)
ToolSvc += BPHY23PhiPiOnium


#########################
###  2 trks + 2 trks  ###
#########################

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__PsiPlusPsiCascade
#--------------------------------------------------------------------
# set up Onium (-> Jpsi pi pi) + Onium (-> Jpsi pi pi) finder, for X(3872)/Psi(2S) + X(3872)/Psi(2S)
#--------------------------------------------------------------------
BPHY23Jpsi2PiJpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23Jpsi2PiJpsi2Pi",
    HypothesisName           = "Jpsi2PiJpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Psi_lo2,
    Psi1MassUpperCut         = Psi_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Psi_lo2,
    Psi2MassUpperCut         = Psi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23Jpsi2PiJpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23Jpsi2PiJpsi2PiCascadeVtx1", "BPHY23Jpsi2PiJpsi2PiCascadeVtx2", "BPHY23Jpsi2PiJpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23Jpsi2PiJpsi2Pi

#--------------------------------------------------------------------
# set up Onium (-> Jpsi pi pi) + Onium (-> Jpsi K pi) finder, for X(3872)/Psi(2S) + B0
#--------------------------------------------------------------------
BPHY23Jpsi2PiJpsiKPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23Jpsi2PiJpsiKPi",
    HypothesisName           = "Jpsi2PiJpsiKPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Psi_lo2,
    Psi1MassUpperCut         = Psi_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = B_lo2,
    Psi2MassUpperCut         = B_hi2,
    DiTrack2MassLower        = Kstar_lo2,
    DiTrack2MassUpper        = Kstar_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23Jpsi2PiJpsiKPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23Jpsi2PiJpsiKPiCascadeVtx1", "BPHY23Jpsi2PiJpsiKPiCascadeVtx2", "BPHY23Jpsi2PiJpsiKPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23Jpsi2PiJpsiKPi

#--------------------------------------------------------------------
# set up Onium (-> Jpsi pi pi) + Onium (-> Jpsi K K) finder, for X(3872)/Psi(2S) + Bs0
#--------------------------------------------------------------------
BPHY23Jpsi2PiJpsiKK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23Jpsi2PiJpsiKK",
    HypothesisName           = "Jpsi2PiJpsiKK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Vtx2Daug4MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Psi_lo2,
    Psi1MassUpperCut         = Psi_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Bs0_lo2,
    Psi2MassUpperCut         = Bs0_hi2,
    DiTrack2MassLower        = Phi_lo2,
    DiTrack2MassUpper        = Phi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23Jpsi2PiJpsiKKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23Jpsi2PiJpsiKKCascadeVtx1", "BPHY23Jpsi2PiJpsiKKCascadeVtx2", "BPHY23Jpsi2PiJpsiKKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23Jpsi2PiJpsiKK

#--------------------------------------------------------------------
# set up Onium (-> Jpsi pi pi) + Onium (-> Upsi pi pi) finder, for X(3872)/Psi(2S) + Upsi2S
#--------------------------------------------------------------------
BPHY23Jpsi2PiUpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23Jpsi2PiUpsi2Pi",
    HypothesisName           = "Jpsi2PiUpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Psi_lo2,
    Psi1MassUpperCut         = Psi_hi2,
    Jpsi2MassLowerCut        = Upsi_lo2,
    Jpsi2MassUpperCut        = Upsi_hi2,
    Psi2MassLowerCut         = Upsi2S_lo2,
    Psi2MassUpperCut         = Upsi2S_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23Jpsi2PiUpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23Jpsi2PiUpsi2PiCascadeVtx1", "BPHY23Jpsi2PiUpsi2PiCascadeVtx2", "BPHY23Jpsi2PiUpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Upsimass)
ToolSvc += BPHY23Jpsi2PiUpsi2Pi

#--------------------------------------------------------------------
# set up Onium (-> Jpsi K pi) + Onium (-> Jpsi K pi) finder, for B0 + B0
#--------------------------------------------------------------------
BPHY23JpsiKPiJpsiKPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKPiJpsiKPi",
    HypothesisName           = "JpsiKPiJpsiKPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Vtx1Daug3MassHypo        = Kmass,
    Vtx2Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    DiTrack1MassLower        = Kstar_lo2,
    DiTrack1MassUpper        = Kstar_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = B_lo2,
    Psi2MassUpperCut         = B_hi2,
    DiTrack2MassLower        = Kstar_lo2,
    DiTrack2MassUpper        = Kstar_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKPiJpsiKPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKPiJpsiKPiCascadeVtx1", "BPHY23JpsiKPiJpsiKPiCascadeVtx2", "BPHY23JpsiKPiJpsiKPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKPiJpsiKPi

#--------------------------------------------------------------------
# set up Onium (-> Jpsi K pi) + Onium (-> Jpsi K K) finder, for B0 + Bs0
#--------------------------------------------------------------------
BPHY23JpsiKPiJpsiKK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKPiJpsiKK",
    HypothesisName           = "JpsiKPiJpsiKK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Vtx1Daug3MassHypo        = Kmass,
    Vtx2Daug3MassHypo        = Kmass,
    Vtx2Daug4MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    DiTrack1MassLower        = Kstar_lo2,
    DiTrack1MassUpper        = Kstar_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Bs0_lo2,
    Psi2MassUpperCut         = Bs0_hi2,
    DiTrack2MassLower        = Phi_lo2,
    DiTrack2MassUpper        = Phi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKPiJpsiKKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKPiJpsiKKCascadeVtx1", "BPHY23JpsiKPiJpsiKKCascadeVtx2", "BPHY23JpsiKPiJpsiKKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKPiJpsiKK

#--------------------------------------------------------------------
# set up Onium (-> Jpsi K pi) + Onium (-> Upsi pi pi) finder, for B0 + Upsi2S
#--------------------------------------------------------------------
BPHY23JpsiKPiUpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKPiUpsi2Pi",
    HypothesisName           = "JpsiKPiUpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Vtx1Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    DiTrack1MassLower        = Kstar_lo2,
    DiTrack1MassUpper        = Kstar_hi2,
    Jpsi2MassLowerCut        = Upsi_lo2,
    Jpsi2MassUpperCut        = Upsi_hi2,
    Psi2MassLowerCut         = Upsi2S_lo2,
    Psi2MassUpperCut         = Upsi2S_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKPiUpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKPiUpsi2PiCascadeVtx1", "BPHY23JpsiKPiUpsi2PiCascadeVtx2", "BPHY23JpsiKPiUpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Upsimass)
ToolSvc += BPHY23JpsiKPiUpsi2Pi

#--------------------------------------------------------------------
# set up Onium (-> Jpsi K K) + Onium (-> Jpsi K K) finder, for Bs0 + Bs0
#--------------------------------------------------------------------
BPHY23JpsiKKJpsiKK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKKJpsiKK",
    HypothesisName           = "JpsiKKJpsiKK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Vtx1Daug3MassHypo        = Kmass,
    Vtx1Daug4MassHypo        = Kmass,
    Vtx2Daug3MassHypo        = Kmass,
    Vtx2Daug4MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Bs0_lo2,
    Psi1MassUpperCut         = Bs0_hi2,
    DiTrack1MassLower        = Phi_lo2,
    DiTrack1MassUpper        = Phi_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Bs0_lo2,
    Psi2MassUpperCut         = Bs0_hi2,
    DiTrack2MassLower        = Phi_lo2,
    DiTrack2MassUpper        = Phi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKKJpsiKKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKKJpsiKKCascadeVtx1", "BPHY23JpsiKKJpsiKKCascadeVtx2", "BPHY23JpsiKKJpsiKKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKKJpsiKK

#--------------------------------------------------------------------
# set up Onium (-> Jpsi K K) + Onium (-> Upsi pi pi) finder, for Bs0 + Upsi2S
#--------------------------------------------------------------------
BPHY23JpsiKKUpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKKUpsi2Pi",
    HypothesisName           = "JpsiKKUpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Vtx1Daug3MassHypo        = Kmass,
    Vtx1Daug4MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Bs0_lo2,
    Psi1MassUpperCut         = Bs0_hi2,
    DiTrack1MassLower        = Phi_lo2,
    DiTrack1MassUpper        = Phi_hi2,
    Jpsi2MassLowerCut        = Upsi_lo2,
    Jpsi2MassUpperCut        = Upsi_hi2,
    Psi2MassLowerCut         = Upsi2S_lo2,
    Psi2MassUpperCut         = Upsi2S_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKKUpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKKUpsi2PiCascadeVtx1", "BPHY23JpsiKKUpsi2PiCascadeVtx2", "BPHY23JpsiKKUpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Upsimass)
ToolSvc += BPHY23JpsiKKUpsi2Pi

#--------------------------------------------------------------------
# set up Onium (-> Upsi pi pi) + Onium (-> Upsi pi pi) finder, for Upsi2S + Upsi2S
#--------------------------------------------------------------------
BPHY23Upsi2PiUpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23Upsi2PiUpsi2Pi",
    HypothesisName           = "Upsi2PiUpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    Psi2Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsi1Daughters    = 4,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Upsi_lo2,
    Jpsi1MassUpperCut        = Upsi_hi2,
    Psi1MassLowerCut         = Upsi2S_lo2,
    Psi1MassUpperCut         = Upsi2S_hi2,
    Jpsi2MassLowerCut        = Upsi_lo2,
    Jpsi2MassUpperCut        = Upsi_hi2,
    Psi2MassLowerCut         = Upsi2S_lo2,
    Psi2MassUpperCut         = Upsi2S_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23Upsi2PiUpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23Upsi2PiUpsi2PiCascadeVtx1", "BPHY23Upsi2PiUpsi2PiCascadeVtx2", "BPHY23Upsi2PiUpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Upsimass,
    Jpsi2Mass                = Upsimass)
ToolSvc += BPHY23Upsi2PiUpsi2Pi


########################
###  1 trk + 2 trks  ###
########################

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Onium (-> Jpsi pi pi) finder, for Zc(3900) + X(3872)/Psi(2S)
#--------------------------------------------------------------------
BPHY23JpsiPiJpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiJpsi2Pi",
    HypothesisName           = "JpsiPiJpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Zc_lo2,
    Psi1MassUpperCut         = Zc_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Psi_lo2,
    Psi2MassUpperCut         = Psi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiJpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiJpsi2PiCascadeVtx1", "BPHY23JpsiPiJpsi2PiCascadeVtx2", "BPHY23JpsiPiJpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiPiJpsi2Pi

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Onium (-> Jpsi K pi) finder, for Zc(3900) + B0
#--------------------------------------------------------------------
BPHY23JpsiPiJpsiKPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiJpsiKPi",
    HypothesisName           = "JpsiPiJpsiKPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Zc_lo2,
    Psi1MassUpperCut         = Zc_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = B_lo2,
    Psi2MassUpperCut         = B_hi2,
    DiTrack2MassLower        = Kstar_lo2,
    DiTrack2MassUpper        = Kstar_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiJpsiKPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiJpsiKPiCascadeVtx1", "BPHY23JpsiPiJpsiKPiCascadeVtx2", "BPHY23JpsiPiJpsiKPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiPiJpsiKPi

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Onium (-> Jpsi K K) finder, for Zc(3900) + Bs0
#--------------------------------------------------------------------
BPHY23JpsiPiJpsiKK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiJpsiKK",
    HypothesisName           = "JpsiPiJpsiKK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Vtx2Daug4MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Zc_lo2,
    Psi1MassUpperCut         = Zc_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Bs0_lo2,
    Psi2MassUpperCut         = Bs0_hi2,
    DiTrack2MassLower        = Phi_lo2,
    DiTrack2MassUpper        = Phi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiJpsiKKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiJpsiKKCascadeVtx1", "BPHY23JpsiPiJpsiKKCascadeVtx2", "BPHY23JpsiPiJpsiKKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiPiJpsiKK

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Onium (-> Upsi pi pi) finder, for Zc(3900) + Upsi2S
#--------------------------------------------------------------------
BPHY23JpsiPiUpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiUpsi2Pi",
    HypothesisName           = "JpsiPiUpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Zc_lo2,
    Psi1MassUpperCut         = Zc_hi2,
    Jpsi2MassLowerCut        = Upsi_lo2,
    Jpsi2MassUpperCut        = Upsi_hi2,
    Psi2MassLowerCut         = Upsi2S_lo2,
    Psi2MassUpperCut         = Upsi2S_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiUpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiUpsi2PiCascadeVtx1", "BPHY23JpsiPiUpsi2PiCascadeVtx2", "BPHY23JpsiPiUpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Upsimass)
ToolSvc += BPHY23JpsiPiUpsi2Pi

#--------------------------------------------------------------------
# set up Meson (-> Jpsi K) + Onium (-> Jpsi pi pi) finder, for B+/- + X(3872)/Psi(2S)
#--------------------------------------------------------------------
BPHY23JpsiKJpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKJpsi2Pi",
    HypothesisName           = "JpsiKJpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    Vtx1Daug3MassHypo        = Kmass,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Psi_lo2,
    Psi2MassUpperCut         = Psi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKJpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKJpsi2PiCascadeVtx1", "BPHY23JpsiKJpsi2PiCascadeVtx2", "BPHY23JpsiKJpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKJpsi2Pi

#--------------------------------------------------------------------
# set up Meson (-> Jpsi K) + Onium (-> Jpsi K pi) finder, for B+/- + B0
#--------------------------------------------------------------------
BPHY23JpsiKJpsiKPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKJpsiKPi",
    HypothesisName           = "JpsiKJpsiKPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    Vtx1Daug3MassHypo        = Kmass,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = B_lo2,
    Psi2MassUpperCut         = B_hi2,
    DiTrack2MassLower        = Kstar_lo2,
    DiTrack2MassUpper        = Kstar_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKJpsiKPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKJpsiKPiCascadeVtx1", "BPHY23JpsiKJpsiKPiCascadeVtx2", "BPHY23JpsiKJpsiKPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKJpsiKPi

#--------------------------------------------------------------------
# set up Meson (-> Jpsi K) + Onium (-> Jpsi K K) finder, for B+/- + Bs0
#--------------------------------------------------------------------
BPHY23JpsiKJpsiKK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKJpsiKK",
    HypothesisName           = "JpsiKJpsiKK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    Vtx1Daug3MassHypo        = Kmass,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Vtx2Daug4MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Bs0_lo2,
    Psi2MassUpperCut         = Bs0_hi2,
    DiTrack2MassLower        = Phi_lo2,
    DiTrack2MassUpper        = Phi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKJpsiKKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKJpsiKKCascadeVtx1", "BPHY23JpsiKJpsiKKCascadeVtx2", "BPHY23JpsiKJpsiKKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKJpsiKK

#--------------------------------------------------------------------
# set up Meson (-> Jpsi K) + Onium (-> Upsi pi pi) finder, for B+/- + Upsi2S
#--------------------------------------------------------------------
BPHY23JpsiKUpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKUpsi2Pi",
    HypothesisName           = "JpsiKUpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    Vtx1Daug3MassHypo        = Kmass,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    Jpsi2MassLowerCut        = Upsi_lo2,
    Jpsi2MassUpperCut        = Upsi_hi2,
    Psi2MassLowerCut         = Upsi2S_lo2,
    Psi2MassUpperCut         = Upsi2S_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKUpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKUpsi2PiCascadeVtx1", "BPHY23JpsiKUpsi2PiCascadeVtx2", "BPHY23JpsiKUpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Upsimass)
ToolSvc += BPHY23JpsiKUpsi2Pi

#--------------------------------------------------------------------
# set up Meson (-> phi pi) + Onium (-> Jpsi pi pi) finder, for Ds+/D+ + X(3872)/Psi(2S)
#--------------------------------------------------------------------
BPHY23PhiPiJpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23PhiPiJpsi2Pi",
    HypothesisName           = "PhiPiJpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_PhiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Phi_lo2,
    Jpsi1MassUpperCut        = Phi_hi2,
    Psi1MassLowerCut         = Ds_lo2,
    Psi1MassUpperCut         = Ds_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Psi_lo2,
    Psi2MassUpperCut         = Psi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23PhiPiJpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23PhiPiJpsi2PiCascadeVtx1", "BPHY23PhiPiJpsi2PiCascadeVtx2", "BPHY23PhiPiJpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi2MassConstraint = True,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23PhiPiJpsi2Pi

#--------------------------------------------------------------------
# set up Meson (-> phi pi) + Onium (-> Jpsi K pi) finder, for Ds+/D+ + B0
#--------------------------------------------------------------------
BPHY23PhiPiJpsiKPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23PhiPiJpsiKPi",
    HypothesisName           = "PhiPiJpsiKPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_PhiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Phi_lo2,
    Jpsi1MassUpperCut        = Phi_hi2,
    Psi1MassLowerCut         = Ds_lo2,
    Psi1MassUpperCut         = Ds_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = B_lo2,
    Psi2MassUpperCut         = B_hi2,
    DiTrack2MassLower        = Kstar_lo2,
    DiTrack2MassUpper        = Kstar_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23PhiPiJpsiKPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23PhiPiJpsiKPiCascadeVtx1", "BPHY23PhiPiJpsiKPiCascadeVtx2", "BPHY23PhiPiJpsiKPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi2MassConstraint = True,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23PhiPiJpsiKPi

#--------------------------------------------------------------------
# set up Meson (-> phi pi) + Onium (-> Jpsi K K) finder, for Ds+/D+ + Bs0
#--------------------------------------------------------------------
BPHY23PhiPiJpsiKK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23PhiPiJpsiKK",
    HypothesisName           = "PhiPiJpsiKK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_PhiTrk",
    Psi2Vertices             = "BPHY23FourTrack_JpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Vtx2Daug3MassHypo        = Kmass,
    Vtx2Daug4MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Phi_lo2,
    Jpsi1MassUpperCut        = Phi_hi2,
    Psi1MassLowerCut         = Ds_lo2,
    Psi1MassUpperCut         = Ds_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Bs0_lo2,
    Psi2MassUpperCut         = Bs0_hi2,
    DiTrack2MassLower        = Phi_lo2,
    DiTrack2MassUpper        = Phi_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23PhiPiJpsiKKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23PhiPiJpsiKKCascadeVtx1", "BPHY23PhiPiJpsiKKCascadeVtx2", "BPHY23PhiPiJpsiKKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi2MassConstraint = True,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23PhiPiJpsiKK

#--------------------------------------------------------------------
# set up Meson (-> phi pi) + Onium (-> Upsi pi pi) finder, for Ds+/D+ + Upsi2S
#--------------------------------------------------------------------
BPHY23PhiPiUpsi2Pi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23PhiPiUpsi2Pi",
    HypothesisName           = "PhiPiUpsi2Pi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_PhiTrk",
    Psi2Vertices             = "BPHY23FourTrack_UpsiTrkTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 4,
    Jpsi1MassLowerCut        = Phi_lo2,
    Jpsi1MassUpperCut        = Phi_hi2,
    Psi1MassLowerCut         = Ds_lo2,
    Psi1MassUpperCut         = Ds_hi2,
    Jpsi2MassLowerCut        = Upsi_lo2,
    Jpsi2MassUpperCut        = Upsi_hi2,
    Psi2MassLowerCut         = Upsi2S_lo2,
    Psi2MassUpperCut         = Upsi2S_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23PhiPiUpsi2PiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23PhiPiUpsi2PiCascadeVtx1", "BPHY23PhiPiUpsi2PiCascadeVtx2", "BPHY23PhiPiUpsi2PiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi2MassConstraint = True,
    Jpsi2Mass                = Upsimass)
ToolSvc += BPHY23PhiPiUpsi2Pi


#######################
###  1 trk + 1 trk  ###
#######################

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Meson (-> Jpsi pi) finder, for Zc(3900) + Zc(3900)
#--------------------------------------------------------------------
BPHY23JpsiPiJpsiPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiJpsiPi",
    HypothesisName           = "JpsiPiJpsiPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 3,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Zc_lo2,
    Psi1MassUpperCut         = Zc_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = Zc_lo2,
    Psi2MassUpperCut         = Zc_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiJpsiPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiJpsiPiCascadeVtx1", "BPHY23JpsiPiJpsiPiCascadeVtx2", "BPHY23JpsiPiJpsiPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiPiJpsiPi

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Meson (-> Jpsi K) finder, for Zc(3900) + B+/-
#--------------------------------------------------------------------
BPHY23JpsiPiJpsiK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiJpsiK",
    HypothesisName           = "JpsiPiJpsiK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 3,
    Vtx2Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Zc_lo2,
    Psi1MassUpperCut         = Zc_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = B_lo2,
    Psi2MassUpperCut         = B_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiJpsiKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiJpsiKCascadeVtx1", "BPHY23JpsiPiJpsiKCascadeVtx2", "BPHY23JpsiPiJpsiKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiPiJpsiK

#--------------------------------------------------------------------
# set up Meson (-> Jpsi pi) + Meson (-> phi pi) finder, for Zc(3900) + Ds+/D+
#--------------------------------------------------------------------
BPHY23JpsiPiPhiPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiPiPhiPi",
    HypothesisName           = "JpsiPiPhiPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23ThreeTrack_PhiTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 3,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = Zc_lo2,
    Psi1MassUpperCut         = Zc_hi2,
    Jpsi2MassLowerCut        = Phi_lo2,
    Jpsi2MassUpperCut        = Phi_hi2,
    Psi2MassLowerCut         = Ds_lo2,
    Psi2MassUpperCut         = Ds_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiPiPhiPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiPiPhiPiCascadeVtx1", "BPHY23JpsiPiPhiPiCascadeVtx2", "BPHY23JpsiPiPhiPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    Jpsi1Mass                = Jpsimass)
ToolSvc += BPHY23JpsiPiPhiPi

#--------------------------------------------------------------------
# set up Meson (-> Jpsi K) + Meson (-> Jpsi K) finder, for B+/- + B+/-
#--------------------------------------------------------------------
BPHY23JpsiKJpsiK = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKJpsiK",
    HypothesisName           = "JpsiKJpsiK",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    NumberOfPsi1Daughters    = 3,
    Vtx1Daug3MassHypo        = Kmass,
    NumberOfPsi2Daughters    = 3,
    Vtx2Daug3MassHypo        = Kmass,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    Jpsi2MassLowerCut        = Jpsi_lo2,
    Jpsi2MassUpperCut        = Jpsi_hi2,
    Psi2MassLowerCut         = B_lo2,
    Psi2MassUpperCut         = B_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKJpsiKRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKJpsiKCascadeVtx1", "BPHY23JpsiKJpsiKCascadeVtx2", "BPHY23JpsiKJpsiKCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    ApplyJpsi2MassConstraint = True,
    Jpsi1Mass                = Jpsimass,
    Jpsi2Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKJpsiK

#--------------------------------------------------------------------
# set up Meson (-> Jpsi K) + Meson (-> phi pi) finder, for B+/- + Ds+/D+
#--------------------------------------------------------------------
BPHY23JpsiKPhiPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23JpsiKPhiPi",
    HypothesisName           = "JpsiKPhiPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_JpsiTrk",
    Psi2Vertices             = "BPHY23ThreeTrack_PhiTrk",
    NumberOfPsi1Daughters    = 3,
    Vtx1Daug3MassHypo        = Kmass,
    NumberOfPsi2Daughters    = 3,
    Jpsi1MassLowerCut        = Jpsi_lo2,
    Jpsi1MassUpperCut        = Jpsi_hi2,
    Psi1MassLowerCut         = B_lo2,
    Psi1MassUpperCut         = B_hi2,
    Jpsi2MassLowerCut        = Phi_lo2,
    Jpsi2MassUpperCut        = Phi_hi2,
    Psi2MassLowerCut         = Ds_lo2,
    Psi2MassUpperCut         = Ds_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23JpsiKPhiPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23JpsiKPhiPiCascadeVtx1", "BPHY23JpsiKPhiPiCascadeVtx2", "BPHY23JpsiKPhiPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7,
    ApplyJpsi1MassConstraint = True,
    Jpsi1Mass                = Jpsimass)
ToolSvc += BPHY23JpsiKPhiPi

#--------------------------------------------------------------------
# set up Meson (-> phi pi) + Meson (-> phi pi) finder, for Ds+/D+ + Ds+/D+
#--------------------------------------------------------------------
BPHY23PhiPiPhiPi = DerivationFramework__PsiPlusPsiCascade(
    name                     = "BPHY23PhiPiPhiPi",
    HypothesisName           = "PhiPiPhiPi",
    TrkVertexFitterTool      = BPHY23VertexFit,
    Psi1Vertices             = "BPHY23ThreeTrack_PhiTrk",
    Psi2Vertices             = "BPHY23ThreeTrack_PhiTrk",
    NumberOfPsi1Daughters    = 3,
    NumberOfPsi2Daughters    = 3,
    Jpsi1MassLowerCut        = Phi_lo2,
    Jpsi1MassUpperCut        = Phi_hi2,
    Psi1MassLowerCut         = Ds_lo2,
    Psi1MassUpperCut         = Ds_hi2,
    Jpsi2MassLowerCut        = Phi_lo2,
    Jpsi2MassUpperCut        = Phi_hi2,
    Psi2MassLowerCut         = Ds_lo2,
    Psi2MassUpperCut         = Ds_hi2,
    MassLowerCut             = 0.,
    MassUpperCut             = 31000.,
    Chi2Cut                  = 50.,
    VxPrimaryCandidateName   = "PrimaryVertices",
    RefPVContainerName       = "BPHY23PhiPiPhiPiRefPrimaryVertices",
    CascadeVertexCollections = [ "BPHY23PhiPiPhiPiCascadeVtx1", "BPHY23PhiPiPhiPiCascadeVtx2", "BPHY23PhiPiPhiPiCascadeVtx3" ],
    RefitPV                  = True,
    MaxnPV                   = 2000,
    DoVertexType             = 7)
ToolSvc += BPHY23PhiPiPhiPi


#====================================================================
# Revertex with mass constraint
#====================================================================

##################################
###  Revertex: 2 trk + 0 trks  ###
##################################

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__ReVertex
#====================================================================
# Onium (-> Jpsi pi pi) + Onium (-> mu mu) finder, for X(3872)/Psi(2S)
#====================================================================
BPHY23Rev1_Jpsi2PiOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiOnium_Psi",
    InputVtxContainerName      = "BPHY23Jpsi2PiOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiOnium_Psi")
ToolSvc += BPHY23Rev1_Jpsi2PiOnium_Psi

BPHY23Rev1_Jpsi2PiOnium_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiOnium_X3872",
    InputVtxContainerName      = "BPHY23Jpsi2PiOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiOnium_X3872")
ToolSvc += BPHY23Rev1_Jpsi2PiOnium_X3872

BPHY23Rev2_Jpsi2PiOnium_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiOnium_Jpsi",
    InputVtxContainerName      = "BPHY23Jpsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Jpsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiOnium_Jpsi")
ToolSvc += BPHY23Rev2_Jpsi2PiOnium_Jpsi

BPHY23Rev2_Jpsi2PiOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiOnium_Psi",
    InputVtxContainerName      = "BPHY23Jpsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Psi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiOnium_Psi")
ToolSvc += BPHY23Rev2_Jpsi2PiOnium_Psi

BPHY23Rev2_Jpsi2PiOnium_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiOnium_Upsi",
    InputVtxContainerName      = "BPHY23Jpsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiOnium_Upsi")
ToolSvc += BPHY23Rev2_Jpsi2PiOnium_Upsi

BPHY23Rev2_Jpsi2PiOnium_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiOnium_Upsi2S",
    InputVtxContainerName      = "BPHY23Jpsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi2S"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiOnium_Upsi2S")
ToolSvc += BPHY23Rev2_Jpsi2PiOnium_Upsi2S

#====================================================================
# Onium (-> Jpsi K pi) + Onium (-> mu mu) finder, for B0
#====================================================================
BPHY23Rev1_JpsiKPiOnium = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKPiOnium",
    InputVtxContainerName      = "BPHY23JpsiKPiOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKPiOnium")
ToolSvc += BPHY23Rev1_JpsiKPiOnium

BPHY23Rev2_JpsiKPiOnium_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPiOnium_Jpsi",
    InputVtxContainerName      = "BPHY23JpsiKPiOniumCascadeVtx2",
    HypothesisNames            = ["Jpsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPiOnium_Jpsi")
ToolSvc += BPHY23Rev2_JpsiKPiOnium_Jpsi

BPHY23Rev2_JpsiKPiOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPiOnium_Psi",
    InputVtxContainerName      = "BPHY23JpsiKPiOniumCascadeVtx2",
    HypothesisNames            = ["Psi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPiOnium_Psi")
ToolSvc += BPHY23Rev2_JpsiKPiOnium_Psi

BPHY23Rev2_JpsiKPiOnium_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPiOnium_Upsi",
    InputVtxContainerName      = "BPHY23JpsiKPiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPiOnium_Upsi")
ToolSvc += BPHY23Rev2_JpsiKPiOnium_Upsi

BPHY23Rev2_JpsiKPiOnium_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPiOnium_Upsi2S",
    InputVtxContainerName      = "BPHY23JpsiKPiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi2S"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPiOnium_Upsi2S")
ToolSvc += BPHY23Rev2_JpsiKPiOnium_Upsi2S

#====================================================================
# Onium (-> Jpsi K K) + Onium (-> mu mu) finder, for Bs0
#====================================================================
BPHY23Rev1_JpsiKKOnium = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKKOnium",
    InputVtxContainerName      = "BPHY23JpsiKKOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKKOnium")
ToolSvc += BPHY23Rev1_JpsiKKOnium

BPHY23Rev2_JpsiKKOnium_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKKOnium_Jpsi",
    InputVtxContainerName      = "BPHY23JpsiKKOniumCascadeVtx2",
    HypothesisNames            = ["Jpsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKKOnium_Jpsi")
ToolSvc += BPHY23Rev2_JpsiKKOnium_Jpsi

BPHY23Rev2_JpsiKKOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKKOnium_Psi",
    InputVtxContainerName      = "BPHY23JpsiKKOniumCascadeVtx2",
    HypothesisNames            = ["Psi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKKOnium_Psi")
ToolSvc += BPHY23Rev2_JpsiKKOnium_Psi

BPHY23Rev2_JpsiKKOnium_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKKOnium_Upsi",
    InputVtxContainerName      = "BPHY23JpsiKKOniumCascadeVtx2",
    HypothesisNames            = ["Upsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKKOnium_Upsi")
ToolSvc += BPHY23Rev2_JpsiKKOnium_Upsi

BPHY23Rev2_JpsiKKOnium_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKKOnium_Upsi2S",
    InputVtxContainerName      = "BPHY23JpsiKKOniumCascadeVtx2",
    HypothesisNames            = ["Upsi2S"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKKOnium_Upsi2S")
ToolSvc += BPHY23Rev2_JpsiKKOnium_Upsi2S

#====================================================================
# Onium (-> Upsi pi pi) + Onium (-> mu mu) finder, for Upsi2S
#====================================================================
BPHY23Rev1_Upsi2PiOnium = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Upsi2PiOnium",
    InputVtxContainerName      = "BPHY23Upsi2PiOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Upsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Upsi2PiOnium")
ToolSvc += BPHY23Rev1_Upsi2PiOnium

BPHY23Rev2_Upsi2PiOnium_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Upsi2PiOnium_Jpsi",
    InputVtxContainerName      = "BPHY23Upsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Jpsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Upsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Upsi2PiOnium_Jpsi")
ToolSvc += BPHY23Rev2_Upsi2PiOnium_Jpsi

BPHY23Rev2_Upsi2PiOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Upsi2PiOnium_Psi",
    InputVtxContainerName      = "BPHY23Upsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Psi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Upsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Upsi2PiOnium_Psi")
ToolSvc += BPHY23Rev2_Upsi2PiOnium_Psi

BPHY23Rev2_Upsi2PiOnium_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Upsi2PiOnium_Upsi",
    InputVtxContainerName      = "BPHY23Upsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Upsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Upsi2PiOnium_Upsi")
ToolSvc += BPHY23Rev2_Upsi2PiOnium_Upsi

BPHY23Rev2_Upsi2PiOnium_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Upsi2PiOnium_Upsi2S",
    InputVtxContainerName      = "BPHY23Upsi2PiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi2S"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Upsi2PiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Upsi2PiOnium_Upsi2S")
ToolSvc += BPHY23Rev2_Upsi2PiOnium_Upsi2S


#################################
###  Revertex: 1 trk + 0 trk  ###
#################################

#====================================================================
# Meson (-> Jpsi pi) + Onium (-> mu mu) finder, for Zc(3900)
#====================================================================
BPHY23Rev1_JpsiPiOnium = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiOnium",
    InputVtxContainerName      = "BPHY23JpsiPiOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiOnium")
ToolSvc += BPHY23Rev1_JpsiPiOnium

BPHY23Rev2_JpsiPiOnium_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiOnium_Jpsi",
    InputVtxContainerName      = "BPHY23JpsiPiOniumCascadeVtx2",
    HypothesisNames            = ["Jpsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiOnium_Jpsi")
ToolSvc += BPHY23Rev2_JpsiPiOnium_Jpsi

BPHY23Rev2_JpsiPiOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiOnium_Psi",
    InputVtxContainerName      = "BPHY23JpsiPiOniumCascadeVtx2",
    HypothesisNames            = ["Psi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiOnium_Psi")
ToolSvc += BPHY23Rev2_JpsiPiOnium_Psi

BPHY23Rev2_JpsiPiOnium_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiOnium_Upsi",
    InputVtxContainerName      = "BPHY23JpsiPiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiOnium_Upsi")
ToolSvc += BPHY23Rev2_JpsiPiOnium_Upsi

BPHY23Rev2_JpsiPiOnium_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiOnium_Upsi2S",
    InputVtxContainerName      = "BPHY23JpsiPiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi2S"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiOnium_Upsi2S")
ToolSvc += BPHY23Rev2_JpsiPiOnium_Upsi2S

#====================================================================
# Meson (-> Jpsi K) + Onium (-> mu mu) finder, for B+
#====================================================================
BPHY23Rev1_JpsiKOnium = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKOnium",
    InputVtxContainerName      = "BPHY23JpsiKOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKOnium")
ToolSvc += BPHY23Rev1_JpsiKOnium

BPHY23Rev2_JpsiKOnium_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKOnium_Jpsi",
    InputVtxContainerName      = "BPHY23JpsiKOniumCascadeVtx2",
    HypothesisNames            = ["Jpsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKOnium_Jpsi")
ToolSvc += BPHY23Rev2_JpsiKOnium_Jpsi

BPHY23Rev2_JpsiKOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKOnium_Psi",
    InputVtxContainerName      = "BPHY23JpsiKOniumCascadeVtx2",
    HypothesisNames            = ["Psi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKOnium_Psi")
ToolSvc += BPHY23Rev2_JpsiKOnium_Psi

BPHY23Rev2_JpsiKOnium_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKOnium_Upsi",
    InputVtxContainerName      = "BPHY23JpsiKOniumCascadeVtx2",
    HypothesisNames            = ["Upsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKOnium_Upsi")
ToolSvc += BPHY23Rev2_JpsiKOnium_Upsi

BPHY23Rev2_JpsiKOnium_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKOnium_Upsi2S",
    InputVtxContainerName      = "BPHY23JpsiKOniumCascadeVtx2",
    HypothesisNames            = ["Upsi2S"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKOnium_Upsi2S")
ToolSvc += BPHY23Rev2_JpsiKOnium_Upsi2S

#====================================================================
# Meson (-> phi pi) + Onium (-> mu mu) finder, for D+, Ds+
#====================================================================
BPHY23Rev1_PhiPiOnium_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiOnium_Dpm",
    InputVtxContainerName      = "BPHY23PhiPiOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiOnium_Dpm")
ToolSvc += BPHY23Rev1_PhiPiOnium_Dpm

BPHY23Rev1_PhiPiOnium_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiOnium_Dspm",
    InputVtxContainerName      = "BPHY23PhiPiOniumCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiOnium_Dspm")
ToolSvc += BPHY23Rev1_PhiPiOnium_Dspm

BPHY23Rev2_PhiPiOnium_Jpsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiOnium_Jpsi",
    InputVtxContainerName      = "BPHY23PhiPiOniumCascadeVtx2",
    HypothesisNames            = ["Jpsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Jpsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiOnium_Jpsi")
ToolSvc += BPHY23Rev2_PhiPiOnium_Jpsi

BPHY23Rev2_PhiPiOnium_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiOnium_Psi",
    InputVtxContainerName      = "BPHY23PhiPiOniumCascadeVtx2",
    HypothesisNames            = ["Psi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiOnium_Psi")
ToolSvc += BPHY23Rev2_PhiPiOnium_Psi

BPHY23Rev2_PhiPiOnium_Upsi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiOnium_Upsi",
    InputVtxContainerName      = "BPHY23PhiPiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsimass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiOnium_Upsi")
ToolSvc += BPHY23Rev2_PhiPiOnium_Upsi

BPHY23Rev2_PhiPiOnium_Upsi2S = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiOnium_Upsi2S",
    InputVtxContainerName      = "BPHY23PhiPiOniumCascadeVtx2",
    HypothesisNames            = ["Upsi2S"],
    TrackIndices               = [ 0, 1 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiOniumRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    MassInputParticles         = [Mumass, Mumass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool	       = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiOnium_Upsi2S")
ToolSvc += BPHY23Rev2_PhiPiOnium_Upsi2S


###################################
###  Revertex: 2 trks + 2 trks  ###
###################################

#====================================================================
# Onium (-> Jpsi pi pi) + Onium (-> Jpsi pi pi) finder, for X(3872)/Psi(2S) + X(3872)/Psi(2S)
#====================================================================
BPHY23Rev1_Jpsi2PiJpsi2Pi_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiJpsi2Pi_Psi",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiJpsi2Pi_Psi")
ToolSvc += BPHY23Rev1_Jpsi2PiJpsi2Pi_Psi

BPHY23Rev1_Jpsi2PiJpsi2Pi_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiJpsi2Pi_X3872",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiJpsi2Pi_X3872")
ToolSvc += BPHY23Rev1_Jpsi2PiJpsi2Pi_X3872

BPHY23Rev2_Jpsi2PiJpsi2Pi_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiJpsi2Pi_Psi",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiJpsi2Pi_Psi")
ToolSvc += BPHY23Rev2_Jpsi2PiJpsi2Pi_Psi

BPHY23Rev2_Jpsi2PiJpsi2Pi_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiJpsi2Pi_X3872",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiJpsi2Pi_X3872")
ToolSvc += BPHY23Rev2_Jpsi2PiJpsi2Pi_X3872

#====================================================================
# Onium (-> Jpsi pi pi) + Onium (-> Jpsi K pi) finder, for X(3872)/Psi(2S) + B0
#====================================================================
BPHY23Rev1_Jpsi2PiJpsiKPi_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiJpsiKPi_Psi",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsiKPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiJpsiKPi_Psi")
ToolSvc += BPHY23Rev1_Jpsi2PiJpsiKPi_Psi

BPHY23Rev1_Jpsi2PiJpsiKPi_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiJpsiKPi_X3872",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsiKPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiJpsiKPi_X3872")
ToolSvc += BPHY23Rev1_Jpsi2PiJpsiKPi_X3872

BPHY23Rev2_Jpsi2PiJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiJpsiKPi",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsiKPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiJpsiKPi")
ToolSvc += BPHY23Rev2_Jpsi2PiJpsiKPi

#====================================================================
# Onium (-> Jpsi pi pi) + Onium (-> Jpsi K K) finder, for X(3872)/Psi(2S) + Bs0
#====================================================================
BPHY23Rev1_Jpsi2PiJpsiKK_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiJpsiKK_Psi",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiJpsiKK_Psi")
ToolSvc += BPHY23Rev1_Jpsi2PiJpsiKK_Psi

BPHY23Rev1_Jpsi2PiJpsiKK_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiJpsiKK_X3872",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiJpsiKK_X3872")
ToolSvc += BPHY23Rev1_Jpsi2PiJpsiKK_X3872

BPHY23Rev2_Jpsi2PiJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiJpsiKK",
    InputVtxContainerName      = "BPHY23Jpsi2PiJpsiKKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiJpsiKK")
ToolSvc += BPHY23Rev2_Jpsi2PiJpsiKK

#====================================================================
# Onium (-> Jpsi pi pi) + Onium (-> Upsi pi pi) finder, for X(3872)/Psi(2S) + Upsi2S
#====================================================================
BPHY23Rev1_Jpsi2PiUpsi2Pi_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiUpsi2Pi_Psi",
    InputVtxContainerName      = "BPHY23Jpsi2PiUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiUpsi2Pi_Psi")
ToolSvc += BPHY23Rev1_Jpsi2PiUpsi2Pi_Psi

BPHY23Rev1_Jpsi2PiUpsi2Pi_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Jpsi2PiUpsi2Pi_X3872",
    InputVtxContainerName      = "BPHY23Jpsi2PiUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Jpsi2PiUpsi2Pi_X3872")
ToolSvc += BPHY23Rev1_Jpsi2PiUpsi2Pi_X3872

BPHY23Rev2_Jpsi2PiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Jpsi2PiUpsi2Pi",
    InputVtxContainerName      = "BPHY23Jpsi2PiUpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Jpsi2PiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Jpsi2PiUpsi2Pi")
ToolSvc += BPHY23Rev2_Jpsi2PiUpsi2Pi

#====================================================================
# Onium (-> Jpsi K pi) + Onium (-> Jpsi K pi) finder, for B0 + B0
#====================================================================
BPHY23Rev1_JpsiKPiJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKPiJpsiKPi",
    InputVtxContainerName      = "BPHY23JpsiKPiJpsiKPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKPiJpsiKPi")
ToolSvc += BPHY23Rev1_JpsiKPiJpsiKPi

BPHY23Rev2_JpsiKPiJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPiJpsiKPi",
    InputVtxContainerName      = "BPHY23JpsiKPiJpsiKPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPiJpsiKPi")
ToolSvc += BPHY23Rev2_JpsiKPiJpsiKPi

#====================================================================
# Onium (-> Jpsi K pi) + Onium (-> Jpsi K K) finder, for B0 + Bs0
#====================================================================
BPHY23Rev1_JpsiKPiJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKPiJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiKPiJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKPiJpsiKK")
ToolSvc += BPHY23Rev1_JpsiKPiJpsiKK

BPHY23Rev2_JpsiKPiJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPiJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiKPiJpsiKKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPiJpsiKK")
ToolSvc += BPHY23Rev2_JpsiKPiJpsiKK

#====================================================================
# Onium (-> Jpsi K pi) + Onium (-> Upsi pi pi) finder, for B0 + Upsi2S
#====================================================================
BPHY23Rev1_JpsiKPiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKPiUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiKPiUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKPiUpsi2Pi")
ToolSvc += BPHY23Rev1_JpsiKPiUpsi2Pi

BPHY23Rev2_JpsiKPiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPiUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiKPiUpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPiUpsi2Pi")
ToolSvc += BPHY23Rev2_JpsiKPiUpsi2Pi

#====================================================================
# Onium (-> Jpsi K K) + Onium (-> Jpsi K K) finder, for Bs0 + Bs0
#====================================================================
BPHY23Rev1_JpsiKKJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKKJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiKKJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKKJpsiKK")
ToolSvc += BPHY23Rev1_JpsiKKJpsiKK

BPHY23Rev2_JpsiKKJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKKJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiKKJpsiKKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKKJpsiKK")
ToolSvc += BPHY23Rev2_JpsiKKJpsiKK

#====================================================================
# Onium (-> Jpsi K K) + Onium (-> Upsi pi pi) finder, for Bs0 + Upsi2S
#====================================================================
BPHY23Rev1_JpsiKKUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKKUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiKKUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKKUpsi2Pi")
ToolSvc += BPHY23Rev1_JpsiKKUpsi2Pi

BPHY23Rev2_JpsiKKUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKKUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiKKUpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKKUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKKUpsi2Pi")
ToolSvc += BPHY23Rev2_JpsiKKUpsi2Pi

#====================================================================
# Onium (-> Upsi pi pi) + Onium (-> Upsi pi pi) finder, for Upsi2S + Upsi2S
#====================================================================
BPHY23Rev1_Upsi2PiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_Upsi2PiUpsi2Pi",
    InputVtxContainerName      = "BPHY23Upsi2PiUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Upsi2PiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_Upsi2PiUpsi2Pi")
ToolSvc += BPHY23Rev1_Upsi2PiUpsi2Pi

BPHY23Rev2_Upsi2PiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_Upsi2PiUpsi2Pi",
    InputVtxContainerName      = "BPHY23Upsi2PiUpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23Upsi2PiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_Upsi2PiUpsi2Pi")
ToolSvc += BPHY23Rev2_Upsi2PiUpsi2Pi


##################################
###  Revertex: 1 trk + 2 trks  ###
##################################

#====================================================================
# Meson (-> Jpsi pi) + Onium (-> Jpsi pi pi) finder, for Zc(3900) + X(3872)/Psi(2S)
#====================================================================
BPHY23Rev1_JpsiPiJpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiJpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiPiJpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiJpsi2Pi")
ToolSvc += BPHY23Rev1_JpsiPiJpsi2Pi

BPHY23Rev2_JpsiPiJpsi2Pi_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiJpsi2Pi_Psi",
    InputVtxContainerName      = "BPHY23JpsiPiJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiJpsi2Pi_Psi")
ToolSvc += BPHY23Rev2_JpsiPiJpsi2Pi_Psi

BPHY23Rev2_JpsiPiJpsi2Pi_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiJpsi2Pi_X3872",
    InputVtxContainerName      = "BPHY23JpsiPiJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiJpsi2Pi_X3872")
ToolSvc += BPHY23Rev2_JpsiPiJpsi2Pi_X3872

#====================================================================
# Meson (-> Jpsi pi) + Onium (-> Jpsi K pi) finder, for Zc(3900) + B0
#====================================================================
BPHY23Rev1_JpsiPiJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiJpsiKPi",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiKPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiJpsiKPi")
ToolSvc += BPHY23Rev1_JpsiPiJpsiKPi

BPHY23Rev2_JpsiPiJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiJpsiKPi",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiKPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiJpsiKPi")
ToolSvc += BPHY23Rev2_JpsiPiJpsiKPi

#====================================================================
# Meson (-> Jpsi pi) + Onium (-> Jpsi K K) finder, for Zc(3900) + Bs0
#====================================================================
BPHY23Rev1_JpsiPiJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiJpsiKK")
ToolSvc += BPHY23Rev1_JpsiPiJpsiKK

BPHY23Rev2_JpsiPiJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiKKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiJpsiKK")
ToolSvc += BPHY23Rev2_JpsiPiJpsiKK

#====================================================================
# Meson (-> Jpsi pi) + Onium (-> Upsi pi pi) finder, for Zc(3900) + Upsi2S
#====================================================================
BPHY23Rev1_JpsiPiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiPiUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiUpsi2Pi")
ToolSvc += BPHY23Rev1_JpsiPiUpsi2Pi

BPHY23Rev2_JpsiPiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiPiUpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiUpsi2Pi")
ToolSvc += BPHY23Rev2_JpsiPiUpsi2Pi

#====================================================================
# Meson (-> Jpsi K) + Onium (-> Jpsi pi pi) finder, for B+/- + X(3872)/Psi(2S)
#====================================================================
BPHY23Rev1_JpsiKJpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKJpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiKJpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKJpsi2Pi")
ToolSvc += BPHY23Rev1_JpsiKJpsi2Pi

BPHY23Rev2_JpsiKJpsi2Pi_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKJpsi2Pi_Psi",
    InputVtxContainerName      = "BPHY23JpsiKJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKJpsi2Pi_Psi")
ToolSvc += BPHY23Rev2_JpsiKJpsi2Pi_Psi

BPHY23Rev2_JpsiKJpsi2Pi_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKJpsi2Pi_X3872",
    InputVtxContainerName      = "BPHY23JpsiKJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKJpsi2Pi_X3872")
ToolSvc += BPHY23Rev2_JpsiKJpsi2Pi_X3872

#====================================================================
# Meson (-> Jpsi K) + Onium (-> Jpsi K pi) finder, for B+/- + B0
#====================================================================
BPHY23Rev1_JpsiKJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKJpsiKPi",
    InputVtxContainerName      = "BPHY23JpsiKJpsiKPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKJpsiKPi")
ToolSvc += BPHY23Rev1_JpsiKJpsiKPi

BPHY23Rev2_JpsiKJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKJpsiKPi",
    InputVtxContainerName      = "BPHY23JpsiKJpsiKPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKJpsiKPi")
ToolSvc += BPHY23Rev2_JpsiKJpsiKPi

#====================================================================
# Meson (-> Jpsi K) + Onium (-> Jpsi K K) finder, for B+/- + Bs0
#====================================================================
BPHY23Rev1_JpsiKJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiKJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKJpsiKK")
ToolSvc += BPHY23Rev1_JpsiKJpsiKK

BPHY23Rev2_JpsiKJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKJpsiKK",
    InputVtxContainerName      = "BPHY23JpsiKJpsiKKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKJpsiKK")
ToolSvc += BPHY23Rev2_JpsiKJpsiKK

#====================================================================
# Meson (-> Jpsi K) + Onium (-> Upsi pi pi) finder, for B+/- + Upsi2S
#====================================================================
BPHY23Rev1_JpsiKUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiKUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKUpsi2Pi")
ToolSvc += BPHY23Rev1_JpsiKUpsi2Pi

BPHY23Rev2_JpsiKUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKUpsi2Pi",
    InputVtxContainerName      = "BPHY23JpsiKUpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKUpsi2Pi")
ToolSvc += BPHY23Rev2_JpsiKUpsi2Pi

#====================================================================
# Meson (-> phi pi) + Onium (-> Jpsi pi pi) finder, for Ds+/D+ + X(3872)/Psi(2S)
#====================================================================
BPHY23Rev1_PhiPiJpsi2Pi_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiJpsi2Pi_Dpm",
    InputVtxContainerName      = "BPHY23PhiPiJpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiJpsi2Pi_Dpm")
ToolSvc += BPHY23Rev1_PhiPiJpsi2Pi_Dpm

BPHY23Rev1_PhiPiJpsi2Pi_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiJpsi2Pi_Dspm",
    InputVtxContainerName      = "BPHY23PhiPiJpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiJpsi2Pi_Dspm")
ToolSvc += BPHY23Rev1_PhiPiJpsi2Pi_Dspm

BPHY23Rev2_PhiPiJpsi2Pi_Psi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiJpsi2Pi_Psi",
    InputVtxContainerName      = "BPHY23PhiPiJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Psi2Smass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiJpsi2Pi_Psi")
ToolSvc += BPHY23Rev2_PhiPiJpsi2Pi_Psi

BPHY23Rev2_PhiPiJpsi2Pi_X3872 = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiJpsi2Pi_X3872",
    InputVtxContainerName      = "BPHY23PhiPiJpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = X3872mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiJpsi2Pi_X3872")
ToolSvc += BPHY23Rev2_PhiPiJpsi2Pi_X3872

#====================================================================
# Meson (-> phi pi) + Onium (-> Jpsi K pi) finder, for Ds+/D+ + B0
#====================================================================
BPHY23Rev1_PhiPiJpsiKPi_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiJpsiKPi_Dpm",
    InputVtxContainerName      = "BPHY23PhiPiJpsiKPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiJpsiKPi_Dpm")
ToolSvc += BPHY23Rev1_PhiPiJpsiKPi_Dpm

BPHY23Rev1_PhiPiJpsiKPi_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiJpsiKPi_Dspm",
    InputVtxContainerName      = "BPHY23PhiPiJpsiKPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiJpsiKPi_Dspm")
ToolSvc += BPHY23Rev1_PhiPiJpsiKPi_Dspm

BPHY23Rev2_PhiPiJpsiKPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiJpsiKPi",
    InputVtxContainerName      = "BPHY23PhiPiJpsiKPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsiKPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = B0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiJpsiKPi")
ToolSvc += BPHY23Rev2_PhiPiJpsiKPi

#====================================================================
# Meson (-> phi pi) + Onium (-> Jpsi K K) finder, for Ds+/D+ + Bs0
#====================================================================
BPHY23Rev1_PhiPiJpsiKK_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiJpsiKK_Dpm",
    InputVtxContainerName      = "BPHY23PhiPiJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiJpsiKK_Dpm")
ToolSvc += BPHY23Rev1_PhiPiJpsiKK_Dpm

BPHY23Rev1_PhiPiJpsiKK_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiJpsiKK_Dspm",
    InputVtxContainerName      = "BPHY23PhiPiJpsiKKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiJpsiKK_Dspm")
ToolSvc += BPHY23Rev1_PhiPiJpsiKK_Dspm

BPHY23Rev2_PhiPiJpsiKK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiJpsiKK",
    InputVtxContainerName      = "BPHY23PhiPiJpsiKKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiJpsiKKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bs0mass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiJpsiKK")
ToolSvc += BPHY23Rev2_PhiPiJpsiKK

#====================================================================
# Meson (-> phi pi) + Onium (-> Upsi pi pi) finder, for Ds+/D+ + Upsi2S
#====================================================================
BPHY23Rev1_PhiPiUpsi2Pi_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiUpsi2Pi_Dpm",
    InputVtxContainerName      = "BPHY23PhiPiUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiUpsi2Pi_Dpm")
ToolSvc += BPHY23Rev1_PhiPiUpsi2Pi_Dpm

BPHY23Rev1_PhiPiUpsi2Pi_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiUpsi2Pi_Dspm",
    InputVtxContainerName      = "BPHY23PhiPiUpsi2PiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiUpsi2Pi_Dspm")
ToolSvc += BPHY23Rev1_PhiPiUpsi2Pi_Dspm

BPHY23Rev2_PhiPiUpsi2Pi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiUpsi2Pi",
    InputVtxContainerName      = "BPHY23PhiPiUpsi2PiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2, 3 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiUpsi2PiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Upsi2Smass,
    SubVertexMass              = Upsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiUpsi2Pi")
ToolSvc += BPHY23Rev2_PhiPiUpsi2Pi


#################################
###  Revertex: 1 trk + 1 trk  ###
#################################

#====================================================================
# Meson (-> Jpsi pi) + Meson (-> Jpsi pi) finder, for Zc(3900) + Zc(3900)
#====================================================================
BPHY23Rev1_JpsiPiJpsiPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiJpsiPi",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiJpsiPi")
ToolSvc += BPHY23Rev1_JpsiPiJpsiPi

BPHY23Rev2_JpsiPiJpsiPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiJpsiPi",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiJpsiPi")
ToolSvc += BPHY23Rev2_JpsiPiJpsiPi

#====================================================================
# Meson (-> Jpsi pi) + Meson (-> Jpsi K) finder, for Zc(3900) + B+/-
#====================================================================
BPHY23Rev1_JpsiPiJpsiK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiJpsiK",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiJpsiK")
ToolSvc += BPHY23Rev1_JpsiPiJpsiK

BPHY23Rev2_JpsiPiJpsiK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiJpsiK",
    InputVtxContainerName      = "BPHY23JpsiPiJpsiKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiJpsiKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiJpsiK")
ToolSvc += BPHY23Rev2_JpsiPiJpsiK

#====================================================================
# set up Meson (-> Jpsi pi) + Meson (-> phi pi) finder, for Zc(3900) + Ds+/D+
#====================================================================
BPHY23Rev1_JpsiPiPhiPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiPiPhiPi",
    InputVtxContainerName      = "BPHY23JpsiPiPhiPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Zcmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiPiPhiPi")
ToolSvc += BPHY23Rev1_JpsiPiPhiPi

BPHY23Rev2_JpsiPiPhiPi_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiPhiPi_Dpm",
    InputVtxContainerName      = "BPHY23JpsiPiPhiPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiPhiPi_Dpm")
ToolSvc += BPHY23Rev2_JpsiPiPhiPi_Dpm

BPHY23Rev2_JpsiPiPhiPi_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiPiPhiPi_Dspm",
    InputVtxContainerName      = "BPHY23JpsiPiPhiPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiPiPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiPiPhiPi_Dspm")
ToolSvc += BPHY23Rev2_JpsiPiPhiPi_Dspm

#====================================================================
# Meson (-> Jpsi K) + Meson (-> Jpsi K) finder, for B+/- + B+/-
#====================================================================
BPHY23Rev1_JpsiKJpsiK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKJpsiK",
    InputVtxContainerName      = "BPHY23JpsiKJpsiKCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsiKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKJpsiK")
ToolSvc += BPHY23Rev1_JpsiKJpsiK

BPHY23Rev2_JpsiKJpsiK = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKJpsiK",
    InputVtxContainerName      = "BPHY23JpsiKJpsiKCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKJpsiKRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKJpsiK")
ToolSvc += BPHY23Rev2_JpsiKJpsiK

#====================================================================
# Meson (-> Jpsi K) + Meson (-> phi pi) finder, for B+/- + Ds+/D+
#====================================================================
BPHY23Rev1_JpsiKPhiPi = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_JpsiKPhiPi",
    InputVtxContainerName      = "BPHY23JpsiKPhiPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    SubVertexTrackIndices      = [ 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Bpmmass,
    SubVertexMass              = Jpsimass,
    MassInputParticles         = [Mumass, Mumass, Kmass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_JpsiKPhiPi")
ToolSvc += BPHY23Rev1_JpsiKPhiPi

BPHY23Rev2_JpsiKPhiPi_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPhiPi_Dpm",
    InputVtxContainerName      = "BPHY23JpsiKPhiPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPhiPi_Dpm")
ToolSvc += BPHY23Rev2_JpsiKPhiPi_Dpm

BPHY23Rev2_JpsiKPhiPi_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_JpsiKPhiPi_Dspm",
    InputVtxContainerName      = "BPHY23JpsiKPhiPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23JpsiKPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_JpsiKPhiPi_Dspm")
ToolSvc += BPHY23Rev2_JpsiKPhiPi_Dspm

#====================================================================
# Meson (-> phi pi) + Meson (-> phi pi) finder, for Ds+/D+ + Ds+/D+
#====================================================================
BPHY23Rev1_PhiPiPhiPi_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiPhiPi_Dpm",
    InputVtxContainerName      = "BPHY23PhiPiPhiPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiPhiPi_Dpm")
ToolSvc += BPHY23Rev1_PhiPiPhiPi_Dpm

BPHY23Rev1_PhiPiPhiPi_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev1_PhiPiPhiPi_Dspm",
    InputVtxContainerName      = "BPHY23PhiPiPhiPiCascadeVtx1",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev1_PhiPiPhiPi_Dspm")
ToolSvc += BPHY23Rev1_PhiPiPhiPi_Dspm

BPHY23Rev2_PhiPiPhiPi_Dpm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiPhiPi_Dpm",
    InputVtxContainerName      = "BPHY23PhiPiPhiPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dpmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiPhiPi_Dpm")
ToolSvc += BPHY23Rev2_PhiPiPhiPi_Dpm

BPHY23Rev2_PhiPiPhiPi_Dspm = DerivationFramework__ReVertex(
    name                       = "BPHY23Rev2_PhiPiPhiPi_Dspm",
    InputVtxContainerName      = "BPHY23PhiPiPhiPiCascadeVtx2",
    TrackIndices               = [ 0, 1, 2 ],
    RefitPV                    = True,
    RefPVContainerName         = "BPHY23PhiPiPhiPiRefPrimaryVertices", # use existing refitted PVs
    UseMassConstraint          = True,
    VertexMass                 = Dspmmass,
    MassInputParticles         = [Mumass, Mumass, Pimass],
    Chi2Cut                    = 200.,
    TrkVertexFitterTool        = BPHY23VertexFit,
    OutputVtxContainerName     = "BPHY23Rev2_PhiPiPhiPi_Dspm")
ToolSvc += BPHY23Rev2_PhiPiPhiPi_Dspm


CascadeCollections = []
CascadeCollections += BPHY23Jpsi2PiOnium.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKPiOnium.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKKOnium.CascadeVertexCollections
CascadeCollections += BPHY23Upsi2PiOnium.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiOnium.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKOnium.CascadeVertexCollections
CascadeCollections += BPHY23PhiPiOnium.CascadeVertexCollections
CascadeCollections += BPHY23Jpsi2PiJpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23Jpsi2PiJpsiKPi.CascadeVertexCollections
CascadeCollections += BPHY23Jpsi2PiJpsiKK.CascadeVertexCollections
CascadeCollections += BPHY23Jpsi2PiUpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKPiJpsiKPi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKPiJpsiKK.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKPiUpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKKJpsiKK.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKKUpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23Upsi2PiUpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiJpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiJpsiKPi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiJpsiKK.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiUpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKJpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKJpsiKPi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKJpsiKK.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKUpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23PhiPiJpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23PhiPiJpsiKPi.CascadeVertexCollections
CascadeCollections += BPHY23PhiPiJpsiKK.CascadeVertexCollections
CascadeCollections += BPHY23PhiPiUpsi2Pi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiJpsiPi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiJpsiK.CascadeVertexCollections
CascadeCollections += BPHY23JpsiPiPhiPi.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKJpsiK.CascadeVertexCollections
CascadeCollections += BPHY23JpsiKPhiPi.CascadeVertexCollections
CascadeCollections += BPHY23PhiPiPhiPi.CascadeVertexCollections

#--------------------------------------------------------------------
## 7/ select the event. We only want to keep events that contain certain vertices which passed certain selection.
##    This is specified by the "SelectionExpression" property, which contains the expression in the following format:
##
##       "ContainerName.passed_HypoName > count"
##
##    where "ContainerName" is output container from some Reco_* tool, "HypoName" is the hypothesis name setup in some "Select_*"
##    tool and "count" is the number of candidates passing the selection you want to keep. 

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY23_SelectEvent = DerivationFramework__xAODStringSkimmingTool(name = "BPHY23_SelectEvent", expression = "( count(BPHY23Jpsi2PiOniumCascadeVtx3.passed_Jpsi2PiOnium) + count(BPHY23JpsiKPiOniumCascadeVtx3.passed_JpsiKPiOnium) + count(BPHY23JpsiKKOniumCascadeVtx3.passed_JpsiKKOnium) + count(BPHY23Upsi2PiOniumCascadeVtx3.passed_Upsi2PiOnium) + count(BPHY23JpsiPiOniumCascadeVtx3.passed_JpsiPiOnium) + count(BPHY23JpsiKOniumCascadeVtx3.passed_JpsiKOnium) + count(BPHY23PhiPiOniumCascadeVtx3.passed_PhiPiOnium) + count(BPHY23Jpsi2PiJpsi2PiCascadeVtx3.passed_Jpsi2PiJpsi2Pi) + count(BPHY23Jpsi2PiJpsiKPiCascadeVtx3.passed_Jpsi2PiJpsiKPi) + count(BPHY23Jpsi2PiJpsiKKCascadeVtx3.passed_Jpsi2PiJpsiKK) + count(BPHY23Jpsi2PiUpsi2PiCascadeVtx3.passed_Jpsi2PiUpsi2Pi) + count(BPHY23JpsiKPiJpsiKPiCascadeVtx3.passed_JpsiKPiJpsiKPi) + count(BPHY23JpsiKPiJpsiKKCascadeVtx3.passed_JpsiKPiJpsiKK) + count(BPHY23JpsiKPiUpsi2PiCascadeVtx3.passed_JpsiKPiUpsi2Pi) + count(BPHY23JpsiKKJpsiKKCascadeVtx3.passed_JpsiKKJpsiKK) + count(BPHY23JpsiKKUpsi2PiCascadeVtx3.passed_JpsiKKUpsi2Pi) + count(BPHY23Upsi2PiUpsi2PiCascadeVtx3.passed_Upsi2PiUpsi2Pi) + count(BPHY23JpsiPiJpsi2PiCascadeVtx3.passed_JpsiPiJpsi2Pi) + count(BPHY23JpsiPiJpsiKPiCascadeVtx3.passed_JpsiPiJpsiKPi) + count(BPHY23JpsiPiJpsiKKCascadeVtx3.passed_JpsiPiJpsiKK) + count(BPHY23JpsiPiUpsi2PiCascadeVtx3.passed_JpsiPiUpsi2Pi) + count(BPHY23JpsiKJpsi2PiCascadeVtx3.passed_JpsiKJpsi2Pi) + count(BPHY23JpsiKJpsiKPiCascadeVtx3.passed_JpsiKJpsiKPi) + count(BPHY23JpsiKJpsiKKCascadeVtx3.passed_JpsiKJpsiKK) + count(BPHY23JpsiKUpsi2PiCascadeVtx3.passed_JpsiKUpsi2Pi) + count(BPHY23PhiPiJpsi2PiCascadeVtx3.passed_PhiPiJpsi2Pi) + count(BPHY23PhiPiJpsiKPiCascadeVtx3.passed_PhiPiJpsiKPi) + count(BPHY23PhiPiJpsiKKCascadeVtx3.passed_PhiPiJpsiKK) + count(BPHY23PhiPiUpsi2PiCascadeVtx3.passed_PhiPiUpsi2Pi) + count(BPHY23JpsiPiJpsiPiCascadeVtx3.passed_JpsiPiJpsiPi) + count(BPHY23JpsiPiJpsiKCascadeVtx3.passed_JpsiPiJpsiK) + count(BPHY23JpsiPiPhiPiCascadeVtx3.passed_JpsiPiPhiPi) + count(BPHY23JpsiKJpsiKCascadeVtx3.passed_JpsiKJpsiK) + count(BPHY23JpsiKPhiPiCascadeVtx3.passed_JpsiKPhiPi) + count(BPHY23PhiPiPhiPiCascadeVtx3.passed_PhiPiPhiPi) ) > 0")

ToolSvc += BPHY23_SelectEvent
print BPHY23_SelectEvent

#--------------------------------------------------------------------
## 8/ track and vertex thinning. We want to remove all reconstructed secondary vertices
##    which hasn't passed any of the selections defined by (Select_*) tools.
##    We also want to keep only tracks which are associates with either muons or any of the
##    vertices that passed the selection. Multiple thinning tools can perform the 
##    selection. The final thinning decision is based OR of all the decisions (by default,
##    although it can be changed by the JO).

## a) thining out vertices that didn't pass any selection and idetifying tracks associated with 
##    selected vertices. The "VertexContainerNames" is a list of the vertex containers, and "PassFlags"
##    contains all pass flags for Select_* tools that must be satisfied. The vertex is kept is it 
##    satisfy any of the listed selections.


#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================
## 9/ IMPORTANT bit. Don't forget to pass the tools to the DerivationKernel! If you don't do that, they will not be 
##    be executed!


# The name of the kernel (BPHY23Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
augmentation_tools = [BPHY23_Reco_mumu, BPHY23FourTrackReco_JpsiTrkTrk, BPHY23FourTrackReco_UpsiTrkTrk, BPHY23ThreeTrackReco_JpsiTrk, BPHY23ThreeTrackReco_PhiTrk, BPHY23Select_Phi, BPHY23Select_Jpsi, BPHY23Select_Psi, BPHY23Select_Upsi, BPHY23Select_Upsi2S, BPHY23Select_MuMu, BPHY23Jpsi2PiOnium, BPHY23JpsiKPiOnium, BPHY23JpsiKKOnium, BPHY23Upsi2PiOnium, BPHY23JpsiPiOnium, BPHY23JpsiKOnium, BPHY23PhiPiOnium, BPHY23Jpsi2PiJpsi2Pi, BPHY23Jpsi2PiJpsiKPi, BPHY23Jpsi2PiJpsiKK, BPHY23Jpsi2PiUpsi2Pi, BPHY23JpsiKPiJpsiKPi, BPHY23JpsiKPiJpsiKK, BPHY23JpsiKPiUpsi2Pi, BPHY23JpsiKKJpsiKK, BPHY23JpsiKKUpsi2Pi, BPHY23Upsi2PiUpsi2Pi, BPHY23JpsiPiJpsi2Pi, BPHY23JpsiPiJpsiKPi, BPHY23JpsiPiJpsiKK, BPHY23JpsiPiUpsi2Pi, BPHY23JpsiKJpsi2Pi, BPHY23JpsiKJpsiKPi, BPHY23JpsiKJpsiKK, BPHY23JpsiKUpsi2Pi, BPHY23PhiPiJpsi2Pi, BPHY23PhiPiJpsiKPi, BPHY23PhiPiJpsiKK, BPHY23PhiPiUpsi2Pi, BPHY23JpsiPiJpsiPi, BPHY23JpsiPiJpsiK, BPHY23JpsiPiPhiPi, BPHY23JpsiKJpsiK, BPHY23JpsiKPhiPi, BPHY23PhiPiPhiPi, BPHY23Rev1_Jpsi2PiOnium_Psi, BPHY23Rev1_Jpsi2PiOnium_X3872, BPHY23Rev2_Jpsi2PiOnium_Jpsi, BPHY23Rev2_Jpsi2PiOnium_Psi, BPHY23Rev2_Jpsi2PiOnium_Upsi, BPHY23Rev2_Jpsi2PiOnium_Upsi2S, BPHY23Rev1_JpsiKPiOnium, BPHY23Rev2_JpsiKPiOnium_Jpsi, BPHY23Rev2_JpsiKPiOnium_Psi, BPHY23Rev2_JpsiKPiOnium_Upsi, BPHY23Rev2_JpsiKPiOnium_Upsi2S, BPHY23Rev1_JpsiKKOnium, BPHY23Rev2_JpsiKKOnium_Jpsi, BPHY23Rev2_JpsiKKOnium_Psi, BPHY23Rev2_JpsiKKOnium_Upsi, BPHY23Rev2_JpsiKKOnium_Upsi2S, BPHY23Rev1_Upsi2PiOnium, BPHY23Rev2_Upsi2PiOnium_Jpsi, BPHY23Rev2_Upsi2PiOnium_Psi, BPHY23Rev2_Upsi2PiOnium_Upsi, BPHY23Rev2_Upsi2PiOnium_Upsi2S, BPHY23Rev1_JpsiPiOnium, BPHY23Rev2_JpsiPiOnium_Jpsi, BPHY23Rev2_JpsiPiOnium_Psi, BPHY23Rev2_JpsiPiOnium_Upsi, BPHY23Rev2_JpsiPiOnium_Upsi2S, BPHY23Rev1_JpsiKOnium, BPHY23Rev2_JpsiKOnium_Jpsi, BPHY23Rev2_JpsiKOnium_Psi, BPHY23Rev2_JpsiKOnium_Upsi, BPHY23Rev2_JpsiKOnium_Upsi2S, BPHY23Rev1_PhiPiOnium_Dpm, BPHY23Rev1_PhiPiOnium_Dspm, BPHY23Rev2_PhiPiOnium_Jpsi, BPHY23Rev2_PhiPiOnium_Psi, BPHY23Rev2_PhiPiOnium_Upsi, BPHY23Rev2_PhiPiOnium_Upsi2S, BPHY23Rev1_Jpsi2PiJpsi2Pi_Psi, BPHY23Rev1_Jpsi2PiJpsi2Pi_X3872, BPHY23Rev2_Jpsi2PiJpsi2Pi_Psi, BPHY23Rev2_Jpsi2PiJpsi2Pi_X3872, BPHY23Rev1_Jpsi2PiJpsiKPi_Psi, BPHY23Rev1_Jpsi2PiJpsiKPi_X3872, BPHY23Rev2_Jpsi2PiJpsiKPi, BPHY23Rev1_Jpsi2PiJpsiKK_Psi, BPHY23Rev1_Jpsi2PiJpsiKK_X3872, BPHY23Rev2_Jpsi2PiJpsiKK, BPHY23Rev1_Jpsi2PiUpsi2Pi_Psi, BPHY23Rev1_Jpsi2PiUpsi2Pi_X3872, BPHY23Rev2_Jpsi2PiUpsi2Pi, BPHY23Rev1_JpsiKPiJpsiKPi, BPHY23Rev2_JpsiKPiJpsiKPi, BPHY23Rev1_JpsiKPiJpsiKK, BPHY23Rev2_JpsiKPiJpsiKK, BPHY23Rev1_JpsiKPiUpsi2Pi, BPHY23Rev2_JpsiKPiUpsi2Pi, BPHY23Rev1_JpsiKKJpsiKK, BPHY23Rev2_JpsiKKJpsiKK, BPHY23Rev1_JpsiKKUpsi2Pi, BPHY23Rev2_JpsiKKUpsi2Pi, BPHY23Rev1_Upsi2PiUpsi2Pi, BPHY23Rev2_Upsi2PiUpsi2Pi, BPHY23Rev1_JpsiPiJpsi2Pi, BPHY23Rev2_JpsiPiJpsi2Pi_Psi, BPHY23Rev2_JpsiPiJpsi2Pi_X3872, BPHY23Rev1_JpsiPiJpsiKPi, BPHY23Rev2_JpsiPiJpsiKPi, BPHY23Rev1_JpsiPiJpsiKK, BPHY23Rev2_JpsiPiJpsiKK, BPHY23Rev1_JpsiPiUpsi2Pi, BPHY23Rev2_JpsiPiUpsi2Pi, BPHY23Rev1_JpsiKJpsi2Pi, BPHY23Rev2_JpsiKJpsi2Pi_Psi, BPHY23Rev2_JpsiKJpsi2Pi_X3872, BPHY23Rev1_JpsiKJpsiKPi, BPHY23Rev2_JpsiKJpsiKPi, BPHY23Rev1_JpsiKJpsiKK, BPHY23Rev2_JpsiKJpsiKK, BPHY23Rev1_JpsiKUpsi2Pi, BPHY23Rev2_JpsiKUpsi2Pi, BPHY23Rev1_PhiPiJpsi2Pi_Dpm, BPHY23Rev1_PhiPiJpsi2Pi_Dspm, BPHY23Rev2_PhiPiJpsi2Pi_Psi, BPHY23Rev2_PhiPiJpsi2Pi_X3872, BPHY23Rev1_PhiPiJpsiKPi_Dpm, BPHY23Rev1_PhiPiJpsiKPi_Dspm, BPHY23Rev2_PhiPiJpsiKPi, BPHY23Rev1_PhiPiJpsiKK_Dpm, BPHY23Rev1_PhiPiJpsiKK_Dspm, BPHY23Rev2_PhiPiJpsiKK, BPHY23Rev1_PhiPiUpsi2Pi_Dpm, BPHY23Rev1_PhiPiUpsi2Pi_Dspm, BPHY23Rev2_PhiPiUpsi2Pi, BPHY23Rev1_JpsiPiJpsiPi, BPHY23Rev2_JpsiPiJpsiPi, BPHY23Rev1_JpsiPiJpsiK, BPHY23Rev2_JpsiPiJpsiK, BPHY23Rev1_JpsiPiPhiPi, BPHY23Rev2_JpsiPiPhiPi_Dpm, BPHY23Rev2_JpsiPiPhiPi_Dspm, BPHY23Rev1_JpsiKJpsiK, BPHY23Rev2_JpsiKJpsiK, BPHY23Rev1_JpsiKPhiPi, BPHY23Rev2_JpsiKPhiPi_Dpm, BPHY23Rev2_JpsiKPhiPi_Dspm, BPHY23Rev1_PhiPiPhiPi_Dpm, BPHY23Rev1_PhiPiPhiPi_Dspm, BPHY23Rev2_PhiPiPhiPi_Dpm, BPHY23Rev2_PhiPiPhiPi_Dspm]

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
BPHY23_StaticContent += ["xAOD::VertexContainer#BPHY23Jpsi2PiOniumRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKPiOniumRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKKOniumRefPrimaryVertices", "xAOD::VertexContainer#BPHY23Upsi2PiOniumRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiOniumRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKOniumRefPrimaryVertices", "xAOD::VertexContainer#BPHY23PhiPiOniumRefPrimaryVertices", "xAOD::VertexContainer#BPHY23Jpsi2PiJpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23Jpsi2PiJpsiKPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23Jpsi2PiJpsiKKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23Jpsi2PiUpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKPiJpsiKPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKPiJpsiKKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKPiUpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKKJpsiKKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKKUpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23Upsi2PiUpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiJpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiJpsiKPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiJpsiKKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiUpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKJpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKJpsiKPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKJpsiKKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKUpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23PhiPiJpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23PhiPiJpsiKPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23PhiPiJpsiKKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23PhiPiUpsi2PiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiJpsiPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiJpsiKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiPiPhiPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKJpsiKRefPrimaryVertices", "xAOD::VertexContainer#BPHY23JpsiKPhiPiRefPrimaryVertices", "xAOD::VertexContainer#BPHY23PhiPiPhiPiRefPrimaryVertices"]
BPHY23_StaticContent += ["xAOD::VertexAuxContainer#BPHY23Jpsi2PiOniumRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKPiOniumRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKKOniumRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23Upsi2PiOniumRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiOniumRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKOniumRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23PhiPiOniumRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23Jpsi2PiJpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23Jpsi2PiJpsiKPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23Jpsi2PiJpsiKKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23Jpsi2PiUpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKPiJpsiKPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKPiJpsiKKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKPiUpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKKJpsiKKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKKUpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23Upsi2PiUpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiJpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiJpsiKPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiJpsiKKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiUpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKJpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKJpsiKPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKJpsiKKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKUpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23PhiPiJpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23PhiPiJpsiKPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23PhiPiJpsiKKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23PhiPiUpsi2PiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiJpsiPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiJpsiKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiPiPhiPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKJpsiKRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23JpsiKPhiPiRefPrimaryVerticesAux.", "xAOD::VertexAuxContainer#BPHY23PhiPiPhiPiRefPrimaryVerticesAux."]

## ID track particles
BPHY23_AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles 
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are stored in InDetTrackParticles collection)
BPHY23_AllVariables += ["CombinedMuonTrackParticles"]
BPHY23_AllVariables += ["ExtrapolatedMuonTrackParticles"]

## muon container
BPHY23_AllVariables += ["Muons"]
BPHY23_AllVariables += ["MuonSegments"]

## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
for cascade in CascadeCollections:
   BPHY23_StaticContent += ["xAOD::VertexContainer#%s" % cascade]
   BPHY23_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascade]

revertex_tools = [BPHY23Rev1_Jpsi2PiOnium_Psi, BPHY23Rev1_Jpsi2PiOnium_X3872, BPHY23Rev2_Jpsi2PiOnium_Jpsi, BPHY23Rev2_Jpsi2PiOnium_Psi, BPHY23Rev2_Jpsi2PiOnium_Upsi, BPHY23Rev2_Jpsi2PiOnium_Upsi2S, BPHY23Rev1_JpsiKPiOnium, BPHY23Rev2_JpsiKPiOnium_Jpsi, BPHY23Rev2_JpsiKPiOnium_Psi, BPHY23Rev2_JpsiKPiOnium_Upsi, BPHY23Rev2_JpsiKPiOnium_Upsi2S, BPHY23Rev1_JpsiKKOnium, BPHY23Rev2_JpsiKKOnium_Jpsi, BPHY23Rev2_JpsiKKOnium_Psi, BPHY23Rev2_JpsiKKOnium_Upsi, BPHY23Rev2_JpsiKKOnium_Upsi2S, BPHY23Rev1_Upsi2PiOnium, BPHY23Rev2_Upsi2PiOnium_Jpsi, BPHY23Rev2_Upsi2PiOnium_Psi, BPHY23Rev2_Upsi2PiOnium_Upsi, BPHY23Rev2_Upsi2PiOnium_Upsi2S, BPHY23Rev1_JpsiPiOnium, BPHY23Rev2_JpsiPiOnium_Jpsi, BPHY23Rev2_JpsiPiOnium_Psi, BPHY23Rev2_JpsiPiOnium_Upsi, BPHY23Rev2_JpsiPiOnium_Upsi2S, BPHY23Rev1_JpsiKOnium, BPHY23Rev2_JpsiKOnium_Jpsi, BPHY23Rev2_JpsiKOnium_Psi, BPHY23Rev2_JpsiKOnium_Upsi, BPHY23Rev2_JpsiKOnium_Upsi2S, BPHY23Rev1_PhiPiOnium_Dpm, BPHY23Rev1_PhiPiOnium_Dspm, BPHY23Rev2_PhiPiOnium_Jpsi, BPHY23Rev2_PhiPiOnium_Psi, BPHY23Rev2_PhiPiOnium_Upsi, BPHY23Rev2_PhiPiOnium_Upsi2S, BPHY23Rev1_Jpsi2PiJpsi2Pi_Psi, BPHY23Rev1_Jpsi2PiJpsi2Pi_X3872, BPHY23Rev2_Jpsi2PiJpsi2Pi_Psi, BPHY23Rev2_Jpsi2PiJpsi2Pi_X3872, BPHY23Rev1_Jpsi2PiJpsiKPi_Psi, BPHY23Rev1_Jpsi2PiJpsiKPi_X3872, BPHY23Rev2_Jpsi2PiJpsiKPi, BPHY23Rev1_Jpsi2PiJpsiKK_Psi, BPHY23Rev1_Jpsi2PiJpsiKK_X3872, BPHY23Rev2_Jpsi2PiJpsiKK, BPHY23Rev1_Jpsi2PiUpsi2Pi_Psi, BPHY23Rev1_Jpsi2PiUpsi2Pi_X3872, BPHY23Rev2_Jpsi2PiUpsi2Pi, BPHY23Rev1_JpsiKPiJpsiKPi, BPHY23Rev2_JpsiKPiJpsiKPi, BPHY23Rev1_JpsiKPiJpsiKK, BPHY23Rev2_JpsiKPiJpsiKK, BPHY23Rev1_JpsiKPiUpsi2Pi, BPHY23Rev2_JpsiKPiUpsi2Pi, BPHY23Rev1_JpsiKKJpsiKK, BPHY23Rev2_JpsiKKJpsiKK, BPHY23Rev1_JpsiKKUpsi2Pi, BPHY23Rev2_JpsiKKUpsi2Pi, BPHY23Rev1_Upsi2PiUpsi2Pi, BPHY23Rev2_Upsi2PiUpsi2Pi, BPHY23Rev1_JpsiPiJpsi2Pi, BPHY23Rev2_JpsiPiJpsi2Pi_Psi, BPHY23Rev2_JpsiPiJpsi2Pi_X3872, BPHY23Rev1_JpsiPiJpsiKPi, BPHY23Rev2_JpsiPiJpsiKPi, BPHY23Rev1_JpsiPiJpsiKK, BPHY23Rev2_JpsiPiJpsiKK, BPHY23Rev1_JpsiPiUpsi2Pi, BPHY23Rev2_JpsiPiUpsi2Pi, BPHY23Rev1_JpsiKJpsi2Pi, BPHY23Rev2_JpsiKJpsi2Pi_Psi, BPHY23Rev2_JpsiKJpsi2Pi_X3872, BPHY23Rev1_JpsiKJpsiKPi, BPHY23Rev2_JpsiKJpsiKPi, BPHY23Rev1_JpsiKJpsiKK, BPHY23Rev2_JpsiKJpsiKK, BPHY23Rev1_JpsiKUpsi2Pi, BPHY23Rev2_JpsiKUpsi2Pi, BPHY23Rev1_PhiPiJpsi2Pi_Dpm, BPHY23Rev1_PhiPiJpsi2Pi_Dspm, BPHY23Rev2_PhiPiJpsi2Pi_Psi, BPHY23Rev2_PhiPiJpsi2Pi_X3872, BPHY23Rev1_PhiPiJpsiKPi_Dpm, BPHY23Rev1_PhiPiJpsiKPi_Dspm, BPHY23Rev2_PhiPiJpsiKPi, BPHY23Rev1_PhiPiJpsiKK_Dpm, BPHY23Rev1_PhiPiJpsiKK_Dspm, BPHY23Rev2_PhiPiJpsiKK, BPHY23Rev1_PhiPiUpsi2Pi_Dpm, BPHY23Rev1_PhiPiUpsi2Pi_Dspm, BPHY23Rev2_PhiPiUpsi2Pi, BPHY23Rev1_JpsiPiJpsiPi, BPHY23Rev2_JpsiPiJpsiPi, BPHY23Rev1_JpsiPiJpsiK, BPHY23Rev2_JpsiPiJpsiK, BPHY23Rev1_JpsiPiPhiPi, BPHY23Rev2_JpsiPiPhiPi_Dpm, BPHY23Rev2_JpsiPiPhiPi_Dspm, BPHY23Rev1_JpsiKJpsiK, BPHY23Rev2_JpsiKJpsiK, BPHY23Rev1_JpsiKPhiPi, BPHY23Rev2_JpsiKPhiPi_Dpm, BPHY23Rev2_JpsiKPhiPi_Dspm, BPHY23Rev1_PhiPiPhiPi_Dpm, BPHY23Rev1_PhiPiPhiPi_Dspm, BPHY23Rev2_PhiPiPhiPi_Dpm, BPHY23Rev2_PhiPiPhiPi_Dspm]

for rev_tool in revertex_tools:
   BPHY23_StaticContent += ["xAOD::VertexContainer#%s" % rev_tool.OutputVtxContainerName]
   BPHY23_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % rev_tool.OutputVtxContainerName]

# Truth information for MC only
if isSimulation:
    BPHY23_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

BPHY23SlimmingHelper.AllVariables = BPHY23_AllVariables
BPHY23SlimmingHelper.StaticContent = BPHY23_StaticContent
BPHY23SlimmingHelper.AppendContentToStream(BPHY23Stream)
