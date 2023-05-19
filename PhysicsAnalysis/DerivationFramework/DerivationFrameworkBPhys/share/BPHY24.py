# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# BPHY24.py (use reductionConf flag BPHY24 in Reco_tf.py)
# Semileptonic rare b -> s/d l+l- decays:
#  - Bd -> K_S ll
#  - TODO: Rewrite to python3-valid code (namely printing)
#==============================================================================

# General scheme:
#  - list of triggers: two options: any trigger and a complied list
#  - di-muon vertex
#  - B-vertex
#  - Redone K* vertices
#  - include both Bd and \bar{Bd}
#  - decorations
#  - MC truth
#  - slimming / thinning / slimming
#  - keep all tracks without covariances (finders may try to remove used signal tracks !)
#  - keep all PV without covariances
#  - keep all muons without covariances

# Lists for better code organization
augsList          = [] # List of active augmentation tools
skimList          = [] # List of active skimming algorithms
thinList          = [] # List of active thinning algorithms
outVtxList        = [] # List of reconstructed candidates to store
outRePVList       = [] # List of candidates holding refitted primary vertices
thinTrkVtxList    = [] # List of reconstructed candidates to use for the thinning of tracks from vertices
thinPassFlagsList = [] # List of pass-flags in the reconstructed candidates to se for the thinning
finalCandidateList = []


#------------------------------------------------------------------------------
# Common services
#  - DONE: Test isSimulation flag
#------------------------------------------------------------------------------

from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkJob
from DerivationFrameworkCore.DerivationFrameworkMaster import buildFileName
from InDetRecExample import TrackingCommon
from pprint import pprint

#------------------------------------------------------------------------------
# Metadata for this derivation settings
#  - TODO: Test the metadata are stored and work
#------------------------------------------------------------------------------

# # Set up specific metadata configuration tool
# Wrong process but large amounts of overlap
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__BdKstarMuMu_metadata
BPHY24_MetaData = DerivationFramework__BdKstarMuMu_metadata( name              = "BPHY24_Metadata",
                                                             DerivationName    = "BPHY24",
                                                             version           = "v0.9", # derivation version (update with every update of the derivation)
                                                             verbose           = 1,      # TODO: verbose output (currently by default)
                                                             looseCuts         = False,  # TODO: apply loose cuts (debugging only)
                                                             skimTrig          = False,  # skim data by selected triggers
                                                             skimData          = True,   # skim data by passed B-candidates
                                                             thinData          = True,   # thin ID tracks, muons and PVs
                                                             slimData          = False,  # TODO: data slimming
                                                             thinMC            = True,   # thin MC-truth (keep wide range of heavy hadrons)
                                                             thinMCsignal      = False,  # thin MC-truth to signal-only (keep only signal PDG b-hadrons)
                                                             trigObjects       = True )  # store trigger objects for B-physics and muons

# # Local shorthand and ensure default contents of __slots__ dict are available as attributes
from DerivationFrameworkBPhys.BPhysPyHelpers import BPhysEnsureAttributes
BPHY24cf = BPhysEnsureAttributes(BPHY24_MetaData)

# print '********** BPHY24 Default Metadata **********'
ToolSvc += BPHY24_MetaData


# Data or simulation?
isSimulation = False
if globalflags.DataSource()=='geant4':
    isSimulation = True
BPHY24cf.isSimulation = isSimulation

# Project tag
BPHY24cf.projectTag = rec.projectName()

# Trigger stream name
from RecExConfig.InputFilePeeker import inputFileSummary
if inputFileSummary is not None:
  BPHY24cf.triggerStream = inputFileSummary['tag_info']['triggerStreamOfFile']

# Release 21 or newer?
from PyJobTransforms.trfUtils import releaseIsOlderThan
BPHY24cf.isRelease21 = not releaseIsOlderThan(21,0)

# MC campaigns by MC run number (keep in sync with BPHY8)
BPHY24MCcampaigns = { 284500 : 'mc16a',
                      300000 : 'mc16d',
                      310000 : 'mc16e' }

# Run number and MC campaign by MC run number
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
import PyUtils.AthFile as BPHY24_af
BPHY24_f = BPHY24_af.fopen(athenaCommonFlags.PoolAODInput()[0])
BPHY24cf.mcCampaign = 'unknown'
if len(BPHY24_f.run_numbers) > 0:
  BPHY24cf.runNumber = int(BPHY24_f.run_numbers[0])
  if BPHY24cf.isSimulation and BPHY24cf.runNumber in BPHY24MCcampaigns:
    BPHY24cf.mcCampaign = BPHY24MCcampaigns[BPHY24cf.runNumber]

pprint( BPHY24_MetaData.properties() )
print('********** BPHY24 Default Metadata (end) **********')



#------------------------------------------------------------------------------
# Trigger counting metadata
#  - TODO: Test metadata
#------------------------------------------------------------------------------

