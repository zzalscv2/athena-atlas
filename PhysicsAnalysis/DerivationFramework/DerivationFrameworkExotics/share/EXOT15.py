#********************************************************************
# EXOT15.py 
# reductionConf flag EXOT15 in Reco_tf.py   
#********************************************************************
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkCore.WeightMetadata import *
from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkHasTruth

exot15Seq = CfgMgr.AthSequencer("EXOT15Sequence")

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_EXOT15Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_EXOT15Stream )
EXOT15Stream = MSMgr.NewPoolRootStream( streamName, fileName )
EXOT15Stream.AcceptAlgs(["EXOT15Kernel"])

SkipTriggerRequirement=(DerivationFrameworkHasTruth)  #apply triggers only to data

#====================================================================
# THINNING TOOLS
#====================================================================

#thinning helper
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
EXOT15ThinningHelper = ThinningHelper( "EXOT15ThinningHelper" )
EXOT15ThinningHelper.AppendToStream( EXOT15Stream )

thinningTools = []

# menu truth thinning
from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__MenuTruthThinning
EXOT15TMCThinningTool = DerivationFramework__MenuTruthThinning(name = "EXOT15TMCThinningTool",
                                                               ThinningService         = EXOT15ThinningHelper.ThinningSvc(),
                                                               WritePartons               = True,
                                                               WriteHadrons               = True,
                                                               WriteBHadrons              = True,
                                                               WriteGeant                 = False,
                                                               GeantPhotonPtThresh        = -1.0,
                                                               WriteTauHad                = False,
                                                               PartonPtThresh             = -1.0,
                                                               WriteBSM                   = True,
                                                               WriteBosons                = False,
                                                               WriteBosonProducts         = False,
                                                               WriteBSMProducts           = True,
                                                               WriteTopAndDecays          = False,
                                                               WriteEverything            = False,
                                                               WriteAllLeptons            = False,
                                                               WriteLeptonsNotFromHadrons = False,
                                                               WriteStatus3               = False,
                                                               WriteFirstN                = -1,
                                                               PreserveDescendants        = False,
                                                               PreserveGeneratorDescendants  = False,
                                                               PreserveAncestors          = False,
                                                               PreserveParentsSiblingsChildren = True)

if SkipTriggerRequirement:
    ToolSvc += EXOT15TMCThinningTool
    thinningTools.append(EXOT15TMCThinningTool)

# generic truth thinning
from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
EXOT15MCGenThinningTool = DerivationFramework__GenericTruthThinning(name = "EXOT15MCGenThinningTool",
                                                                    ThinningService         = EXOT15ThinningHelper.ThinningSvc(),
                                                                    ParticleSelectionString = "abs(TruthParticles.pdgId)==25 || abs(TruthParticles.pdgId)==35", 
                                                                    PreserveDescendants = True,
                                                                    PreserveAncestors   = True)

if SkipTriggerRequirement:
    ToolSvc += EXOT15MCGenThinningTool
    thinningTools.append(EXOT15MCGenThinningTool)

#=======================================
# SKIMMING
#=======================================
skimmingTools = []

##
## Mandatory condition for data: pass GRL
## This will be put in AND with the tools in skimmingTools
##
cmvfsGRLsLocation = '/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/'
listOfGRLs = [
    '%s/data18_13TeV/20190318/data18_13TeV.periodAllYear_DetStatus-v102-pro22-04_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml' % cmvfsGRLsLocation,
    '%s/data17_13TeV/20180619/data17_13TeV.periodAllYear_DetStatus-v99-pro22-01_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml' % cmvfsGRLsLocation,
    '%s/data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml' % cmvfsGRLsLocation,
    '%s/data15_13TeV/20170619/data15_13TeV.periodAllYear_DetStatus-v89-pro21-02_Unknown_PHYS_StandardGRL_All_Good_25ns.xml' % cmvfsGRLsLocation
]

print "==============================="
print "Applying the GRL to data events"
print "==============================="

print "First: decorate EventInfo with passDFGRL"
def applyGRL (Prefix):
    from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__GoodRunsListFilterAlgorithm
    from AthenaCommon.AppMgr import ToolSvc

    AugmentationTool = DerivationFramework__GoodRunsListFilterAlgorithm (
        name = Prefix+"_GRLAugmentationAlg",
        GoodRunsListVec = listOfGRLs,
    )
    ToolSvc += AugmentationTool
    return AugmentationTool, 'EventInfo.passDFGRL'


