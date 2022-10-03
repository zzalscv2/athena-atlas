# *****************************************************
# TAUP6.py 
# reductionConf flag TAUP6 in Reco_tf.py  
DAOD_StreamID = 'TAUP6' 
# *****************************************************

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__CommonAugmentation
from DerivationFrameworkInDet.InDetCommon import *
from AthenaCommon.GlobalFlags import globalflags
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import addMETOutputs
from DerivationFrameworkTau.TauCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkFlavourTag.FlavourTagCommon import *

useTrackClassifierBDT = True     ##  whether to benefit from TrackClassifier via BDT algo

saveCalo = True      ##  whether to save relevant "CaloCalTopoClusters" 

saveEleBDT = True   ##  whether to provide score of EleBDT for downstream usage

doComparison = True   ## whether to store original/conventional TauJets from AOD to dAOD
##  by default the Comparison is not necessary for real data 

outputKey = 'MuRmTauJets'
TracksKey = 'MuRmTauTracks'

# =============================================
# Private sequence here
# =============================================
TAUP6seq = CfgMgr.AthSequencer("TAUP6Sequence")

# =============================================
# Set up stream
# =============================================
streamName      = derivationFlags.WriteDAOD_TAUP6Stream.StreamName
fileName        = buildFileName( derivationFlags.WriteDAOD_TAUP6Stream )
TAUP6Stream     = MSMgr.NewPoolRootStream( streamName, fileName )
TAUP6Stream.AcceptAlgs(["TAUP6Kernel"])

print( ' ... TAUP6 coming ... ' ) 

# =============================================
# Thinning tool
# =============================================

muonTriggers       = '^(?!.*_[0-9]*(e|tau|mu))(?!HLT_mu.*_[0-9]*mu.*)HLT_mu.*|HLT_2mu.*'
tauTriggers        = 'HLT_tau.*'

from DerivationFrameworkCore.ThinningHelper import ThinningHelper
TAUP6ThinningHelper                              = ThinningHelper( "TAUP6ThinningHelper" )
TAUP6ThinningHelper.TriggerChains                = muonTriggers + '|' + tauTriggers
TAUP6ThinningHelper.AppendToStream( TAUP6Stream )

thinningTools = []

# Tracks associated with muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
TAUP6MuonTPThinningTool   = DerivationFramework__MuonTrackParticleThinning(
  name                      = "TAUP6MuonTPThinningTool",
  ThinningService           = TAUP6ThinningHelper.ThinningSvc(),
  MuonKey                   = "Muons",
  InDetTrackParticlesKey    =  "InDetTrackParticles" )
ToolSvc += TAUP6MuonTPThinningTool
thinningTools.append(TAUP6MuonTPThinningTool)

# Tracks associated with taus
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TauTrackParticleThinning
TAUP6TauTPThinningTool  = DerivationFramework__TauTrackParticleThinning(
  name                    = "TAUP6TauTPThinningTool",
  ThinningService         = TAUP6ThinningHelper.ThinningSvc(),
  TauKey                  = outputKey,
  InDetTrackParticlesKey  = "InDetTrackParticles" ,
  TauTracksKey            = TracksKey, 
  ConeSize                = 0.4 )
ToolSvc += TAUP6TauTPThinningTool
thinningTools.append(TAUP6TauTPThinningTool)

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
TAUP6ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(
        name                    = "ElectronTPThinningToolMRtau",
        ThinningService         = TAUP6ThinningHelper.ThinningSvc(),
        SGKey                   = "Electrons",
        BestMatchOnly           = False,
        InDetTrackParticlesKey  = "InDetTrackParticles"
    )
ToolSvc += TAUP6ElectronTPThinningTool
thinningTools.append( TAUP6ElectronTPThinningTool ) 

## only CaloCalTopoClusters around tau and muon, borrowing AODFix
## remember to store "CaloCalTopoClusters" to the output stream
if ( saveCalo or doComparison ) :
    hasAODFix = False
    from AODFix import AODFix   # saveCalo and tauidRNN is UNnecessarily bounded together here 
    if not ( AODFix_willDoAODFix() and "tauid" in AODFix._aodFixInstance.latestAODFixVersion() ) :
        try :
            AODFix_postSystemRec() # just ASSUME this postSystemRec() can guarantee to call tauid
            hasAODFix = True
        except ( AttributeError, RuntimeError ) :
            mlog.error("Failed to schedule AODFix to tauidRNN for original taus while to LCOriginTopoClusters for new MuRmTaus ")
            hasAODFix = False
    else :
        print( ' AODFix had been scheduled befor TAUP6 by default ' ) 
        hasAODFix = True
 
    saveCalo = saveCalo and hasAODFix
    doComparison = doComparison and hasAODFix