triggers2Metadata = [ # Pure muon triggers
"HLT_mu11_mu6_bDimu",
"HLT_mu11_mu6_bDimu2700",
"HLT_mu11_mu6_bDimu_L1LFV-MU11",
"HLT_mu11_mu6_bDimu2700_L1LFV-MU11",
"HLT_mu11_mu6_bJpsimumu",
"HLT_2mu10_bJpsimumu",
"HLT_mu11_mu6_bJpsimumu_L1LFV-MU11",
"HLT_2mu6_bJpsimumu_L1BPH-2M9-2MU6_BPH-2DR15-2MU6",
"HLT_2mu6_bJpsimumu_delayed_L1BPH-2M9-2MU6_BPH-2DR15-2MU6",
"HLT_2mu10_bJpsimumu_noL2",
"HLT_mu10_mu6_bJpsimumu",
"HLT_mu10_mu6_bJpsimumu_delayed",
"HLT_2mu6_bJpsimumu",
"HLT_2mu6_bJpsimumu_delayed",
"HLT_mu6_2mu4_bJpsi",
"HLT_mu6_2mu4_bJpsi_delayed",
"HLT_2mu14",
"HLT_2mu10",
                      # dielectron triggers
"HLT_2e5_lhvloose_nod0_bBeexM6000t",  #37,143,877  inb
"HLT_e5_lhvloose_nod0_bBeexM6000t",  #37,143,877
"HLT_e5_lhvloose_nod0_bBeexM6000t_2mu4_nomucomb_L1BPH-0DR3-EM7J15_2MU4",   #37,312,506
"HLT_e5_lhvloose_nod0_bBeexM6000t_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #27,041,892
"HLT_e5_lhvloose_nod0_bBeexM6000_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #149,100	
"HLT_e9_lhloose_bBeexM2700_2mu4_nomucomb_L1BPH-0DR3-EM7J15_2MU4",   #2,681,764
"HLT_e9_lhloose_bBeexM2700_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #1,979,362
"HLT_e9_lhloose_bBeexM6000_2mu4_nomucomb_L1BPH-0DR3-EM7J15_2MU4",   #3,359,105
"HLT_e9_lhloose_bBeexM6000_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #2,426,663
"HLT_e9_lhloose_e5_lhloose_bBeexM2700_2mu4_nomucomb_L1BPH-0M9-EM7-EM5_2MU4",   #2,950,935
"HLT_e9_lhloose_e5_lhloose_bBeexM2700_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #2,928,030
"HLT_e9_lhloose_e5_lhloose_bBeexM6000_2mu4_nomucomb_L1BPH-0M9-EM7-EM5_2MU4",   #3,647,507
"HLT_e9_lhloose_e5_lhloose_bBeexM6000_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #3,605,371
"HLT_e9_lhvloose_nod0_e5_lhvloose_nod0_bBeexM6000t_2mu4_nomucomb_L1BPH-0M9-EM7-EM5_2MU4",   #40,169,436
"HLT_e9_lhvloose_nod0_e5_lhvloose_nod0_bBeexM6000t_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #37,312,506
"HLT_e9_lhvloose_nod0_e5_lhvloose_nod0_bBeexM6000_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #677,340
                       ]

#from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__TriggerCountToMetadata
#BPHY24_TrigCountMetadata = DerivationFramework__TriggerCountToMetadata( name        = "BPHY24_TrigCountMetadata",
#                                                                        TriggerList = triggers2Metadata,
#                                                                        FolderName  = "BPHY24" )
#print('********** BPHY24 Trigger Counts Metadata **********')
#ToolSvc += BPHY24_TrigCountMetadata
#pprint   ( BPHY24_TrigCountMetadata.properties() )
#print('********** BPHY24 Trigger Counts Metadata (end) **********')


streamName   = derivationFlags.WriteDAOD_BPHY24Stream.StreamName
#------------------------------------------------------------------------------
# Trigger skimming
#  - Partly DONE: Test that skimming works (not all MC events accepted if active, TODO: Exactly test passed triggers)
#------------------------------------------------------------------------------

if BPHY24cf.skimTrig:
  triggerList_unseeded = ["HLT_2e5_lhvloose_nod0_bBeexM6000t",  #37,143,877  inb
                        "HLT_e5_lhvloose_nod0_bBeexM6000t"  #37,143,877
  ]
  triggersSkim = [_ for _ in triggers2Metadata if _ not in triggerList_unseeded]

  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
  BPHY24_Skim_Trig = DerivationFramework__TriggerSkimmingTool( name           = "BPHY24_Skim_Trig",
                                                               TriggerListOR  = triggersSkim,
                                                               TriggerListAND = [],
                                                               TriggerListORHLTOnly = triggerList_unseeded )
  print('********** BPHY24 Trigger Skimming **********')
  skimList += [ BPHY24_Skim_Trig ]
  ToolSvc  +=   BPHY24_Skim_Trig
  pprint      ( BPHY24_Skim_Trig.properties() )
  print('********** BPHY24 Trigger Skimming (end) **********')



#------------------------------------------------------------------------------
# Vertexing tools and services
#------------------------------------------------------------------------------

include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY24_VertexTools = BPHYVertexTools("BPHY24")

print('********** BPHY24 Vertex Tools **********')
print(BPHY24_VertexTools)
print(BPHY24_VertexTools.TrkV0Fitter)
print('********** BPHY24 Vertex Tools (end) **********')



#------------------------------------------------------------------------------
# Augment orignal counts of vertices and tracks ?
#  - DONE: Test newly agumented data
#  - TODO: Possibly remove if case basic info on all tracks and vertices is kept
#------------------------------------------------------------------------------

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__AugOriginalCounts
BPHY24_AugOriginalCounts = DerivationFramework__AugOriginalCounts( name            = "BPHY24_AugOriginalCounts",
                                                                   VertexContainer = "PrimaryVertices",
                                                                   TrackContainer  = "InDetTrackParticles" )
print('********** BPHY24 Augment Original Counts **********')
augsList += [ BPHY24_AugOriginalCounts ]
ToolSvc  +=   BPHY24_AugOriginalCounts
pprint      ( BPHY24_AugOriginalCounts.properties() )
print('********** BPHY24 Augment Original Counts (end) **********')