GRLAugTool, GRLVar = applyGRL ('EXOT15')
#=========================================
# Adding the GRL tool
# - The GRLTool augments the content with GRLVar: 1 = pass GRL, 0 = not pass GRL
# - Selection based on the string selection: GRLVar>0
#=========================================
exot15Seq += GRLAugTool

expression_grl = GRLVar + ">0"

# Event selection tool
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
GRLSkimmingTool = DerivationFramework__xAODStringSkimmingTool(name = "GRLSkimmingTool",
                                                              expression = expression_grl)
ToolSvc += GRLSkimmingTool

##
## Expression-based skimming.
## In case of MC: (at least one EMTopo jet within eta <2.8 with pT > 20 GeV) OR (at least on MS vertex)
## In case of data: (at least on MS vertex)
## Since (>=1MS vtx condition) is not used: adding "remove_the_ge1msvtx" flag to switch this stream of events off
##
remove_the_ge1msvtx =True

if SkipTriggerRequirement: 
    topology_selection = "( (count (abs(AntiKt4EMTopoJets.eta) < 2.8 && AntiKt4EMTopoJets.pt > 20) > 0) || (count (abs(MSDisplacedVertex.z) >= 0) > 0) )"
else: 
    topology_selection = "((count (abs(MSDisplacedVertex.z) >= 0) > 0) )"

expression = topology_selection

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
EXOT15SkimmingTool = DerivationFramework__xAODStringSkimmingTool(name = "EXOT15SkimmingTool", expression = expression)

ToolSvc += EXOT15SkimmingTool
if not SkipTriggerRequirement and not rec.triggerStream() == 'ZeroBias' and not remove_the_ge1msvtx: # add topology selection only to data. Keep all events in MC
    skimmingTools.append(EXOT15SkimmingTool)
 
##
## Trigger skimming: applied only on data.
## In case of ZeroBias is specified as trigger stream, the zerobias trigger is required
##
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool

if  SkipTriggerRequirement:
    print "trigger disabled"
elif rec.triggerStream() == 'ZeroBias':
    EXOT15TriggerSkimmingTool = DerivationFramework__TriggerSkimmingTool(name = "EXOT15TriggerSkimmingTool",
                                                                         TriggerListAND = [],
                                                                         TriggerListOR  = ["HLT_noalg_zb_L1ZB"])
    ToolSvc += EXOT15TriggerSkimmingTool