if saveCalo :
    from DerivationFrameworkCalo.DerivationFrameworkCaloConf import DerivationFramework__CaloClusterThinning

    TAUP6MuonCCThinningTool = DerivationFramework__CaloClusterThinning( 
                                               name                  = "TAUP6MuonCCThinningTool",
                                               ThinningService         = TAUP6ThinningHelper.ThinningSvc(),
                                               SGKey                   = "Muons",
                                               TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                               ConeSize                = 0.2 )

    ToolSvc += TAUP6MuonCCThinningTool
    thinningTools.append( TAUP6MuonCCThinningTool )

    TAUP6tauCCThinningTool = DerivationFramework__CaloClusterThinning(
                                               name                  = "TAUP6tauCCThinningTool",
                                               ThinningService         = TAUP6ThinningHelper.ThinningSvc(),
                                               SGKey                   = outputKey,
                                               TopoClCollectionSGKey   = "LCOriginTopoClusters",  
                                               # careful on the trick from LCOriginTopoClusters to CaloCalTopoClusters
                                               AdditionalTopoClCollectionSGKey = ["CaloCalTopoClusters"],
                                               ConeSize                = 0.5 )
    
    ToolSvc += TAUP6tauCCThinningTool
    thinningTools.append( TAUP6tauCCThinningTool )

    TAUP6ElectronCCThinningTool = DerivationFramework__CaloClusterThinning( 
                                               name                  = "TAUP6ElectronCCThinningTool",
                                               ThinningService         = TAUP6ThinningHelper.ThinningSvc(),
                                               SGKey                   = "Electrons",
                                               TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                               ConeSize                = 0.4)

    ToolSvc += TAUP6ElectronCCThinningTool
    thinningTools.append( TAUP6ElectronCCThinningTool )

## not all the Electron are useful : thinning
EleReq = '( Electrons.pt > 7.0*GeV && abs(Electrons.eta) < 2.6 && ( Electrons.Loose || Electrons.DFCommonElectronsLHLoose ) )'
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__GenericObjectThinning
TAUP6ElectronsThinningTool = DerivationFramework__GenericObjectThinning( 
                              name            = "TAUP6ElectronsThinningTool",
                              ThinningService = TAUP6ThinningHelper.ThinningSvc(),
                              SelectionString = EleReq ,
                              ContainerName   = "Electrons"
                                                                  )
ToolSvc += TAUP6ElectronsThinningTool
thinningTools.append( TAUP6ElectronsThinningTool )
## We leave muon and tau not thinned for possible performance xcheck.

# =============================================
# Skimming tool
# =============================================
mySkimmingtools = [ ]

MuReq = '( Muons.pt >= 9.0*GeV && abs(Muons.eta) < 2.6 && Muons.DFCommonMuonsPreselection )'
tauReq = '(' + outputKey + '.pt >= 10.0*GeV && abs( ' + outputKey + '.eta) < 2.6 && ' + outputKey + '.nChargedTracks >= 1 )'
muTauSel = '( count(' + MuReq + ') >= 1 && count(' + tauReq + ') >=1 )' 

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
objSkimmingTool = DerivationFramework__xAODStringSkimmingTool( name = "ObjSkimmingTool",
                                                               expression = muTauSel )
ToolSvc += objSkimmingTool
mySkimmingtools.append( objSkimmingTool )

# =============================================
# muHad Tau reconstruction
# =============================================

import tauRec.TauAlgorithmsHolder as taualgs
taualgs.setAODmode(True)
from tauRec.tauRecFlags import tauFlags

