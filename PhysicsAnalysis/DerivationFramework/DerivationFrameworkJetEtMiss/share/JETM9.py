#====================================================================
# JETM9.py 
# reductionConf flag JETM9 in Reco_tf.py   
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
#from DerivationFrameworkJetEtMiss.METCommon import *

#====================================================================
# SKIMMING TOOL 
#====================================================================
triggers = [
'HLT_j15',       'HLT_j15_320eta490',
'HLT_j25',       'HLT_j25_320eta490',
'HLT_j35',       'HLT_j35_320eta490',
'HLT_j45_L1RDO',
'HLT_j45',       'HLT_j45_320eta490',
'HLT_j55_L1RDO', 
'HLT_j55',       'HLT_j55_320eta490',
'HLT_j60',       'HLT_j60_320eta490',
'HLT_j85_L1RDO',
'HLT_j85',       'HLT_j85_320eta490',
'HLT_j110',      'HLT_j110_320eta490',
'HLT_j150',
'HLT_j175',      'HLT_j175_320eta490',
'HLT_j200',
'HLT_j260',      'HLT_j260_320eta490',
'HLT_j300',
'HLT_j320',
'HLT_j360',
'HLT_j380',
'HLT_j400',
'HLT_j420',
'HLT_j440',
'HLT_j460',
'HLT_noalg_L1J50',
'HLT_j35_j35_320eta490',
'HLT_j45_j45_320eta490',
'HLT_j55_j55_320eta490',
'HLT_j60_j60_320eta490',
'HLT_j85_j85_320eta490',
'HLT_j15_j15_320eta490',
'HLT_j25_j25_320eta490',
]

# NOTE: need to be able to OR isSimulated as an OR with the trigger
orstr =' || '
trigger = '('+orstr.join(triggers)+')'
expression = trigger+' || (EventInfo.eventTypeBitmask==1)'

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
JETM9SkimmingTool = DerivationFramework__xAODStringSkimmingTool(name = "JETM9SkimmingTool1",
                                                                    expression = expression)
ToolSvc += JETM9SkimmingTool

#=======================================
# CREATE PRIVATE SEQUENCE
#=======================================

jetm9Seq = CfgMgr.AthSequencer("JETM9Sequence")
DerivationFrameworkJob += jetm9Seq
#jetm9Seq = DerivationFrameworkJob

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm9Seq += CfgMgr.DerivationFramework__DerivationKernel(	name = "JETM9Kernel", 
                                                                SkimmingTools = [JETM9SkimmingTool])

#====================================================================
# Jets for R-scan 
#====================================================================
from JetRec.JetRecStandard import jtm
from JetRec.JetRecConf import JetAlgorithm


topo_rscan_mods = jtm.modifiersMap["calib_topo_ungroomed"]
print topo_rscan_mods
if jetFlags.useTruth():
    truth_rscan_mods = jtm.modifiersMap["truth_ungroomed"]
    print truth_rscan_mods
skipmods = ["ktdr","nsubjettiness","ktsplitter","angularity","dipolarity","planarflow","ktmassdrop","encorr","comshapes"]
for mod in skipmods:
    print "remove", mod
    topo_rscan_mods.remove(jtm.tools[mod])
    if jetFlags.useTruth(): truth_rscan_mods.remove(jtm.tools[mod])
jtm.modifiersMap["topo_rscan"] = topo_rscan_mods
if jetFlags.useTruth(): jtm.modifiersMap["truth_rscan"] = truth_rscan_mods

def addRscanJets(jetalg,radius,inputtype,sequence,outputlist):
    jetname = "{0}{1}{2}Jets".format(jetalg,int(radius*10),inputtype)
    algname = "jetalg"+jetname

    if not hasattr(sequence,algname):
        if inputtype == "Truth":
            addStandardJets(jetalg, radius, "Truth", mods="truth_rscan", ptmin=5000, algseq=sequence, outputGroup=outputlist)
        if inputtype == "TruthWZ":
            addStandardJets(jetalg, radius, "TruthWZ", mods="truth_rscan", ptmin=5000, algseq=sequence, outputGroup=outputlist)
        elif inputtype == "LCTopo":
            addStandardJets(jetalg, radius, "LCTopo", mods="topo_rscan",
                            ghostArea=0.01, ptmin=2000, ptminFilter=7000, calibOpt="aro", algseq=sequence, outputGroup=outputlist)

OutputJets["JETM9"] = ["AntiKt4TruthJets","AntiKt4EMTopoJets","AntiKt4LCTopoJets"]
addDefaultTrimmedJets(jetm9Seq,"JETM9")
if jetFlags.useTruth:
    replaceBuggyAntiKt4TruthWZJets(jetm9Seq,"JETM9")
for radius in [0.2, 0.3, 0.5, 0.6, 0.7, 0.8]:
    if jetFlags.useTruth:
        addRscanJets("AntiKt",radius,"Truth",jetm9Seq,"JETM9")
        addRscanJets("AntiKt",radius,"TruthWZ",jetm9Seq,"JETM9")
    addRscanJets("AntiKt",radius,"LCTopo",jetm9Seq,"JETM9")

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM9Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM9Stream )
JETM9Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM9Stream.AcceptAlgs(["JETM9Kernel"])
# for thinning
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="JETM9ThinningSvc", outStreams=[evtStream] )

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM9SlimmingHelper = SlimmingHelper("JETM9SlimmingHelper")
JETM9SlimmingHelper.SmartCollections = ["AntiKt4EMTopoJets","AntiKt4LCTopoJets","PrimaryVertices"]
JETM9SlimmingHelper.AllVariables = ["TruthEvents", "TruthVertices",
                                    "MuonSegments"
                                    ]
#JETM9SlimmingHelper.ExtraVariables = []

# Trigger content
JETM9SlimmingHelper.IncludeJetTriggerContent = True

# Add the jet containers to the stream
addJetOutputs(JETM9SlimmingHelper,["JETM9"])
JETM9SlimmingHelper.AppendContentToStream(JETM9Stream)
JETM9Stream.RemoveItem("xAOD::TrigNavigation#*")
JETM9Stream.RemoveItem("xAOD::TrigNavigationAuxInfo#*")