#------------------------------------------------------------------------------
# Di-muon vertex finder
#  - TODO: Make sure these are all options needed and check defaults
#  - TODO: Start invariant mass at zero? (check size enlargement)
#  - TODO: Rename HypothesisName in BPHY24_Select_DiMuons to better match reality?
#  - TODO: Can Select_onia2mumu be forced to use reconstruced mass for the pseudo-proper decay time etc. calculations?
#------------------------------------------------------------------------------

from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY24_Finder_DiMuon = Analysis__JpsiFinder( name                        = "BPHY24_Finder_DiMuon",
                                             muAndMu                     = True,
                                             muAndTrack                  = False,
                                             TrackAndTrack               = False,
                                             assumeDiMuons               = True,
                                             muonThresholdPt             = 3000.,
                                             higherPt                    = 3500.,
                                             invMassUpper                = 7000.,
                                             invMassLower                = 1.,
                                             Chi2Cut                     = 30.,
                                             oppChargesOnly              = False,
                                             allChargeCombinations       = True,
                                             atLeastOneComb              = True,
                                             useCombinedMeasurement      = False, # Only takes effect if combOnly=True
                                             muonCollectionKey           = "Muons",
                                             TrackParticleCollection     = "InDetTrackParticles",
                                             V0VertexFitterTool          = BPHY24_VertexTools.TrkV0Fitter,
                                             useV0Fitter                 = False,
                                             TrkVertexFitterTool         = BPHY24_VertexTools.TrkVKalVrtFitter,
                                             TrackSelectorTool           = BPHY24_VertexTools.InDetTrackSelectorTool,
                                             VertexPointEstimator        = BPHY24_VertexTools.VtxPointEstimator,
                                             useMCPCuts                  = False )
print('********** BPHY24 DiMuon Finder **********')
ToolSvc += BPHY24_Finder_DiMuon
pprint   ( BPHY24_Finder_DiMuon.properties() )
print('********** BPHY24 DiMuon Finder (end) **********')

# Select and write the di-muon candidates, do no do PV refit
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY24_SelectAndWrite_DiMuon = DerivationFramework__Reco_mumu( name                     = "BPHY24_SelectAndWrite_DiMuon",
                                                               JpsiFinder               = BPHY24_Finder_DiMuon,
                                                               V0Tools                  = TrackingCommon.getV0Tools(),
                                                               PVRefitter               = BPHY24_VertexTools.PrimaryVertexRefitter,
                                                               OutputVtxContainerName   = "BPHY24_DiMuon_Candidates",
                                                               PVContainerName          = "PrimaryVertices",
                                                               RefPVContainerName       = "SHOULDNOTBEUSED", # The container would be created if PV refit was requested (not needed at this point)
                                                               DoVertexType             = 7 ) # Vertex type marking our own reconstruced secondary candidates
print('********** BPHY24 DiMuon Writer **********')
augsList += [ BPHY24_SelectAndWrite_DiMuon ]
ToolSvc  +=   BPHY24_SelectAndWrite_DiMuon
pprint      ( BPHY24_SelectAndWrite_DiMuon.properties() )
print('********** BPHY24 DiMuon Writer (end) **********')

# Final selection of the di-muon candidates
thinTrkVtxList    += [ "BPHY24_DiMuon_Candidates" ]
outVtxList        += [ "BPHY24_DiMuon_Candidates" ]
thinPassFlagsList += [ "passed_Jpsi" ] # TODO: is this really needed?
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu
BPHY24_Select_DiMuons = DerivationFramework__Select_onia2mumu( name                  = "BPHY24_Select_DiMuons",
                                                               HypothesisName        = "Jpsi", # should be changed ?
                                                               InputVtxContainerName = "BPHY24_DiMuon_Candidates",
                                                               V0Tools               = TrackingCommon.getV0Tools(),
                                                               VtxMassHypo           = 3096.916, # used only for pseudo-proper decay time etc. calculations
                                                               MassMax               = 10000., # loose cut to keep selection from BPHY24_Finder_DiMuon
                                                               MassMin               = 0.,     # loose cut to keep selection from BPHY24_Finder_DiMuon
                                                               Chi2Max               = 1000.,  # loose cut to keep selection from BPHY24_Finder_DiMuon (chi2, not chi2/NDF)
                                                               DoVertexType          = 7 ) # Vertex type marking our own reconstruced secondary candidates
print('********** BPHY24 DiMuon Selector **********')
augsList += [ BPHY24_Select_DiMuons ]
ToolSvc  +=   BPHY24_Select_DiMuons
pprint      ( BPHY24_Select_DiMuons.properties() )
print('********** BPHY24 DiMuon Selector (end) **********')


##### ELECTRONS
#lhvloose_nod0
from ElectronPhotonSelectorTools.ElectronPhotonSelectorToolsConf import AsgElectronLikelihoodTool
ElectronLHSelectorLHvloose_nod0 = AsgElectronLikelihoodTool("ElectronLHSelectorLHvloosenod0", 
ConfigFile="ElectronPhotonSelectorTools/offline/mc16_20190328_nod0/ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth_nod0.conf")
ElectronLHSelectorLHvloose_nod0.primaryVertexContainer = "PrimaryVertices"
ToolSvc += ElectronLHSelectorLHvloose_nod0
print(ElectronLHSelectorLHvloose_nod0)

from ROOT import LikeEnum
# decorate electrons with the output of LH vloose nod0
from DerivationFrameworkEGamma.DerivationFrameworkEGammaConf import DerivationFramework__EGElectronLikelihoodToolWrapper
from DerivationFrameworkEGamma.DerivationFrameworkEGammaConf import DerivationFramework__EGSelectionToolWrapper
from ElectronPhotonSelectorTools.ConfiguredAsgElectronLikelihoodTools import ConfiguredAsgElectronLikelihoodTool