import DiTauRec.MuHadAlgorithmsHolder as MuRmTauConfig 
tools = []
RNNtools = []
try:

    tools.append( MuRmTauConfig.getTauAxis() )

    TauTrackFinder = MuRmTauConfig.getTauTrackFinder() 
    TauTrackFinder.TauTrackParticleContainer  = TracksKey
    tools.append( TauTrackFinder )

    if useTrackClassifierBDT  :
        trackClassifier = taualgs.getTauTrackClassifier() 
        trackClassifier.TauTrackContainerName = TracksKey 
        tools.append( trackClassifier )

    tools.append( taualgs.getEnergyCalibrationLC(correctEnergy=True, correctAxis=False, postfix='_onlyEnergy'))

    tools.append( MuRmTauConfig.getClusterSubStructVariable( useTrackClassifierBDT ) )
    tools.append( MuRmTauConfig.getTauVertexVariables() )

    dfltTauTrackFinder = taualgs.getTauTrackFinder()
    dfltTauTrackFinder.TauTrackParticleContainer  = TracksKey
    RNNtools.append( dfltTauTrackFinder )

    if useTrackClassifierBDT :
        RNNtools.append( trackClassifier ) 

    RNNtools.append( MuRmTauConfig.getTauJetRNNEvaluator( useTrackClassifierBDT,
                                                          NetworkFile1P="rnnid_mc16d_config_1p.json",
                                                          NetworkFile3P="rnnid_mc16d_config_3p.json",
                                                          OutputVarname="RNNJetScore", 
                                                          MaxTracks=10, 
                                                          MaxClusters=6
                                                        )
                   )

    RNNtools.append(     taualgs.getTauWPDecoratorJetRNN(
                              flatteningFile1Prong="rnnid_mc16d_flat_1p.root",
                              flatteningFile3Prong="rnnid_mc16d_flat_3p.root"
                          )
    )

## check  TauRecAODProcessor_EleBDTFix  from TauRecAODBuilder.py 
##  if  EleBDT will be used in analysis
    if saveEleBDT :
         tools.append( MuRmTauConfig.getElectronVetoVars() )
         tools.append( MuRmTauConfig.getTauIDVarCalculator() )

         RNNtools.append( taualgs.getTauJetBDTEvaluator(  suffix="TauEleBDT_def", 
                                                          weightsFile="", 
                                                          outputVarName="BDTEleScore_retuned"
                                                       )
                        )
         RNNtools.append( taualgs.getTauJetBDTEvaluator(  suffix="TauEleBDT_bar", 
                                                          weightsFile="EleBDTBar_fix20190108.root", 
                                                          minNTracks=1, maxAbsTrackEta=1.37, 
                                                          outputVarName="BDTEleScore_retuned"
                                                       )
                        )
         RNNtools.append( taualgs.getTauJetBDTEvaluator(  suffix="TauEleBDT_end1", 
                                                          weightsFile="EleBDTEnd1_fix20190108.root", 
                                                          minNTracks=1, minAbsTrackEta=1.37, maxAbsTrackEta=2.0, 
                                                          outputVarName="BDTEleScore_retuned"
                                                       )
                        )
         RNNtools.append( taualgs.getTauJetBDTEvaluator(  suffix="TauEleBDT_end23", 
                                                          weightsFile="EleBDTEnd23_fix20190108.root", 
                                                          minNTracks=1, minAbsTrackEta=2.0, maxAbsTrackEta=3.0, 
                                                          outputVarName="BDTEleScore_retuned"
                                                       )
                        )

        # Decorate working points
         RNNtools.append( taualgs.getTauWPDecoratorEleBDT(  "EleBDTFlat_fix20190108.root",
                                                           "EleBDTFlat_fix20190108.root",
                                                           retuned=True
                                                         )
                        )


except ( AttributeError, RuntimeError ) :
    mlog.error("could not append tools to TauBuilder")
    print( traceback.format_exc() )

print( 'Adding tools to ToolSvc ... '  )
for tool in tools :
    tool.calibFolder = tauFlags.tauRecToolsCVMFSPath()
    print( tool ) 
    ToolSvc += tool  

for tool in RNNtools :
    tool.calibFolder = tauFlags.tauRecToolsCVMFSPath()
    print( tool ) 
    ToolSvc += tool  
print( 'tools prepared. '  )

from MuonMomentumCorrections.MuonMomentumCorrectionsConf import CP__MuonCalibrationAndSmearingTool
MuonCalibrationTool = CP__MuonCalibrationAndSmearingTool("MuonCalibrationAndSmearingTool" )
ToolSvc += MuonCalibrationTool

from MuonSelectorTools.MuonSelectorToolsConf import CP__MuonSelectionTool
MuonSelectionTool= CP__MuonSelectionTool("MuonSelectionTool")
MuonSelectionTool.MaxEta = 2.5
ToolSvc += MuonSelectionTool


        ########################################################################
        # TauBuilder
        # create the taus