else:
    ##
    ## EXOT15DJTriggerSkimmingTool: trigger skimming for MS vtx and CalRation analyses, applied only on data.
    ## If data events, MS vertex and CR (paired/unpaired) triggers are required to be passed
    ##
    print "===================================="
    print "Trigger enabled: CR + MSvtx triggers"
    print "===================================="
    
    EXOT15DJTriggerSkimmingTool = DerivationFramework__TriggerSkimmingTool(name = "EXOT15DJTriggerSkimmingTool",
                                                                         TriggerListAND = [],
                                                                         TriggerListOR  = [
                                                                             "HLT_j30_muvtx","HLT_j30_muvtx_noiso",
                                                                             "HLT_j30_jes_PS_llp_L1TAU60",
                                                                             "HLT_j30_jes_PS_llp_noiso_L1TAU60",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1TAU60", 
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1TAU60", 
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1TAU100", 
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1TAU100",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1LLP-NOMATCH",    
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1LLP-NOMATCH", 
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1LLP-RO",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1LLP-RO",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1TAU8_EMPTY",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1TAU8_EMPTY",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1TAU30_EMPTY",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1TAU30_EMPTY",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1TAU8_UNPAIRED_ISO",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1TAU8_UNPAIRED_ISO",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1TAU8_UNPAIRED_NONISO",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1TAU8_UNPAIRED_NONISO",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_L1TAU30_UNPAIRED_ISO",
                                                                             "HLT_j30_jes_cleanLLP_PS_llp_noiso_L1TAU30_UNPAIRED_ISO",
                                                                             "HLT_j30_muvtx_L1MU6_EMPTY",
                                                                             "HLT_j30_muvtx_noiso_L1MU6_EMPTY",
                                                                             "HLT_j30_muvtx_L1MU4_UNPAIRED_ISO",
                                                                             "HLT_j30_muvtx_noiso_L1MU4_UNPAIRED_ISO",
                                                                             "HLT_noalg_bkg_L1J12_BGRP12"
                                                                             ])
    ToolSvc += EXOT15DJTriggerSkimmingTool
    skimmingTools.append (EXOT15DJTriggerSkimmingTool)
    print "List of triggers of EXOT15DJTriggerSkimmingTool:"
    print ", ".join (list (EXOT15DJTriggerSkimmingTool.TriggerListOR))

    ##
    ## EXOT15LepTriggerSkimmingTool: trigger skimming asking for leptonic triggers
    ## Triggers are the lowest unprescaled single lepton, dilepton (ee, mumu, emu) triggers
    ##
    print "===================================================="
    print "Trigger enabled: single-lepton and dilepton triggers"
    print "===================================================="

    leptonTriggers = [
        "HLT_mu26_ivarmedium", "HLT_mu50", "HLT_mu60_0eta105_msonly", # mu 2016-18 
        "HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0", "HLT_e140_lhloose_nod0", "HLT_e300_etcut", # el 2016-18
        "HLT_2mu14", "HLT_mu22_mu8noL1", "HLT_mu22_mu8noL1_calotag_0eta010",  # mumu 2016-18
        "HLT_2e17_lhvloose_nod0", "HLT_2e17_lhvloose_nod0_L12EM15VHI" , "HLT_2e24_lhvloose_nod0" # ee 2016-18
        "HLT_e17_lhloose_nod0_mu14","HLT_e26_lhmedium_nod0_mu8noL1", "HLT_e7_lhmedium_nod0_mu24", "HLT_e12_lhloose_nod0_2mu10", "HLT_2e12_lhloose_nod0_mu10" #emu 2018
        "HLT_e24_lhmedium_nod0_L1EM20VHI_mu8noL1", #emu 2016
        "HLT_e17_lhloose_mu14", "HLT_e7_lhmedium_mu24", "HLT_2e12_lhloose_mu10", "HLT_e12_lhloose_2mu10", # emu 2015
        "HLT_mu24_ivarmedium", # mu 2016 (low lumi)
        "HLT_e24_lhmedium_nod0_L1EM20VH", "HLT_e24_lhtight_nod0_ivarloose", # el 2016 (prescale in one run, comb full stat)
        "HLT_mu20_mu8noL1",  # mumu 2016 (low lumi)
        "HLT_mu20_iloose_L1MU15", "HLT_mu40", # mu 2015
        "HLT_2mu10", "HLT_mu18_mu8noL1", # mumu 2015
        "HLT_2e12_lhloose_L12EM10VH", # ee 2015
        "HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium", "HLT_e120_lhloose", # el 2015
        ]

    EXOT15LepTriggerSkimmingTool = DerivationFramework__TriggerSkimmingTool(name = "EXOT15LepTriggerSkimmingTool",
                                                                            TriggerListAND = [],
                                                                            TriggerListOR  = leptonTriggers
                                                                            )

    ToolSvc += EXOT15LepTriggerSkimmingTool
    print "List of triggers of EXOT15LepTriggerSkimmingTool:"
    print ", ".join (list (EXOT15LepTriggerSkimmingTool.TriggerListOR))

    
    ##
    ## SkimmingToolEXOT15: skimming tool to reduce the impact of the lepton triggers
    ## It aims at the selection of events with a lepton assocaited with potential CalRatio or MSvtx
    ## - UPDATE [09/06/2022]: trackless jet pT > 40 GeV
    ##
    print "=============================================================="
    print "Skimming the lepton triggers contribution (SkimmingToolEXOT15)"
    print "=============================================================="

    from DerivationFrameworkExotics.DerivationFrameworkExoticsConf import DerivationFramework__SkimmingToolEXOT15

    EXOT15CustomSkimmingTool = DerivationFramework__SkimmingToolEXOT15(
        name                = 'EXOT15CustomSkimmingTool',
        JetContainer        = 'AntiKt4EMTopoJets',
        TrackContainer      = 'InDetTrackParticles',
        minTrackPt          = 2,
        minJetPt            = 40,
        jetEtaCut           = 2.5,
        minDeltaR           = 0.2,
        cleanLLP            = True,
        cutLeptonPt         = False,
        minLeptonPt         = 15,
        minNLeptons         = 2,
        cutNMSRoIs          = False,
        minNMSRoIs          = 3,
        cutNMSVtx           = False,
        minNMSVtx           = 1)
    ToolSvc += EXOT15CustomSkimmingTool

    print "======================================================"
    print "EXOT25LepANDSkimmingTool: skimming of leptonic trigger"
    print "======================================================"

    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
    EXOT15LepANDSkimmingTool = DerivationFramework__FilterCombinationAND(name = "EXOT15LepANDSkimmingTool", FilterList = [EXOT15LepTriggerSkimmingTool, EXOT15CustomSkimmingTool] )

    ToolSvc += EXOT15LepANDSkimmingTool
    skimmingTools.append (EXOT15LepANDSkimmingTool)
    print "List of tools in EXOT15LepANDSkimmingTool:"
    print "[EXOT15LepTriggerSkimmingTool, EXOT15CustomSkimmingTool]" 
    
    ##
    ## EXOT15J400TriggerSkimmingTool: inclusion of J400 trigger
    ## It aims at the selection of events with a high pT jets, useful for systematics studies in MS and CR analyses
    ##
    print "=============================================================="
    print "Trigger enabled: high pT jet (J400) trigger"
    print "=============================================================="

    EXOT15J400TriggerSkimmingTool = DerivationFramework__TriggerSkimmingTool(name = "EXOT15J400TriggerSkimmingTool",
                                                                            TriggerListAND = [],
                                                                             TriggerListOR  = [
                                                                                 'HLT_j420','HLT_j400' 
                                                                             ])
    ToolSvc += EXOT15J400TriggerSkimmingTool
    print "List of triggers in EXOT15J400TriggerSkimmingTool:"
    print ", ".join (list (EXOT15J400TriggerSkimmingTool.TriggerListOR))
    
    ##
    ## EXOT15J400PSSkimmingTool: skimming tool to reduce the impact of the J400 trigger with a prescale
    ## It aims at the selection of events with a high pT jet, without any specific object for MSvtx or CR jet
    ##
    print "====================================================="
    print "Skimming the J400 triggers contribution with PreScale"
    print "====================================================="
    
    PS = 8
    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__PrescaleTool
    EXOT15J400PSSkimmingTool = DerivationFramework__PrescaleTool(   name                    = "EXOT15J400PSSkimmingTool",
                                                                    Prescale                = PS)
    ToolSvc += EXOT15J400PSSkimmingTool
    print "Applied Prescale = {0}".format (str (EXOT15J400PSSkimmingTool.Prescale))

    print "==================================================="
    print "EXOT25J400ANDSkimmingTool: skimming of J400 trigger"
    print "==================================================="

    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
    EXOT15J400ANDSkimmingTool = DerivationFramework__FilterCombinationAND(name = "EXOT15J400ANDSkimmingTool", FilterList = [EXOT15J400TriggerSkimmingTool, EXOT15J400PSSkimmingTool] )

    ToolSvc += EXOT15J400ANDSkimmingTool
    skimmingTools.append (EXOT15J400ANDSkimmingTool)
    print "List of tools in EXOT15J400ANDSkimmingTool:"
    print "[EXOT15J400TriggerSkimmingTool, EXOT15J400PSSkimmingTool] "


    ##
    ## EXOT15J25TriggerSkimmingTool: inclusion of lower-pT jet trigger
    ## It aims at the selection of events with a high pT jets, useful for systematics studies in MS and CR analyses
    ##
    print "=============================================================="
    print "Trigger enabled: lower pT jet (J25) trigger"
    print "=============================================================="

    EXOT15J25TriggerSkimmingTool = DerivationFramework__TriggerSkimmingTool(name = "EXOT15J25TriggerSkimmingTool",
                                                                            TriggerListAND = [],
                                                                            TriggerListOR  = [ 'HLT_j25',
                                                                                               'HLT_j35',
                                                                                               'HLT_j45',
                                                                                               'HLT_j60',
                                                                                               'HLT_j110'])
    ToolSvc += EXOT15J25TriggerSkimmingTool
    print "List of triggers in EXOT15J25TriggerSkimmingTool:"
    print ", ".join (list (EXOT15J25TriggerSkimmingTool.TriggerListOR))
    
    ##
    ## EXOT15J25PSSkimmingTool: skimming tool to reduce the impact of the J25 trigger with a prescale
    ## It aims at the selection of events with a lower pT jet, without any specific object for MSvtx or CR jet
    ## - PS keep separated (despite are both PS=8 for high/low pT jet trigger) to let different pre-scale in future for size adjustment.
    ##
    print "====================================================="
    print "Skimming the low-pT jet triggers contribution with PreScale"
    print "====================================================="
    
    PS = 8
    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__PrescaleTool
    EXOT15J25PSSkimmingTool = DerivationFramework__PrescaleTool(   name                    = "EXOT15J25PSSkimmingTool",
                                                                   Prescale                = PS)
    ToolSvc += EXOT15J25PSSkimmingTool
    print "Applied Prescale = {0}".format (str (EXOT15J25PSSkimmingTool.Prescale))

    print "==================================================="
    print "EXOT25J25ANDSkimmingTool: skimming of low-pT jet trigger"
    print "==================================================="

    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
    EXOT15J25ANDSkimmingTool = DerivationFramework__FilterCombinationAND(name = "EXOT15J25ANDSkimmingTool", FilterList = [EXOT15J25TriggerSkimmingTool, EXOT15J25PSSkimmingTool] )

    ToolSvc += EXOT15J25ANDSkimmingTool
    skimmingTools.append (EXOT15J25ANDSkimmingTool)
    print "List of tools in EXOT15J25ANDSkimmingTool:"
    print "[EXOT15J25TriggerSkimmingTool, EXOT15J25PSSkimmingTool] "