ElectronLHSelectorVeryLoose = ConfiguredAsgElectronLikelihoodTool("ElectronLHSelectorVeryLoose", LikeEnum.VeryLoose)
ElectronLHSelectorVeryLoose.primaryVertexContainer = "PrimaryVertices"
ToolSvc += ElectronLHSelectorVeryLoose


ElectronPassLHVeryLoose = DerivationFramework__EGElectronLikelihoodToolWrapper( name = "ElectronPassLHVeryLoose",
                                                                       EGammaElectronLikelihoodTool = ElectronLHSelectorVeryLoose,
                                                                       EGammaFudgeMCTool = "",
                                                                       CutType = "",
                                                                       StoreGateEntryName = "DFCommonElectronsLHVeryLoose",
                                                                       ContainerName = "Electrons")
ToolSvc += ElectronPassLHVeryLoose

ElectronPassLHvloosenod0 = DerivationFramework__EGElectronLikelihoodToolWrapper(name = "ElectronPassLHvloosenod0",
                                                                       EGammaElectronLikelihoodTool = ElectronLHSelectorLHvloose_nod0,
                                                                       EGammaFudgeMCTool = "",
                                                                       CutType = "",
                                                                       StoreGateEntryName = "DFCommonElectronsLHVeryLoosenod0",
                                                                       ContainerName = "Electrons")
augsList += [ElectronPassLHVeryLoose, ElectronPassLHvloosenod0]
ToolSvc += ElectronPassLHvloosenod0
print(ElectronPassLHvloosenod0)

#--------------------------------------------------------------------
## 2/ setup JpsiFinder tool
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder_ee
BPHY24DiElectronFinder = Analysis__JpsiFinder_ee(
    name                        = "BPHY24DiElectronFinder",
    elAndEl                     = True,
    elAndTrack                  = False,
    TrackAndTrack               = False,
    assumeDiElectrons           = True,
    elThresholdPt               = 4000.0,
    invMassUpper                = 7000.0,
    invMassLower                = 200.0,
    Chi2Cut                     = 30.,
    oppChargesOnly	            = False,
    allChargeCombinations       = True,
    useElectronTrackMeasurement = True,
    electronCollectionKey       = "Electrons",
    TrackParticleCollection     = "GSFTrackParticles",
    useEgammaCuts               = True,
    V0VertexFitterTool          = BPHY24_VertexTools.TrkV0Fitter,
    useV0Fitter                 = False,
    TrkVertexFitterTool         = BPHY24_VertexTools.TrkVKalVrtFitter,
    TrackSelectorTool           = BPHY24_VertexTools.InDetTrackSelectorTool,
    VertexPointEstimator        = BPHY24_VertexTools.VtxPointEstimator,
    ElectronSelection 		      = "d0_or_nod0"
    )

ToolSvc += BPHY24DiElectronFinder
print(BPHY24DiElectronFinder)

#--------------------------------------------------------------------
## 3/ setup the vertex reconstruction "call" tool(s).
BPHY24_SelectAndWrite_DiElectron = DerivationFramework__Reco_mumu(
    name                   = "BPHY24_SelectAndWrite_DiElectron",
    JpsiFinder             = BPHY24DiElectronFinder,
    V0Tools               = TrackingCommon.getV0Tools(),
    PVRefitter             = BPHY24_VertexTools.PrimaryVertexRefitter,
    OutputVtxContainerName = "BPHY24_DiElectron_Candidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    DoVertexType           = 7
    )

augsList += [ BPHY24_SelectAndWrite_DiElectron ]
ToolSvc += BPHY24_SelectAndWrite_DiElectron
pprint (BPHY24_SelectAndWrite_DiElectron)

BPHY24_Select_DiElectrons = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY24_Select_DiElectrons",
    HypothesisName        = "Jpsi",
    InputVtxContainerName = "BPHY24_DiElectron_Candidates",
    V0Tools               = TrackingCommon.getV0Tools(),
    VtxMassHypo           = 3096.916,
    MassMin               = 400.0,
    MassMax               = 7000.0,
    Chi2Max               = 30,
    DoVertexType          = 7
    )

thinTrkVtxList    += [ "BPHY24_DiElectron_Candidates" ]
outVtxList        += [ "BPHY24_DiElectron_Candidates" ]
 
augsList += [ BPHY24_Select_DiElectrons ]
ToolSvc += BPHY24_Select_DiElectrons
pprint      (BPHY24_Select_DiElectrons)


### Find Kshorts
include("DerivationFrameworkBPhys/configureV0Finder.py")

BPHY24_V0FinderTools = BPHYV0FinderTools("BPHY24")
BPHY24_V0FinderTools.BPhysGammaFitter.FirstMeasuredPoint=False
BPHY24_V0FinderTools.V0FinderTool.V0ContainerName = "BPHY24RecoV0Candidates"
BPHY24_V0FinderTools.V0FinderTool.KshortContainerName = "BPHY24RecoKshortCandidates"
BPHY24_V0FinderTools.V0FinderTool.LambdaContainerName = "BPHY24RecoLambdaCandidates"
BPHY24_V0FinderTools.V0FinderTool.LambdabarContainerName = "BPHY24RecoLambdabarCandidates"
print(BPHY24_V0FinderTools)

## 8/ setup the cascade vertexing tool
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
JpsiV0VertexFit = Trk__TrkVKalVrtFitter(
    name                 = "JpsiV0VertexFit",
    Extrapolator         = BPHY24_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint   = False,
    CascadeCnstPrecision = 1e-6,
    MakeExtendedVertex   = True)