try:

    from DiTauRec.DiTauRecConf import MuHadProcessorTool
    myTauProcessorTool = MuHadProcessorTool(
             name = "MuonRemovedTauProcessorTool",
             TauContainer                = outputKey,
             TauTrackContainer           = TracksKey,
             MuonCalibrationToolHandle   = MuonCalibrationTool ,
             MuonSelectionToolHandle     = MuonSelectionTool ,
             seedJetConeSize             = 0.4 ,
             muonIDWP                    = 2 , 
             # consitent with MuonIDqualityCut1P, MuonIDqualityCut3P, MuonIDqualityCutRing in TauTrackFinder
             RecTools                    = tools,
             RNNTools                    = RNNtools,
             SaveClusters                = saveCalo ,
             TrackClassificationDone     = useTrackClassifierBDT ,
#             OutputLevel                 = 1 ,
             runOnAOD                    = True 
    )

except ( AttributeError, RuntimeError ) :
    mlog.error("could not instance MuonRemoved TauBuilderTool")
    print( traceback.format_exc() )


try:
    from tauRec.tauRecConf import TauProcessorAlg
    mrTauReBuilder = TauProcessorAlg( name= "MuonRemovedTauReBuilderAlg", 
                                      Tools = [ myTauProcessorTool ]
                                    )
except ( AttributeError, RuntimeError ) :
    mlog.error("could not instance MuonRemoved TauBuilderAlgorithm ")
    print( traceback.format_exc() )

TAUP6seq += mrTauReBuilder 

 #re-tag PFlow jets so they have b-tagging info.
from DerivationFrameworkFlavourTag.FlavourTagCommon import *
FlavorTagInit( JetCollections = [ 'AntiKt4EMPFlowJets', 'AntiKt4EMTopoJets' ], Sequencer = TAUP6seq )

## more augument for Truth info
augTools = []
if not globalflags.DataSource()=='data' :

    from DerivationFrameworkCore.LHE3WeightMetadata import *

    MuRmTauConfig.addParticleTruth( DerivationFrameworkJob, doComparison, augTools )  

#    chLep = " ( abs(TruthParticles.pdgId) == 13 || abs(TruthParticles.pdgId) == 15 )"
    qrk = "( abs(TruthParticles.pdgId) >= 1 && abs(TruthParticles.pdgId) <= 5 )"
    ## this TruthParticles.eta some time may invite FPE 
    phs = "( TruthParticles.pt > 5*GeV && abs(TruthParticles.eta) < 2.6 )"
    fstt = "( ( TruthParticles.status >=1 && TruthParticles.status <=3 ) || TruthParticles.status ==11 || TruthParticles.status ==23 )"
    bcd = "( TruthParticles.barcode<200000 )" 
#    TruthParts_cond = '( (' + chLep + ' || ' + qrk + ') &&' + phs + ' && ' + fstt + ' && ' + bcd + ')'
    TruthParts_cond = '(' + qrk + ' &&' + phs + ' && ' + fstt + ' && ' + bcd + ')'

    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
    TruthPartsThinningTool  = DerivationFramework__GenericTruthThinning(
                            name                          = "TAUP6MCGenThinningTool",
                            ThinningService               = TAUP6ThinningHelper.ThinningSvc() ,
                            ParticleSelectionString       = TruthParts_cond ,
                            PreserveGeneratorDescendants  = False ,
                            PreserveAncestors             = True  )
    ToolSvc += TruthPartsThinningTool
    thinningTools.append( TruthPartsThinningTool )

# =============================================
# Create derivation Kernel
# =============================================

TAUP6seq += CfgMgr.DerivationFramework__DerivationKernel(
  "TAUP6Kernel",
  SkimmingTools             = mySkimmingtools,
  ThinningTools             = thinningTools,
  AugmentationTools         = augTools
  )

DerivationFrameworkJob += TAUP6seq

## redo MET 

metSfx = "AntiKt4EMPFlow_" + outputKey 
MuRmTauConfig.getMetReMaker( DerivationFrameworkJob, metSfx ) 

# =============================================
# Add the containers to the output stream (slimming done here)
# =============================================

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkTau.MuHadExtraContent import *

TAUP6SlimmingHelper       = SlimmingHelper("TAUP6SlimmingHelper")