if not SkipTriggerRequirement and not rec.triggerStream() == 'ZeroBias':

    ##
    ## EXOT15ORSkimmingTool: final logical OR of the previous tools
    ## It aims at select all the categories of event above:
    ## - Events passing MSvtx and CR triggers (or with at least one MSvtx)
    ## - Events passing leptonic trigger && (trackless jet conditions || MS vtx/RoI condition)
    ## - Events passing prescaled J400 trigger
    ##
    print "=================================================="
    print "Final skimming tool: logical OR of the above ones!"
    print "=================================================="

    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationOR
    EXOT15ORSkimmingTool = DerivationFramework__FilterCombinationOR(name = "EXOT15ORSkimmingTool", FilterList = skimmingTools )
    ToolSvc += EXOT15ORSkimmingTool

    print "List of tools in EXOT15RSkimmingTool:"
    print "[EXOT15SkimmingTool, EXOT15DJTriggerSkimmingTool, EXOT15LepANDSkimmingTool, EXOT15J400ANDSkimmingTool, EXOT15J25ANDSkimmingTool]"

    ##
    ## EXOT15GRLandSkimmingTool: logical AND of GRL and the final OR of the previous tools
    ## It aims at select all the categories of event above IF they pass the GRL:
    ## - Events passing MSvtx and CR triggers (or with at least one MSvtx, in case the flag is switched off)
    ## - Events passing leptonic trigger && trackless jet conditions
    ## - Events passing prescaled J400 trigger
    ## - Events passing prescaled J25+ trigger
    ##
    print "========================================================================"
    print "Final skimming tool: logical AND of GRL and the OR of all previous ones!"
    print "========================================================================"

    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationAND
    EXOT15GrlANDFinalSkimmingTool = DerivationFramework__FilterCombinationAND(name = "EXOT15GrlANDFinalSkimmingTool", 
                                                                              FilterList = [EXOT15ORSkimmingTool, GRLSkimmingTool] )
    ToolSvc += EXOT15GrlANDFinalSkimmingTool

    print "List of tools in EXOT15GrlANDFinalSkimmingTool:"
    print "[EXOT15ORSkimmingTool,GRLSkimmingTool]"


