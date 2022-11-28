#********************************************************************
# HIGG4D5.py:  reductionConf flag HIGG4D5 in Reco_tf.py
# Z. Zinonos - zenon@cern.ch
#
DAOD_StreamID = 'HIGG4D5'
#********************************************************************

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *

# running on data or MC
from AthenaCommon.GlobalFlags import globalflags
DFisMC = (globalflags.DataSource()=='geant4')

if DFisMC:
    from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
    addStandardTruthContents()
    from DerivationFrameworkMCTruth.HFHadronsCommon import *

print "Hello, my name is {} and I am running on {}".format(DAOD_StreamID, 'MC' if DFisMC else 'Data')

#==============
# SET UP STREAM
#==============
streamName = derivationFlags.WriteDAOD_HIGG4D5Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_HIGG4D5Stream )
HIGG4D5Stream = MSMgr.NewPoolRootStream( streamName, fileName )
HIGG4D5Stream.AcceptAlgs([DAOD_StreamID+"Kernel"])

#============
# Setup tools
#============
# Establish the thinning helper
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
HIGG4D5ThinningHelper = ThinningHelper( DAOD_StreamID+"ThinningHelper" )

#trigger navigation thinning
import DerivationFrameworkHiggs.HIGG4DxThinning
HIGG4D5ThinningHelper.TriggerChains = DerivationFrameworkHiggs.HIGG4DxThinning.TriggerChains(DAOD_StreamID)
HIGG4D5ThinningHelper.AppendToStream( HIGG4D5Stream )

# thinning tools
thinningTools = DerivationFrameworkHiggs.HIGG4DxThinning.setup(DAOD_StreamID, HIGG4D5ThinningHelper.ThinningSvc(), ToolSvc)

# skimming tools
import DerivationFrameworkHiggs.HIGG4DxSkimming
skimmingTools = DerivationFrameworkHiggs.HIGG4DxSkimming.setup(DAOD_StreamID, ToolSvc)

#augmentation tools
from DerivationFrameworkHiggs.HIGG4DxAugmentation import *
augmentationTools = DerivationFrameworkHiggs.HIGG4DxAugmentation.setup(DAOD_StreamID, ToolSvc)

#slimming tools
import DerivationFrameworkHiggs.HIGG4DxSlimming

#slimming helper
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
HIGG4D5SlimmingHelper = SlimmingHelper(DAOD_StreamID+"SlimmingHelper")

# jets and calibration
import DerivationFrameworkHiggs.HIGG4DxJets

################
# Truth thinning 
################
#Truth tau/nutau and their ancestors and descendants
truth_cond_tau = " ((abs(TruthParticles.pdgId) == 15 || abs(TruthParticles.pdgId) == 16) && (TruthParticles.pt > 0.01*GeV))"
truth_cond_lep = " ((abs(TruthParticles.pdgId) >= 11 && abs(TruthParticles.pdgId) <= 14) && (TruthParticles.pt > 4.0*GeV) && (TruthParticles.status == 1))"
truth_photon = " ((abs(TruthParticles.pdgId) == 22) && (TruthParticles.pt > 1*GeV)) "
truth_cond_comb = "("+truth_cond_lep+"||"+truth_cond_tau+"||"+truth_photon+")"

# PreserveGeneratorDescendants only keeps particles that came directly from the event generator
# PreserveDescendants keeps all particles including those that come from Geant processes

from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
HIGG4D5TruthTool1 = DerivationFramework__GenericTruthThinning(name                         = "HIGG4D5TruthTool1",
                                                              ThinningService              = HIGG4D5ThinningHelper.ThinningSvc(),
                                                              ParticleSelectionString      = truth_cond_comb,
                                                              PreserveDescendants          = True, 
                                                              PreserveGeneratorDescendants = False,
                                                              PreserveAncestors            = True,
                                                              TauHandling                  = False)



from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__MenuTruthThinning
HIGG4D5TruthTool2 = DerivationFramework__MenuTruthThinning(name                      = "HIGG4D5TruthTool2",
                                                           ThinningService            = HIGG4D5ThinningHelper.ThinningSvc(),
                                                           WritePartons               = False,
                                                           WriteHadrons               = False,
                                                           WriteBHadrons              = False,
                                                           WriteGeant                 = False,
                                                           GeantPhotonPtThresh        = -1.0,
                                                           WriteTauHad                = True,
                                                           PartonPtThresh             = -1.0,
                                                           WriteBSM                   = False,
                                                           WriteBosons                = True,
                                                           WriteBSMProducts           = False,
                                                           WriteBosonProducts         = True,
                                                           WriteTopAndDecays          = True,
                                                           WriteEverything            = False,
                                                           WriteAllLeptons            = True,
                                                           WriteStatus3               = False,
                                                           PreserveParentsSiblingsChildren = True,
                                                           WriteFirstN                = -1)