ToolSvc += JpsiV0VertexFit
print(JpsiV0VertexFit)

from InDetV0Finder.InDetV0FinderConf import InDet__V0MainDecorator
V0Decorator = InDet__V0MainDecorator(name = "BPHY24V0Decorator",
                                     V0Tools = TrackingCommon.getV0Tools(),
                                     V0ContainerName = "BPHY24RecoV0Candidates",
                                    KshortContainerName = "BPHY24RecoKshortCandidates",
                                    LambdaContainerName = "BPHY24RecoLambdaCandidates",
                                    LambdabarContainerName = "BPHY24RecoLambdabarCandidates")
ToolSvc += V0Decorator

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_V0Finder
BPHY24_Reco_V0Finder   = DerivationFramework__Reco_V0Finder(
    name                   = "BPHY24_Reco_V0Finder",
    V0FinderTool           = BPHY24_V0FinderTools.V0FinderTool,
    Decorator              = V0Decorator,
    V0ContainerName        = "BPHY24RecoV0Candidates",
    KshortContainerName    = "BPHY24RecoKshortCandidates",
    LambdaContainerName    = "BPHY24RecoLambdaCandidates",
    LambdabarContainerName = "BPHY24RecoLambdabarCandidates",
    CheckVertexContainers  = ['BPHY24_DiMuon_Candidates', 'BPHY24_DiElectron_Candidates'])

augsList += [BPHY24_Reco_V0Finder]
ToolSvc += BPHY24_Reco_V0Finder
print(BPHY24_Reco_V0Finder)

outVtxList += ['BPHY24RecoKshortCandidates']
thinTrkVtxList += ['BPHY24RecoKshortCandidates']
thinPassFlagsList += [ "" ] # TODO: is this really needed?
finalCandidateList += ["BPHY24RecoKshortCandidates"]

## 9/ setup the Jpsi+V0 finder
## a/ Bd->mmKshort
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiPlusV0Cascade
BPHY24JpsimmKshort            = DerivationFramework__JpsiPlusV0Cascade(
    name                    = "BPHY24mmKshort",
    V0Tools                 = TrackingCommon.getV0Tools(),
    HypothesisName          = "Bd",
    TrkVertexFitterTool     = JpsiV0VertexFit,
    V0Hypothesis            = 310,
    JpsiMassLowerCut        = 1.,
    JpsiMassUpperCut        = 7000.,
    ApplyJpsiMassConstraint = False,
    V0MassLowerCut          = 400.,
    V0MassUpperCut          = 600.,
    MassLowerCut            = 4300.,
    MassUpperCut            = 6300.,
    RefitPV                 = True,
    RefPVContainerName      = "BPHY24RefittedPrimaryVertices_mm",
    JpsiVertices            = "BPHY24_DiMuon_Candidates",
    CascadeVertexCollections= ["BPHY24JpsimmKshortCascadeSV2", "BPHY24JpsimmKshortCascadeSV1"],
    V0Vertices              = "BPHY24RecoV0Candidates")

augsList += [BPHY24JpsimmKshort]
ToolSvc += BPHY24JpsimmKshort
print(BPHY24JpsimmKshort)
outVtxList += BPHY24JpsimmKshort.CascadeVertexCollections
outVtxList += ["BPHY24RefittedPrimaryVertices_mm"]
thinTrkVtxList += BPHY24JpsimmKshort.CascadeVertexCollections
finalCandidateList += BPHY24JpsimmKshort.CascadeVertexCollections

## a/ Bd->eeKshort
BPHY24JpsieeKshort            = DerivationFramework__JpsiPlusV0Cascade(
    name                    = "BPHY24eeKshort",
    V0Tools                 = TrackingCommon.getV0Tools(),
    HypothesisName          = "Bd",
    TrkVertexFitterTool     = JpsiV0VertexFit,
    V0Hypothesis            = 310,
    JpsiMassLowerCut        = 100.,
    JpsiMassUpperCut        = 7000.,
    ApplyJpsiMassConstraint = False,
    V0MassLowerCut          = 400.,
    V0MassUpperCut          = 600.,
    MassLowerCut            = 4300.,
    MassUpperCut            = 6300.,
    JpsiTrackPDGID          = 11,
    JpsiTrackContainerName  = "GSFTrackParticles",
    RefitPV                 = True,
    RefPVContainerName      = "BPHY24RefittedPrimaryVertices_ee",
    JpsiVertices            = "BPHY24_DiElectron_Candidates",
    CascadeVertexCollections= ["BPHY24JpsieeKshortCascadeSV2", "BPHY24JpsieeKshortCascadeSV1"],
    V0Vertices              = "BPHY24RecoV0Candidates")

augsList += [BPHY24JpsieeKshort]
ToolSvc += BPHY24JpsieeKshort
print(BPHY24JpsieeKshort)
finalCandidateList += BPHY24JpsieeKshort.CascadeVertexCollections
outVtxList += BPHY24JpsieeKshort.CascadeVertexCollections
outVtxList += ["BPHY24RefittedPrimaryVertices_ee"]
thinTrkVtxList += BPHY24JpsieeKshort.CascadeVertexCollections


####################################################

#Track isolation for candidates

#get TTVA tool first (may or may not use this)
from IsolationTool.IsolationToolConf import xAOD__TrackIsolationTool
trackVertexAssoTool=CfgMgr.CP__TrackVertexAssociationTool("TrackVertexAssociationTool", WorkingPoint="Loose") 
ToolSvc+=trackVertexAssoTool 