#=======================================
# JETS
#=======================================

from DerivationFrameworkJetEtMiss.ExtendedJetCommon import replaceAODReducedJets
#restore AOD-reduced jet collections           
OutputJets["EXOT15"] = []
reducedJetList = [
    "AntiKt2PV0TrackJets", #flavour-tagged automatically
    "AntiKt4PV0TrackJets",
    "AntiKt4TruthJets",
    "AntiKt10TruthJets",
    "AntiKt10LCTopoJets" ] 
replaceAODReducedJets(reducedJetList,exot15Seq,"EXOT15")
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import addDefaultTrimmedJets
addDefaultTrimmedJets(exot15Seq,"EXOT15")
applyJetCalibration_xAODColl("AntiKt4EMTopo", exot15Seq)
applyJetCalibration_CustomColl("AntiKt10LCTopoTrimmedPtFrac5SmallR20", exot15Seq)


#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += exot15Seq
if SkipTriggerRequirement:
    exot15Seq += CfgMgr.DerivationFramework__DerivationKernel("EXOT15Kernel", SkimmingTools = [EXOT15SkimmingTool], ThinningTools = thinningTools)
elif rec.triggerStream() == 'ZeroBias':
    exot15Seq += CfgMgr.DerivationFramework__DerivationKernel("EXOT15Kernel", SkimmingTools = [EXOT15TriggerSkimmingTool])    