if DerivationFrameworkHasTruth:
    ToolSvc += HIGG4D5TruthTool1
    thinningTools.append(HIGG4D5TruthTool1)
    ToolSvc += HIGG4D5TruthTool2
    thinningTools.append(HIGG4D5TruthTool2)



#==============================================================================
# HEAVY FLAVOR DECORATION
#==============================================================================
# PhysicsAnalysis/DerivationFramework/DerivationFrameworkTop/trunk/src/TTbarPlusHeavyFlavorFilterTool.cxx
# PhysicsAnalysis/DerivationFramework/DerivationFrameworkTop/trunk/src/TopHeavyFlavorFilterAugmentation.cxx
# these are supposed to mimic the TTbarPlusBFilter, TTbarPlusBBFilter, and TTbarPlusCFilter Filters in https://svnweb.cern.ch/trac/atlasoff/browser/Generators/MC15JobOptions/trunk/common/Filters
if DFisMC:
    from DerivationFrameworkTop.DerivationFrameworkTopConf import DerivationFramework__TTbarPlusHeavyFlavorFilterTool

    HIGG4D5ttbarBfiltertool = DerivationFramework__TTbarPlusHeavyFlavorFilterTool("HIGG4D5TTbarPlusBFilterTool")
    HIGG4D5ttbarBfiltertool.SelectB = True
    HIGG4D5ttbarBfiltertool.BpTMinCut = 5000
    HIGG4D5ttbarBfiltertool.BMultiplicityCut = 1 # >=
    ToolSvc += HIGG4D5ttbarBfiltertool

    HIGG4D5ttbarBBfiltertool = DerivationFramework__TTbarPlusHeavyFlavorFilterTool("HIGG4D5TTbarPlusBBFilterTool")
    HIGG4D5ttbarBBfiltertool.SelectB = True
    HIGG4D5ttbarBBfiltertool.BpTMinCut = 15000
    HIGG4D5ttbarBBfiltertool.BMultiplicityCut = 2 # >=
    ToolSvc += HIGG4D5ttbarBBfiltertool

    HIGG4D5ttbarCfiltertool = DerivationFramework__TTbarPlusHeavyFlavorFilterTool("HIGG4D5TTbarPlusCFilterTool")
    HIGG4D5ttbarCfiltertool.SelectC = True
    HIGG4D5ttbarCfiltertool.CpTMinCut = 15000
    HIGG4D5ttbarCfiltertool.CMultiplicityCut = 1 # >=
    # these two are the default values
    # B-hadrons have precedence; if one B is found, it won't pass the CFilter
    HIGG4D5ttbarCfiltertool.BpTMinCut = 5000
    HIGG4D5ttbarCfiltertool.BMultiplicityCut = 1 # >=
    ToolSvc += HIGG4D5ttbarCfiltertool

    from DerivationFrameworkTop.DerivationFrameworkTopConf import DerivationFramework__TopHeavyFlavorFilterAugmentation
    HIGG4D5TopHFFilterAugmentation = DerivationFramework__TopHeavyFlavorFilterAugmentation(name = "HIGG4D5TopHFFilterAugmentation")
    HIGG4D5TopHFFilterAugmentation.BFilterTool = HIGG4D5ttbarBfiltertool
    HIGG4D5TopHFFilterAugmentation.BBFilterTool = HIGG4D5ttbarBBfiltertool
    HIGG4D5TopHFFilterAugmentation.CFilterTool = HIGG4D5ttbarCfiltertool
    ToolSvc += HIGG4D5TopHFFilterAugmentation
    augmentationTools.append(HIGG4D5TopHFFilterAugmentation)
    print "HIGG4D5TopHFFilterAugmentationTool: ", HIGG4D5TopHFFilterAugmentation