from IsolationTool.IsolationToolConf import xAOD__TrackIsolationTool
TrackIsoTool = xAOD__TrackIsolationTool("TrackIsoTool")
TrackIsoTool.TrackSelectionTool.maxZ0SinTheta= 2
TrackIsoTool.TrackSelectionTool.minPt= 1000.
TrackIsoTool.TrackSelectionTool.CutLevel= "Loose"
ToolSvc += TrackIsoTool

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__VertexTrackIsolation
BPHY24TrackIsolationDecoratorBtoKee = DerivationFramework__VertexTrackIsolation(
  name                            = "BPHY24TrackIsolationDecoratorBtoKee",
  TrackIsoTool                    = TrackIsoTool,
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY24JpsieeKshortCascadeSV1",
  FixElecExclusion                = True,
  IncludeV0                       = True)

BPHY24TrackIsolationDecoratorBtoKmumu = DerivationFramework__VertexTrackIsolation(
  name                            = "BPHY24TrackIsolationDecoratorBtoKmumu ",
  TrackIsoTool                    = TrackIsoTool,
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY24JpsimmKshortCascadeSV1",
  FixElecExclusion                = False,
  IncludeV0                       = True)

BPHY24TrackIsolationDecoratorJpsiee = DerivationFramework__VertexTrackIsolation(
  name                            = "BPHY24TrackIsolationDecoratorJpsiee",
  TrackIsoTool                    = TrackIsoTool,
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY24_DiElectron_Candidates",
  FixElecExclusion                = True,
  IncludeV0                       = False)

BPHY24TrackIsolationDecoratorJpsimumu = DerivationFramework__VertexTrackIsolation(
  name                            = "BPHY24TrackIsolationDecoratorJpsimumu",
  TrackIsoTool                    = TrackIsoTool,
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY24_DiMuon_Candidates",
  FixElecExclusion                = False,
  IncludeV0                       = False)

BPHY24TrackIsolationDecoratorKshort = DerivationFramework__VertexTrackIsolation(
  name                            = "BPHY24TrackIsolationDecoratorKshort",
  TrackIsoTool                    = TrackIsoTool,
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY24RecoKshortCandidates",
  FixElecExclusion                = False,
  IncludeV0                       = False)

ToolSvc += BPHY24TrackIsolationDecoratorBtoKee
ToolSvc += BPHY24TrackIsolationDecoratorBtoKmumu
ToolSvc += BPHY24TrackIsolationDecoratorJpsiee
ToolSvc += BPHY24TrackIsolationDecoratorJpsimumu
ToolSvc += BPHY24TrackIsolationDecoratorKshort
augsList += [ BPHY24TrackIsolationDecoratorBtoKee,
              BPHY24TrackIsolationDecoratorBtoKmumu,
              BPHY24TrackIsolationDecoratorJpsiee,
              BPHY24TrackIsolationDecoratorJpsimumu,
              BPHY24TrackIsolationDecoratorKshort]
print(BPHY24TrackIsolationDecoratorBtoKee)
print(BPHY24TrackIsolationDecoratorBtoKmumu)
print(BPHY24TrackIsolationDecoratorJpsiee)
print(BPHY24TrackIsolationDecoratorJpsimumu)
print(BPHY24TrackIsolationDecoratorKshort)#######################################################

#------------------------------------------------------------------------------
# B-candidates skimming
#  - Partly DONE: test that skimming works (not all MC events are accepted if active, while number of B-candidates remains same)
#------------------------------------------------------------------------------

if BPHY24cf.skimData:

  # TODO: Include other semileptonic rare b-hadrons and use OR in the skimming (or just one large expression bSkim)
  bSkim = "(count(BPHY24JpsimmKshortCascadeSV1.Bd_mass) + count(BPHY24JpsieeKshortCascadeSV1.Bd_mass)) > 0";

  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
  BPHY24_Skim_Bcandidates = DerivationFramework__xAODStringSkimmingTool( name       = "BPHY24_Skim_Bcandidates",
                                                                         expression = bSkim )
  print('********** BPHY24 B-Candidates Skimming **********')
  skimList += [ BPHY24_Skim_Bcandidates ]
  ToolSvc  +=   BPHY24_Skim_Bcandidates
  pprint      ( BPHY24_Skim_Bcandidates.properties() )
  print('********** BPHY24 B-Candidates Skimming (end) **********')



#------------------------------------------------------------------------------
# Combine all skimming together (AND)
#  - TODO: Test that it works (combines two skimmings together) as expected
#------------------------------------------------------------------------------

if len(skimList) > 1:

  from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
  BPHY24_Skim = CfgMgr.DerivationFramework__FilterCombinationAND(name       = "BPHY24_Skim",
                                                                 FilterList = skimList )
  print('********** BPHY24 Combined Skimming **********')
  ToolSvc += BPHY24_Skim
  pprint   ( BPHY24_Skim.properties() )
  print('********** BPHY24 Combined Skimming (end) **********')

elif len(skimList) == 1:

  BPHY24_Skim = skimList[0]



#------------------------------------------------------------------------------
# Data thinning: tracks, muons, PV
#  - TODO: Test that each of the thinning works
#------------------------------------------------------------------------------