else:
    exot15Seq += CfgMgr.DerivationFramework__DerivationKernel("EXOT15Kernel", 
                                                              SkimmingTools = [EXOT15GrlANDFinalSkimmingTool])

#====================================================================
# PromptLeptonVeto decorations
#====================================================================
import JetTagNonPromptLepton.JetTagNonPromptLeptonConfig as PLVConfig
import LeptonTaggers.LeptonTaggersConfig as PLIVConfig
PLVConfig.ConfigureAntiKt4PV0TrackJets(exot15Seq, "EXOT15")
exot15Seq += PLVConfig.GetDecoratePromptLeptonAlgs()
exot15Seq += PLIVConfig.GetDecorateImprovedPromptLeptonAlgs()

#=========================================
# Adding MC Truth Meta data
#=========================================
if SkipTriggerRequirement:
    from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
    addStandardTruthContents()

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkExotics.EXOT15ContentList import *
EXOT15SlimmingHelper = SlimmingHelper("EXOT15SlimmingHelper")
EXOT15SlimmingHelper.AppendToDictionary = {
    'LCOriginTopoClusters':'xAOD::CaloClusterContainer', 
    'LCOriginTopoClustersAux':'xAOD::ShallowAuxContainer',
    'AntiKt10TruthTrimmedPtFrac5SmallR20Jets':'xAOD::JetContainer',
    'AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux':'xAOD::JetAuxContainer',
}
if DerivationFrameworkHasTruth:
    for truthc in ["TruthBoson", "TruthTop"]:
        EXOT15SlimmingHelper.StaticContent.append("xAOD::TruthParticleContainer#"+truthc)
        EXOT15SlimmingHelper.StaticContent.append("xAOD::TruthParticleAuxContainer#"+truthc+"Aux.")

EXOT15SlimmingHelper.SmartCollections = EXOT15SmartContent
EXOT15SlimmingHelper.AllVariables = EXOT15AllVariablesContent
EXOT15SlimmingHelper.ExtraVariables += ['HLT_xAOD__JetContainer_a4tcemsubjesFS.m.EMFrac','Electrons.LHMedium','PrimaryVertices.x.y','Muons.allAuthors.rpcHitTime.rpcHitIdentifier.rpcHitPositionX.rpcHitPositionY.rpcHitPositionZ', "AntiKt10LCTopoJets.DFCommonJets_jetClean_LooseBad.DFCommonJets_jetClean_LooseBadLLP.DFCommonJets_jetClean_SuperLooseBadLLP.DFCommonJets_jetClean_VeryLooseBadLLP.DFCommonJets_passJvt"]
EXOT15SlimmingHelper.ExtraVariables += PLVConfig.GetExtraPromptVariablesForDxAOD()
EXOT15SlimmingHelper.ExtraVariables += PLIVConfig.GetExtraImprovedPromptVariablesForDxAOD()
if not SkipTriggerRequirement : EXOT15SlimmingHelper.ExtraVariables += [GRLVar]
EXOT15SlimmingHelper.IncludeJetTriggerContent = True
EXOT15SlimmingHelper.IncludeBJetTriggerContent = True
EXOT15SlimmingHelper.IncludeTauTriggerContent = True
EXOT15SlimmingHelper.IncludeMuonTriggerContent = True
EXOT15SlimmingHelper.IncludeEGammaTriggerContent = True
addOriginCorrectedClusters(EXOT15SlimmingHelper,writeLC=False,writeEM=True)
EXOT15SlimmingHelper.AppendContentToStream(EXOT15Stream)