TAUP6SlimmingHelper.AppendToDictionary = { outputKey : 'xAOD::TauJetContainer' ,
                                           outputKey + 'Aux' : 'xAOD::TauJetAuxContainer' ,
                                           TracksKey : 'xAOD::TauTrackContainer' ,
                                           TracksKey + 'Aux' : 'xAOD::TauTrackAuxContainer' ,
                                           'MuRmTauSecondaryVertices' : 'xAOD::VertexContainer' ,
                                           'MuRmTauSecondaryVerticesAux' : 'xAOD::VertexAuxContainer'
                                         }

TAUP6SlimmingHelper.AppendToDictionary.update( {
                                           'MET_' + metSfx : 'xAOD::MissingETContainer' ,
                                           'MET_' + metSfx + 'Aux' : 'xAOD::MissingETAuxContainer' ,
                                           'METAssoc_' + metSfx : 'xAOD::MissingETAssociationMap' ,
                                           'METAssoc_' + metSfx + 'Aux' : 'xAOD::MissingETAuxAssociationMap' ,
                                           'MET_Core_' + metSfx : 'xAOD::MissingETContainer' ,
                                           'MET_Core_' + metSfx + 'Aux' : 'xAOD::MissingETAuxContainer' ,
                                           'MET_Reference_' + metSfx : 'xAOD::MissingETContainer' ,
                                           'MET_Reference_' + metSfx + 'Aux' : 'xAOD::MissingETAuxContainer'
                                         } )

TAUP6SlimmingHelper.SmartCollections = [
                                        "Muons", 
                                        outputKey, 
                                        "Electrons",
                                        "AntiKt4EMTopoJets",
                                        "AntiKt4EMTopoJets_BTagging201810",
                                        "BTagging_AntiKt4EMTopo_201810",
                                        "MET_Reference_AntiKt4EMTopo",
                                        "AntiKt4EMPFlowJets",
                                        "AntiKt4EMPFlowJets_BTagging201903",
                                        "BTagging_AntiKt4EMPFlow_201903",
                                        "MET_Reference_AntiKt4EMPFlow",
                                        "PrimaryVertices"]

if doComparison :
    TAUP6SlimmingHelper.SmartCollections += [ "TauJets" ]

from DerivationFrameworkEGamma.ElectronsCPDetailedContent import *

TAUP6SlimmingHelper.ExtraVariables              = ExtraContentTAUP6
if doComparison :
    TAUP6SlimmingHelper.ExtraVariables          += ExtraContentOrignalTAU

if saveCalo :
    TAUP6SlimmingHelper.ExtraVariables          += ExtraContentCaloClusters

if DerivationFrameworkHasTruth:

    TAUP6SlimmingHelper.StaticContent +=  [ "xAOD::TruthParticleContainer#TruthMuons",
                                            "xAOD::TruthParticleAuxContainer#TruthMuonsAux.",
                                            "xAOD::TruthParticleContainer#TruthElectrons",
                                            "xAOD::TruthParticleAuxContainer#TruthElectronsAux.",
                                            "xAOD::TruthParticleContainer#TruthTaus",
                                            "xAOD::TruthParticleAuxContainer#TruthTausAux."
                                           ] 
    TAUP6SlimmingHelper.SmartCollections += [ "AntiKt4TruthJets" ]  
    TAUP6SlimmingHelper.AllVariables += [ "TruthTaus" ]
    TAUP6SlimmingHelper.ExtraVariables    += ExtraContentTruthTAUP6


TAUP6Stream.AddItem("xAOD::EventInfo#*")
TAUP6Stream.AddItem("xAOD::EventAuxInfo#*")
TAUP6Stream.AddItem("xAOD::EventShape#*")
TAUP6Stream.AddItem("xAOD::EventShapeAuxInfo#*")

TAUP6SlimmingHelper.IncludeMuonTriggerContent    = True
TAUP6SlimmingHelper.IncludeTauTriggerContent     = True
TAUP6SlimmingHelper.IncludeEGammaTriggerContent  = False
TAUP6SlimmingHelper.IncludeEtMissTriggerContent  = False
TAUP6SlimmingHelper.IncludeJetTriggerContent     = False
TAUP6SlimmingHelper.IncludeBJetTriggerContent    = False

TAUP6SlimmingHelper.AppendContentToStream(TAUP6Stream)