if BPHY24cf.thinData:

  # ID tracks
  from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Thin_vtxTrk
  BPHY24_Thin_VtxTracks = DerivationFramework__Thin_vtxTrk( name                       = "BPHY24_Thin_VtxTracks",
                                                            StreamName = streamName,
                                                            TrackParticleContainerName = "InDetTrackParticles",
                                                            VertexContainerNames       = finalCandidateList,
                                                            IgnoreFlags                = True )
                                                            # PassFlags                  = thinPassFlagsList )
  print('********** BPHY24 Track Thinning **********')
  thinList += [ BPHY24_Thin_VtxTracks ]
  ToolSvc  +=   BPHY24_Thin_VtxTracks
  pprint      ( BPHY24_Thin_VtxTracks.properties() )
  print('********** BPHY24 Track Thinning (end) **********')

  # GSF tracks
  BPHY24_Thin_VtxTracks_GSF = DerivationFramework__Thin_vtxTrk( name                   = "BPHY24_Thin_VtxTracks_GSF",
                                                            StreamName = streamName,
                                                            TrackParticleContainerName = "GSFTrackParticles",
                                                            VertexContainerNames       = finalCandidateList,
                                                            IgnoreFlags                = True )
  print('********** BPHY24 GSF Track Thinning **********')
  thinList += [ BPHY24_Thin_VtxTracks_GSF ]
  ToolSvc  +=   BPHY24_Thin_VtxTracks_GSF
  pprint      ( BPHY24_Thin_VtxTracks_GSF.properties() )
  print('********** BPHY24 GSF Track Thinning (end) **********')

  # Muons (TODO: thinning not used muons or something else ?)
  from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
  BPHY24_Thin_Muons = DerivationFramework__MuonTrackParticleThinning( name                   = "BPHY24_Thin_Muons",
                                                                      MuonKey                = "Muons",
                                                                      StreamName = streamName,
                                                                      InDetTrackParticlesKey = "InDetTrackParticles" )
  print('********** BPHY24 Muon Thinning **********')
  thinList += [ BPHY24_Thin_Muons ]
  ToolSvc  +=   BPHY24_Thin_Muons
  pprint      ( BPHY24_Thin_Muons.properties() )
  print('********** BPHY24 Muon Thinning (end) **********')

  # Electrons
  from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
  BPHY24_Thin_Egamma = DerivationFramework__EgammaTrackParticleThinning(
      name                   = "BPHY24_Thin_Egamma",
      SGKey                  = "Electrons",
      StreamName = streamName,
      InDetTrackParticlesKey = "InDetTrackParticles")
  thinList += [BPHY24_Thin_Egamma]
  ToolSvc += BPHY24_Thin_Egamma
  pprint      ( BPHY24_Thin_Egamma.properties() )

  # Primary vertices
  from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__BPhysPVThinningTool
  BPHY24_Thin_PV = DerivationFramework__BPhysPVThinningTool( name                 = "BPHY24_Thin_PV",
                                                             CandidateCollections = finalCandidateList,
                                                             StreamName = streamName,
                                                             KeepPVTracks         = True )
  print('********** BPHY24 Primary Vertices Thinning **********')
  thinList += [ BPHY24_Thin_PV ]
  ToolSvc  +=   BPHY24_Thin_PV
  pprint      ( BPHY24_Thin_PV.properties() )
  print('********** BPHY24 Primary Vertices Thinning (end) **********')



#------------------------------------------------------------------------------
# MC thinning: only same b/c-meson decays and quarks
#  - TODO: Test that each of the thinning works (seems OK in log-files, but not in the size of the MC containers ! Try to get inspired by BPHY8)
#------------------------------------------------------------------------------

if BPHY24cf.isSimulation and (BPHY24cf.thinMC or BPHY24cf.thinMCsignal):

  # Keep all muons and electrons
  keepParticles = ('abs(TruthParticles.pdgId) == 11 || ' # mu
                   'abs(TruthParticles.pdgId) == 13')    # e
  # Keep only the potentially signal b-hadrons
  if BPHY24cf.thinMCsignal:
    keepParticles += (' || '
                      'abs(TruthParticles.pdgId) == 511 || ' # B0
                      'abs(TruthParticles.pdgId) == 513 || ' # B0*
                      'abs(TruthParticles.pdgId) == 515')    # B0**
  # Keep all heavy particles (B and D mesons)
  else:
    keepParticles += (' || '
                      '(abs(TruthParticles.pdgId) >=  400 && abs(TruthParticles.pdgId) <  500) || ' # D-mesons
                      '(abs(TruthParticles.pdgId) >=  500 && abs(TruthParticles.pdgId) <  600) || ' # B-mesons
                      '(abs(TruthParticles.pdgId) >= 4000 && abs(TruthParticles.pdgId) < 5000) || ' # D-baryons
                      '(abs(TruthParticles.pdgId) >= 5000 && abs(TruthParticles.pdgId) < 6000) || ' # B-baryons
                      'abs(TruthParticles.pdgId) == 100553 || ' # Upsilon(2S)
                      'abs(TruthParticles.pdgId) == 200553 || ' # Upsilon(3S)
                      'abs(TruthParticles.pdgId) == 3122 || abs(TruthParticles.pdgId) == 3124 || abs(TruthParticles.pdgId) == 4122 || abs(TruthParticles.pdgId) == 4124 || abs(TruthParticles.pdgId) == 5122 || abs(TruthParticles.pdgId) == 13122 || abs(TruthParticles.pdgId) == 14122 || abs(TruthParticles.pdgId) == 23122 || abs(TruthParticles.pdgId) == 33122') # Lambdas

  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
  BPHY24_Thin_TruthHadrons = DerivationFramework__GenericTruthThinning( name                    = "BPHY24_Thin_TruthHadrons",
                                                                        ParticleSelectionString = keepParticles,
                                                                        PreserveDescendants     = True,
                                                                        StreamName = streamName,
                                                                        PreserveAncestors       = True)
  print('********** BPHY24 MC Particles Thinning **********')
  thinList += [ BPHY24_Thin_TruthHadrons ]
  ToolSvc  +=   BPHY24_Thin_TruthHadrons
  pprint      ( BPHY24_Thin_TruthHadrons.properties() )
  print('********** BPHY24 MC Particles Thinning (end) **********')

  # Save also neutrinos and b-quarks, without their decay trees
  BPHY24_Thin_TruthQuarks = DerivationFramework__GenericTruthThinning( name                    = "BPHY24_Thin_TruthQuarks",
                                                                       ParticleSelectionString = ('abs(TruthParticles.pdgId) ==  5 || '
                                                                                                  'abs(TruthParticles.pdgId) == 12 || abs(TruthParticles.pdgId) == 14' ),
                                                                       PreserveDescendants     = False,
                                                                       StreamName = streamName,
                                                                       PreserveAncestors       = False)
  print('********** BPHY24 MC Quarks Thinning **********')
  thinList += [ BPHY24_Thin_TruthQuarks ]
  ToolSvc  +=   BPHY24_Thin_TruthQuarks
  pprint      ( BPHY24_Thin_TruthQuarks.properties() )
  print('********** BPHY24 MC Quarks Thinning (end) **********')