#==============================================================================
# HEAVY FLAVOR DECORATIONS (ttbar)
#==============================================================================
# PhysicsAnalysis/DerivationFramework/DerivationFrameworkMCTruth/src/HadronOriginClassifier.cxx -- HOC
# PhysicsAnalysis/DerivationFramework/DerivationFrameworkMCTruth/src/HadronOriginDecorator.cxx
# list of ttbar samples by mc_channel_number
HIGG4D5DSIDList=[
                 346343,
                 346344,
                 346345,
                 410081,
                 410250,#in HOC, Sherpa
                 410251,#in HOC, Sherpa
                 410252,#in HOC, Sherpa
                 410397,
                 410398,
                 410399,
                 410464,#in HOC, Pythia8
                 410465,#in HOC, Pythia8
                 410470,#in HOC, Pythia8
                 410471,#in HOC, Pythia8
                 410472,#in HOC, Pythia8
                 410480,#in HOC, Pythia8
                 410482,
                 410501,#in HOC, Pythia8
                 410503,#in HOC, Pythia8
                 410544,
                 411287,
                 411233, 
                 411234,
                 600666,
                 600667,
                 600668,
                 410557,
                 410558,
                 412116,
                 412117,
                 411320,
                 411321,
                 600640,
                 600641,
                 505074,
                 505075,
                 505076,
                 506085,
                 506086,
                 506087,      
                 700122,
                 700123,
                 700124,    
                 410144, #Sherpa ttW
                 410376, #MG+P8 ttW
                 410377, #ttW 
                 413001, #ttW 
                 413008, #ttW 
                 410155,#aMC@NlO+P8 ttW
                 412123,#MG+P8 ttW
                 600793, 
                 600794,
                 600795,
                 600796,
                 600787,
                 600788,
                 600789,
                 600790,
                 501720,
                 410143, #ttZ
                 410156, 
                 410157,
                 410218,#aMC@NlO+P8 ttZ
                 410219,#aMC@NlO+P8 ttZ
                 410220,#aMC@NlO+P8 ttZ
                 410276,#aMC@NlO+P8 ttZ_lowMass
                 410277,#aMC@NlO+P8 ttZ_lowMass
                 410278,#aMC@NlO+P8 ttZ_lowMass
                 413022,
                 413023,#sherpa 2.2.1 ttZ
                 504329,#amc@NLO+H7.2.1 refined ttZ
                 504330,#aMC@NLO+P8 refined ttZ
                 504331,#MC@NLO+P8 refined ttZ
                 504332,#MC@NLO+P8 refined ttZ
                 504333,#amc@NLO+H7.2.1 refined ttZ
                 504334,#aMC@NLO+P8 refined ttZ
                 504335,#aMC@NLO+P8 refined ttZ
                 504336,#MC@NLO+P8 refined ttZ
                 504337,#amc@NLO+H7.2.1 refined ttZ
                 504338,#MC@NLO+P8 refined ttZ
                 504341,#amc@NLO+H7.2.1 refined ttZ
                 504342,#aMC@NLO+P8 refined ttZ
                 504343,#aMC@NLO+P8 refined ttZ
                 504344,#aMC@NLO+P8 refined ttZ
                 504345,#amc@NLO+H7.2.1 refined ttZ
                 504346,#aMC@NLO+P8 refined ttZ
                 700000,#Sherpa 2.2.8 ttW
                 700168,#Sherpa 2.2.10 ttW
                 700205,#Sherpa 2.2.10 ttW
                 700309,#Sherpa 2.2.11 ttZ
                ]

import PyUtils.AthFile as af
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
f = af.fopen(athenaCommonFlags.PoolAODInput()[0])
if len(f.mc_channel_number) > 0:
    if(int(f.mc_channel_number[0]) in HIGG4D5DSIDList):
        from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__HadronOriginClassifier
        HIGG4D5hadronorigintool = DerivationFramework__HadronOriginClassifier("HIGG4D5HadronOriginClassifier",DSID=int(f.mc_channel_number[0]))
        ToolSvc += HIGG4D5hadronorigintool
        print "HIGG4D5hadronorigintool: ", HIGG4D5hadronorigintool
        from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__HadronOriginDecorator
        HIGG4D5hadronorigindecorator = DerivationFramework__HadronOriginDecorator(name = "HIGG4D5HadronOriginDecorator")
        HIGG4D5hadronorigindecorator.ToolName = HIGG4D5hadronorigintool
        ToolSvc += HIGG4D5hadronorigindecorator
        print "HIGG4D5hadronorigindecorator: ", HIGG4D5hadronorigindecorator
        augmentationTools.append(HIGG4D5hadronorigindecorator)
                                                                       

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM
#=======================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel

# Create the private sequence
HIGG4D5Sequence = CfgMgr.AthSequencer(DAOD_StreamID+"Sequence")

# augmentation
HIGG4D5Sequence += CfgMgr.DerivationFramework__CommonAugmentation("HIGG4DxCommonAugmentationKernel", AugmentationTools = augmentationTools)

# skimming
HIGG4D5Sequence += CfgMgr.DerivationFramework__DerivationKernel(DAOD_StreamID+"SkimmingKernel", SkimmingTools = skimmingTools)

# fat/trimmed jet building (after skimming)
DerivationFrameworkHiggs.HIGG4DxJets.setup(DAOD_StreamID, HIGG4D5Sequence, HIGG4D5SlimmingHelper)

# thinning
HIGG4D5Sequence += CfgMgr.DerivationFramework__DerivationKernel(DAOD_StreamID+"Kernel", ThinningTools = thinningTools)

# add the private sequence to the main job
DerivationFrameworkJob += HIGG4D5Sequence

# slimming - last position
DerivationFrameworkHiggs.HIGG4DxSlimming.setup(DAOD_StreamID, HIGG4D5Stream, HIGG4D5SlimmingHelper)