#------------------------------------------------------------------------------
# Apply the augmentation, skimming and thinning
#------------------------------------------------------------------------------

print("BPHY24: List of augmentations: ", augsList)
print("BPHY24: List of skimmigs: "     , skimList)
print("BPHY24: List of thinnings: "    , thinList)

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel( "BPHY24Kernel",
                                                                        AugmentationTools = augsList,
                                                                        SkimmingTools     = skimList,
                                                                        ThinningTools     = thinList )
#DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel( "BPHY24Kernel_TrigCountMetadata",
#                                                                        AugmentationTools = [ BPHY24_TrigCountMetadata ],
#                                                                        SkimmingTools     = [],
#                                                                        ThinningTools     = [] )



#------------------------------------------------------------------------------
# Setup the output stream
#------------------------------------------------------------------------------

streamName   = derivationFlags.WriteDAOD_BPHY24Stream.StreamName
fileName     = buildFileName( derivationFlags.WriteDAOD_BPHY24Stream )
BPHY24Stream = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY24Stream.AcceptAlgs( [ "BPHY24Kernel" ] )

#------------------------------------------------------------------------------
# Slimming service, defining what to store
#------------------------------------------------------------------------------

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY24SlimmingHelper = SlimmingHelper( "BPHY24SlimmingHelper" )

AllVariables   = []
StaticContent  = []
ExtraVariables = []

# Smart collections
# TODO: What is the difference w.r.t. adding them into AllVariables?
BPHY24SlimmingHelper.SmartCollections = [ "Electrons", "Muons", "InDetTrackParticles" ]

# Full combined muon-ID tracks
AllVariables += [ "CombinedMuonTrackParticles" ]
AllVariables += [ "ExtrapolatedMuonTrackParticles" ]

# Extra variables
# TODO: Where do these come from? Are they missed due to the "smart" collection?
ExtraVariables += [ "Muons.etaLayer1Hits.etaLayer2Hits.etaLayer3Hits.etaLayer4Hits.phiLayer1Hits.phiLayer2Hits.phiLayer3Hits.phiLayer4Hits",
                    "Muons.numberOfTriggerEtaLayers.numberOfPhiLayers",
                    "InDetTrackParticles.numberOfTRTHits.numberOfTRTHighThresholdHits.vx.vy.vz",
                    "PrimaryVertices.chiSquared.covariance", "Electrons.deltaEta1.DFCommonElectronsLHVeryLoosenod0",
                    "egammaClusters.calE.calEta.calPhi.e_sampl.eta_sampl.etaCalo.phiCalo.ETACALOFRAME.PHICALOFRAME",
                    "HLT_xAOD__ElectronContainer_egamma_ElectronsAuxDyn.charge" ]

# Include also trigger objects
# DONE: Test it works (HLT objects appear/not-present)
if BPHY24cf.trigObjects:
  BPHY24SlimmingHelper.IncludeMuonTriggerContent  = True
  BPHY24SlimmingHelper.IncludeEgammaTriggerContent  = True
  BPHY24SlimmingHelper.IncludeBPhysTriggerContent = True

# Include primary vertices
AllVariables  += [ "PrimaryVertices" ]
print("BPHY24: List of refitted-PV output: ", outRePVList)
for i in outRePVList:
  StaticContent += [ "xAOD::VertexContainer#%s"        % i ]
  StaticContent += [ "xAOD::VertexAuxContainer#%sAux." % i ]

# B-vertexing output
print("BPHY24: List of B-vertexing output: ", outVtxList)
for i in outVtxList:
  StaticContent += [ "xAOD::VertexContainer#%s"                        % i ]
  StaticContent += [ "xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % i ]
  if [ "ReVertex" in i ]:
    StaticContent += [ "xAOD::VertexAuxContainer#%sAux." % i ]

print("BPHY24: Full list of B-augmentation: ", StaticContent)

# Truth information for MC only
if BPHY24cf.isSimulation:
  AllVariables += [ "TruthEvents",
                    "TruthParticles",
                    "TruthVertices",
                    "MuonTruthParticles" ]

# Remove duplicates
AllVariables = list(set(AllVariables))

BPHY24SlimmingHelper.AllVariables   = AllVariables
BPHY24SlimmingHelper.ExtraVariables = ExtraVariables
BPHY24SlimmingHelper.StaticContent  = StaticContent
BPHY24SlimmingHelper.AppendContentToStream(BPHY24Stream)
