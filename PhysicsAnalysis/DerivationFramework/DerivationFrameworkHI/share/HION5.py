#====================================================================
# HION5.py
# author: Aaron Angerami <angerami@cern.ch>, Andrzej Zemla <azemla@cern.ch>
#	  Jakub Kremer <jakub.kremer@cern.ch>, Piotr Janus <piotr.janus.cern.ch>
# Application: W, Z, gamma (including W/Z/gamma-jet)
# Triggers: Single high pT leptons and photons
# Selection: loose single lepton/photon selection
# Content: Electrons, Muons w/ associated tracks, Photons, limited jet info and ES summary
#====================================================================

# Set up common services and job object.
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

#for CP common tools
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *

#HI derivation tools
from DerivationFrameworkHI.HIJetDerivationTools import *
from DerivationFrameworkHI.HISkimmingTools import *
from DerivationFrameworkHI.HIAugmentationTools import *
from DerivationFrameworkHI.HIDerivationFlags import HIDerivationFlags
from HIRecExample.HIRecExampleFlags import jobproperties
from HIJetRec.HIJetRecUtils import HasCollection


#====================================================================
#Read and set conditions
#====================================================================

GetConditionsFromMetaData()

#Store truth information for W/Z boson analysis (may conflict with ttbar)
doWZBosonsTruth=False

# no truth info for data xAODs
if DerivationFrameworkHasTruth and not doWZBosonsTruth:
  import DerivationFrameworkMCTruth.MCTruthCommon

#====================================================================
# SKIMMING
#====================================================================
#removing events
#==========================================================
#Trigger selection
#==========================================================

triggers = []
if HIDerivationFlags.isPPb():
    triggers += ['HLT_e15_lhloose_nod0']
    triggers += ['HLT_mu15']
    triggers += ['HLT_g10_loose']
    triggers += ['HLT_g15_loose']
    triggers += ['HLT_g20_loose']
    triggers += ['HLT_g25_loose']
    triggers += ['HLT_g30_loose']
    triggers += ['HLT_g35_loose']
    triggers += ['HLT_2g10_loose_L12EM5']
elif not HIDerivationFlags.isPP(): 
    if not HIDerivationFlags.doMinBiasSelection():
        triggers += ['HLT_e15_loose_ion_L1EM12']
        triggers += ['HLT_mu8']
        triggers += ['HLT_g20_loose_ion']
        triggers += ['HLT_e15_lhloose_ion_L1EM12']
        triggers += ['HLT_e15_loose_ion']
    else :
        triggers += ['HLT_noalg_mb_L1TE50']
        triggers += ['HLT_mb_sptrk_ion_L1ZDC_A_C_VTE50']    

    #remove egamma tools scheduled for ForwardElectrons, which are missing in Pb+Pb reconstruction
    toolsToRemove = []
    for tool in topSequence.EGammaCommonKernel.AugmentationTools:
        if 'ForwardElectron' in tool.getName():
            toolsToRemove.append(tool)

    for tool in toolsToRemove:
        topSequence.EGammaCommonKernel.AugmentationTools.remove(tool)
else:
    triggers += ['HLT_e15_lhloose_L1EM13VH']
    triggers += ['HLT_e15_lhloose_L1EM12']
    triggers += ['HLT_e15_lhloose_nod0_L1EM12']
    triggers += ['HLT_mu14']
    triggers += ['HLT_g30_loose']    
    triggers += ['HLT_g35_loose_L1EM15']    


#Thinning threshods dictionary for jets is applied only in data
JetThinningThreshold = {'AntiKt2HIJets': 15, 'AntiKt4HIJets': 15,'DFAntiKt2HIJets': 15, 'DFAntiKt4HIJets': 15} #in GeV

#==========================================================
#Event Selection
#==========================================================
#DFCommonMuonsSelectorPreselection.TrtCutOff = True
req_electrons = 'count( Electrons.DFCommonElectronsHILHLoose && ( Electrons.pt > 15*GeV ))>0'
req_muons     = 'count( Muons.DFCommonMuonsPreselection && (Muons.pt > 15*GeV) && ( abs(Muons.eta) < 2.7))>0'
if HIDerivationFlags.isPPb(): req_photons = 'count( Photons.DFCommonPhotonsIsEMLoose && (Photons.pt > 10*GeV) ) > 0'
else : req_photons = 'count( Photons.DFCommonPhotonsIsEMLoose && (Photons.pt > 30*GeV) ) > 0'

if not HIDerivationFlags.isSimulation(): req_total = '(' + ' || '.join(triggers) + ') && (' + req_electrons + ' || ' + req_muons + ' || ' + req_photons + ')'
else : req_total = '(' + req_electrons + ' || ' + req_muons + ' || ' + req_photons + ')'

print "================AND====BEGIN=================="
print req_total
print "================AND====END===================="

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool


HION5SkimmingLeptonsAndPhotonsTool = DerivationFramework__xAODStringSkimmingTool( name = "HION5SkimmingLeptonsAndPhotonsTool",
                                                                          expression = req_total )

ToolSvc+=HION5SkimmingLeptonsAndPhotonsTool

#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_HION5Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_HION5Stream )
derivationName=streamName.split('_')[-1]
HION5Stream = MSMgr.NewPoolRootStream( streamName, fileName )
HION5Stream.AcceptAlgs(["HION5Kernel"])

#====================================================================
# PDF Weight Metadata
#====================================================================
if DerivationFrameworkHasTruth and not doWZBosonsTruth:
  import DerivationFrameworkCore.WeightMetadata

#====================================================================
# THINNING
#====================================================================
# Establish the thinning helper (which will set up the services behind the scenes)
# Plug in the required triggers

thinningTools = []
#Jet constituents for SEB only for main collection
BookDFJetCollection = (jobproperties.HIRecExampleFlags.doHIAODFix or HasCollection("DFAntiKt4HI"))
MainJetCollection = ""
if BookDFJetCollection : MainJetCollection = "DF"
thinningTools.append(addJetClusterThinningTool(MainJetCollection+'AntiKt4HIJets','HION5',20))


#b-tagging
BtaggedCollectionList=['BTagging_'+ MainJetCollection +'AntiKt4HI']
if ( HasCollection("BTagging_AntiKt4HI") and jobproperties.HIRecExampleFlags.doHIAODFix): BtaggedCollectionList.append('BTagging_AntiKt4HI')

for collection in BtaggedCollectionList :thinningTools.append(addBtaggThinningTool(collection, collection.split("_",1)[1]+"Jets", derivationName, JetThinningThreshold[collection.split("_",1)[1]+"Jets"]))

from DerivationFrameworkCore.ThinningHelper import ThinningHelper
HION5ThinningHelper = ThinningHelper( "HION5ThinningHelper" )
HION5ThinningHelper.AppendToStream( HION5Stream )

from InDetTrackSelectionTool.InDetTrackSelectionToolConf import InDet__InDetTrackSelectionTool
HITrackSelector = InDet__InDetTrackSelectionTool("HION5InDetTrackSelectionTool") 
HITrackSelector.CutLevel = "Loose"
HITrackSelector.maxNSiSharedModules = 100
HITrackSelector.minPt = 900
ToolSvc += HITrackSelector

# Create the private sequence
HION5Sequence = CfgMgr.AthSequencer("HION5Sequence")

# Then apply truth tools in the form of aumentation
if DerivationFrameworkHasTruth and not doWZBosonsTruth:
  from DerivationFrameworkTop.TOPQCommonTruthTools import TOPQCommonTruthKernel
  HION5Sequence += TOPQCommonTruthKernel

from DerivationFrameworkHI.DerivationFrameworkHIConf import DerivationFramework__HITrackParticleThinningTool
HION5ThinningTracksTool = DerivationFramework__HITrackParticleThinningTool( name = 'HION5ThinningTracksTool',
                                                                            ThinningService = HION5ThinningHelper.ThinningSvc(),
                                                                            InDetTrackParticlesKey = "InDetTrackParticles",
                                                                            PrimaryVertexKey = "PrimaryVertices",
                                                                            PrimaryVertexSelection = "sumPt2",
                                                                            TrackSelectionTool = HITrackSelector
                                                                          )

ToolSvc += HION5ThinningTracksTool
thinningTools.append(HION5ThinningTracksTool)

augToolList = []

if HIDerivationFlags.isSimulation() and doWZBosonsTruth:
    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning

    truth_req_boson = "((abs(TruthParticles.pdgId) == 23) || (abs(TruthParticles.pdgId) == 24))"
    truth_req_lepton = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 14) && (TruthParticles.pt > 1*GeV) && (TruthParticles.status == 1) && (TruthParticles.barcode < 200000))"
    truth_req_general = "(TruthParticles.status == 1) && (TruthParticles.pt > 0.7*GeV) && (abs(TruthParticles.eta) < 2.7)"

    HION5TruthLepTool = DerivationFramework__GenericTruthThinning(name                         = "HION5TruthLepTool",
                                                                  ThinningService              = HION5ThinningHelper.ThinningSvc(),
                                                                  ParticleSelectionString      = truth_req_lepton,
                                                                  PreserveDescendants          = False,
                                                                  PreserveGeneratorDescendants = False,
                                                                  PreserveAncestors            = True)

    HION5TruthBosTool = DerivationFramework__GenericTruthThinning(name                         = "HION5TruthBosTool",
                                                                  ThinningService              = HION5ThinningHelper.ThinningSvc(),
                                                                  ParticleSelectionString      = truth_req_boson,
                                                                  PreserveDescendants          = False,
                                                                  PreserveGeneratorDescendants = True,
                                                                  PreserveAncestors            = False)

    HION5TruthGenTool = DerivationFramework__GenericTruthThinning(name                         = "HION5TruthGenTool",
                                                                  ThinningService              = HION5ThinningHelper.ThinningSvc(),
                                                                  ParticleSelectionString      = truth_req_general)

    ToolSvc += HION5TruthLepTool
    ToolSvc += HION5TruthBosTool
    ToolSvc += HION5TruthGenTool
    thinningTools.append(HION5TruthLepTool)
    thinningTools.append(HION5TruthBosTool)
    thinningTools.append(HION5TruthGenTool)

if DerivationFrameworkHasTruth and not doWZBosonsTruth:
  
  from MCTruthClassifier.MCTruthClassifierConf import MCTruthClassifier

  HION5Classifier = MCTruthClassifier(
    name                      = 'HION5Classifier',
         ParticleCaloExtensionTool = '')
  ToolSvc += HION5Classifier

  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import \
    DerivationFramework__TruthClassificationDecorator
 
  HION5ClassificationDecorator = DerivationFramework__TruthClassificationDecorator(
    name              = 'HION5ClassificationDecorator',
    ParticlesKey      = 'TruthParticles',
         MCTruthClassifier = HION5Classifier)
  
  ToolSvc += HION5ClassificationDecorator
  augToolList.append(HION5ClassificationDecorator)

  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthCollectionMaker

  DFCommonTruthMuonTool = DerivationFramework__TruthCollectionMaker(name           = "DFCommonTruthMuonTool",
                                                                    NewCollectionName       = "TruthMuons",
                                                                    KeepNavigationInfo      = True,
                                                                    ParticleSelectionString = "(abs(TruthParticles.pdgId) == 13) && (TruthParticles.status == 1) && TruthParticles.barcode < "+str(DerivationFrameworkSimBarcodeOffset))
  ToolSvc += DFCommonTruthMuonTool
  augToolList.append(DFCommonTruthMuonTool)
    
  DFCommonTruthElectronTool = DerivationFramework__TruthCollectionMaker(name       = "DFCommonTruthElectronTool",
                                                                        NewCollectionName       = "TruthElectrons",
                                                                        KeepNavigationInfo      = True,
                                                                        ParticleSelectionString = "(abs(TruthParticles.pdgId) == 11) && (TruthParticles.status == 1) && TruthParticles.barcode < "+str(DerivationFrameworkSimBarcodeOffset))
  ToolSvc += DFCommonTruthElectronTool
  augToolList.append(DFCommonTruthElectronTool)

  #add the 'decoration' tool to dress the main truth collection with the classification
  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthClassificationDecorator
  DFCommonTruthClassificationTool = DerivationFramework__TruthClassificationDecorator(name = "DFCommonTruthClassificationTool",
                                                                                      ParticlesKey = "TruthParticles",
                                                                                      MCTruthClassifier = HION5Classifier)  
  ToolSvc += DFCommonTruthClassificationTool
  augToolList.append(DFCommonTruthClassificationTool)

  #add the 'decoration' tools for dressing and isolation
  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthDressingTool
  DFCommonTruthElectronDressingTool = DerivationFramework__TruthDressingTool(name = "DFCommonTruthElectronDressingTool",
                                                                             dressParticlesKey = "TruthElectrons",
                                                                             usePhotonsFromHadrons = False,
                                                                             dressingConeSize = 0.1,
                                                                             particleIDsToDress = [11]
    )
  ToolSvc += DFCommonTruthElectronDressingTool
  augToolList.append(DFCommonTruthElectronDressingTool)

  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthIsolationTool
  DFCommonTruthElectronIsolationTool1 = DerivationFramework__TruthIsolationTool(name = "DFCommonTruthElectronIsolationTool1",
                                                                                isoParticlesKey = "TruthElectrons",
                                                                                allParticlesKey = "TruthParticles",
                                                                                particleIDsToCalculate = [11],
                                                                                IsolationConeSizes = [0.2],
                                                                                IsolationVarNamePrefix = 'etcone',
                                                                                ChargedParticlesOnly = False
                                                                  )
  ToolSvc += DFCommonTruthElectronIsolationTool1
  augToolList.append(DFCommonTruthElectronIsolationTool1)

  DFCommonTruthElectronIsolationTool2 = DerivationFramework__TruthIsolationTool(name = "DFCommonTruthElectronIsolationTool2",
                                                                                isoParticlesKey = "TruthElectrons",
                                                                                allParticlesKey = "TruthParticles",
                                                                                particleIDsToCalculate = [11],
                                                                                IsolationConeSizes = [0.3],
                                                                                IsolationVarNamePrefix = 'ptcone',
                                                                                ChargedParticlesOnly = True
                                                                  )
  ToolSvc += DFCommonTruthElectronIsolationTool2
  augToolList.append(DFCommonTruthElectronIsolationTool2)

  DFCommonTruthMuonDressingTool = DerivationFramework__TruthDressingTool(name = "DFCommonTruthMuonDressingTool",
                                                                         dressParticlesKey = "TruthMuons",
                                                                         usePhotonsFromHadrons = False,
                                                                         dressingConeSize = 0.1,
                                                                         particleIDsToDress = [13]
  )
  ToolSvc += DFCommonTruthMuonDressingTool
  augToolList.append(DFCommonTruthMuonDressingTool)

  DFCommonTruthMuonIsolationTool1 = DerivationFramework__TruthIsolationTool(name = "DFCommonTruthMuonIsolationTool1",
                                                                            isoParticlesKey = "TruthMuons",
                                                                            allParticlesKey = "TruthParticles",
                                                                            particleIDsToCalculate = [13],
                                                                            IsolationConeSizes = [0.2],
                                                                            IsolationVarNamePrefix = 'etcone',
                                                                            ChargedParticlesOnly = False
                                                                  )
  ToolSvc += DFCommonTruthMuonIsolationTool1
  augToolList.append(DFCommonTruthMuonIsolationTool1)

  DFCommonTruthMuonIsolationTool2 = DerivationFramework__TruthIsolationTool(name = "DFCommonTruthMuonIsolationTool2",
                                                                            isoParticlesKey = "TruthMuons",
                                                                            allParticlesKey = "TruthParticles",
                                                                            particleIDsToCalculate = [13],
                                                                            IsolationConeSizes = [0.3],
                                                                            IsolationVarNamePrefix = 'ptcone',
                                                                            ChargedParticlesOnly = True
                                                                            )
  ToolSvc += DFCommonTruthMuonIsolationTool2
  augToolList.append(DFCommonTruthMuonIsolationTool2)

  DFCommonTruthTauDressingTool = DerivationFramework__TruthDressingTool(name = "DFCommonTruthTauDressingTool",
                                                                        dressParticlesKey = "TruthTaus",
                                                                        usePhotonsFromHadrons = False,
                                                                        dressingConeSize = 0.2, # Tau special
                                                                        particleIDsToDress = [15]
                                                                  )
  ToolSvc += DFCommonTruthTauDressingTool
  augToolList.append(DFCommonTruthTauDressingTool)

ElectronLHSelectorHILHLoose = AsgElectronLikelihoodTool("ElectronLHSelectorHILHLoose", ConfigFile="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/ElectronPhotonSelectorTools/offline/mc15_20160907_HI/ElectronLikelihoodLooseOfflineConfig2016_HI.conf")
ElectronLHSelectorHILHLoose.primaryVertexContainer = "PrimaryVertices"
ToolSvc += ElectronLHSelectorHILHLoose
ElectronPassHILHLoose = DerivationFramework__EGSelectionToolWrapper(name = "ElectronPassHILHLoose",
                                                                    EGammaSelectionTool = ElectronLHSelectorHILHLoose,
                                                                    EGammaFudgeMCTool = "",
                                                                    CutType = "",
                                                                    StoreGateEntryName = "DFCommonElectronsHILHLoose",
                                                                    ContainerName = "Electrons")
ToolSvc += ElectronPassHILHLoose
augToolList.append(ElectronPassHILHLoose)

ElectronLHSelectorHILHMedium = AsgElectronLikelihoodTool("ElectronLHSelectorHILHMedium", ConfigFile="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/ElectronPhotonSelectorTools/offline/mc15_20160907_HI/ElectronLikelihoodMediumOfflineConfig2016_HI.conf")
ElectronLHSelectorHILHMedium.primaryVertexContainer = "PrimaryVertices"
ToolSvc += ElectronLHSelectorHILHMedium
ElectronPassHILHMedium = DerivationFramework__EGSelectionToolWrapper(name = "ElectronPassHILHMedium",
                                                                     EGammaSelectionTool = ElectronLHSelectorHILHMedium,
                                                                     EGammaFudgeMCTool = "",
                                                                     CutType = "",
                                                                     StoreGateEntryName = "DFCommonElectronsHILHMedium",
                                                                     ContainerName = "Electrons")
ToolSvc += ElectronPassHILHMedium
augToolList.append(ElectronPassHILHMedium)

JetCollectionList = ['AntiKt2HIJets', 'AntiKt4HIJets', 'DFAntiKt2HIJets', 'DFAntiKt4HIJets', 'DFAntiKt6HIJets', 'DFAntiKt8HIJets', 'AntiKt10HIJets', 'DFAntiKt10HIJets', 'AntiKt2HIJets_Seed1']

if doWZBosonsTruth:
    JetCollectionList += ['AntiKt6TruthJets', 'AntiKt8TruthJets', 'AntiKt10TruthJets']

#b-tagging
extra_Bjets=[]
if jobproperties.HIRecExampleFlags.doHIAODFix: extra_Bjets=['BTagging_DFAntiKt4HI']

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
HION5SlimmingHelper = SlimmingHelper("HION5SlimmingHelper")

for item in JetCollectionList :  
    if not HION5SlimmingHelper.AppendToDictionary.has_key(item):
        HION5SlimmingHelper.AppendToDictionary[item]='xAOD::JetContainer'
        HION5SlimmingHelper.AppendToDictionary[item+"Aux"]='xAOD::JetAuxContainer'
for item in extra_Bjets :  
    if not HION5SlimmingHelper.AppendToDictionary.has_key(item):
        HION5SlimmingHelper.AppendToDictionary[item]='xAOD::BTaggingContainer'
        HION5SlimmingHelper.AppendToDictionary[item+"Aux"]='xAOD::BTaggingAuxContainer'

###############Augmentation#############
HIGlobalAugmentationTool = addHIGlobalAugmentationTool('HION5',3,500)
augToolList+=[HIGlobalAugmentationTool]

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel("HION5Kernel",
                                                                           AugmentationTools = augToolList,
                                                                           SkimmingTools = [HION5SkimmingLeptonsAndPhotonsTool],
                                                                           ThinningTools = thinningTools
                                                                      )
HION5Stream.AcceptAlgs(["HION5Kernel"])

#trackMET calculations
met_ptCutList = [1000,2000,3000,4000,5000]
from DerivationFrameworkHI.TrackMET_config import *
configTrackMET(DerivationFrameworkJob,met_ptCutList)

# Finally, add the private sequence to the main job
DerivationFrameworkJob += HION5Sequence

#====================================================================
# CONTENT LIST
#====================================================================
# Add the remaning contents (offline)

for item in JetCollectionList :  
    if not HION5SlimmingHelper.AppendToDictionary.has_key(item):
        HION5SlimmingHelper.AppendToDictionary[item]='xAOD::JetContainer'
        HION5SlimmingHelper.AppendToDictionary[item+"Aux"]='xAOD::JetAuxContainer'

from DerivationFrameworkHI.HION5ExtraContent import *

HION5SlimmingHelper.SmartCollections = ["InDetTrackParticles",
                                        "PrimaryVertices",
                                        "Electrons",
                                        "Muons",
                                        "Photons"
                                       ]
if HIDerivationFlags.isPPb(): HION5SlimmingHelper.SmartCollections += ["MET_Reference_AntiKt4EMTopo", "AntiKt4EMTopoJets"]

ExtraContentAll=[]
for collection in JetCollectionList :  
    if "Seed" not in collection:    
        for j in HIJetBranches: 
            ExtraContentAll.append(collection+'.'+j)
    else:
        for j in HISeedBranches: 
            ExtraContentAll.append(collection+'.'+j)        
        
HION5SlimmingHelper.ExtraVariables = ExtraContentAll

for v in HIClusterVars : HION5SlimmingHelper.ExtraVariables.append(v)

from DerivationFrameworkEGamma.ElectronsCPDetailedContent import *
HION5SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
HION5SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import *
HION5SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

#b-tagging
HION5SlimmingHelper.ExtraVariables.append("InDetTrackParticles.truthMatchProbability.x.y.z.vx.vy.vz")
HION5SlimmingHelper.ExtraVariables.append("InDetTrackParticles.numberOfInnermostPixelLayerSplitHits.numberOfNextToInnermostPixelLayerSplitHits.numberOfNextToInnermostPixelLayerSharedHits")
HION5SlimmingHelper.ExtraVariables.append("InDetTrackParticles.numberOfPixelSplitHits.numberOfInnermostPixelLayerSharedHits.numberOfContribPixelLayers.hitPattern.radiusOfFirstHit")
HION5SlimmingHelper.ExtraVariables.append("InDetTrackParticles.is_selected.is_associated.is_svtrk_final.pt_wrtSV.eta_wrtSV.phi_wrtSV.d0_wrtSV.z0_wrtSV.errP_wrtSV.errd0_wrtSV.errz0_wrtSV.chi2_toSV")
HION5SlimmingHelper.ExtraVariables.append("PrimaryVertices.neutralWeights.numberDoF.sumPt2.chiSquared.covariance.trackWeights")
HION5SlimmingHelper.ExtraVariables.append("PrimaryVertices.x.y.trackParticleLinks.vertexType.neutralParticleLinks")
HION5SlimmingHelper.ExtraVariables.append("ExtrapolatedMuonTrackParticles.vx.vy.vz")
HION5SlimmingHelper.ExtraVariables.append("MuonSpectrometerTrackParticles.vx.vy.vz")
HION5SlimmingHelper.ExtraVariables.append("CombinedMuonTrackParticles.vx.vy.vz")
HION5SlimmingHelper.ExtraVariables.append("Muons.EnergyLoss.energyLossType")
HION5SlimmingHelper.ExtraVariables.append("Electrons.DFCommonElectronsHILHLoose.DFCommonElectronsHILHMedium.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.etcone20.etcone30.etcone40.topoetcone20.topoetcone30.topoetcone40.ptvarcone20_TightTTVA_pt500.ptvarcone30_TightTTVA_pt500.ptvarcone40_TightTTVA_pt500.ptvarcone20_TightTTVA_pt1000.ptvarcone30_TightTTVA_pt1000.ptvarcone40_TightTTVA_pt1000.ptvarcone20_TightTTVALooseCone_pt500.ptvarcone30_TightTTVALooseCone_pt500.ptvarcone40_TightTTVALooseCone_pt500.ptvarcone20_TightTTVALooseCone_pt1000.ptvarcone30_TightTTVALooseCone_pt1000.ptvarcone40_TightTTVALooseCone_pt1000.ptcone20_TightTTVA_pt500.ptcone30_TightTTVA_pt500.ptcone40_TightTTVA_pt500.ptcone20_TightTTVA_pt1000.ptcone30_TightTTVA_pt1000.ptcone40_TightTTVA_pt1000.ptcone20_TightTTVALooseCone_pt500.ptcone30_TightTTVALooseCone_pt500.ptcone40_TightTTVALooseCone_pt500.ptcone20_TightTTVALooseCone_pt1000.ptcone30_TightTTVALooseCone_pt1000.ptcone40_TightTTVALooseCone_pt1000.topoetcone20ptCorrection.topoetcone30ptCorrection.topoetcone40ptCorrection")
HION5SlimmingHelper.ExtraVariables.append("Muons.ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.etcone20.etcone30.etcone40.topoetcone20.topoetcone30.topoetcone40.ptcone20_TightTTVA_pt500.ptcone30_TightTTVA_pt500.ptcone40_TightTTVA_pt500.ptcone20_TightTTVA_pt1000.ptcone30_TightTTVA_pt1000.ptcone40_TightTTVA_pt1000.ptvarcone20_TightTTVA_pt500.ptvarcone30_TightTTVA_pt500.ptvarcone40_TightTTVA_pt500.ptvarcone20_TightTTVA_pt1000.ptvarcone30_TightTTVA_pt1000.ptvarcone40_TightTTVA_pt1000.ptcone20_TightTTVALooseCone_pt500.ptcone30_TightTTVALooseCone_pt500.ptcone40_TightTTVALooseCone_pt500.ptcone20_TightTTVALooseCone_pt1000.ptcone30_TightTTVALooseCone_pt1000.ptcone40_TightTTVALooseCone_pt1000.ptvarcone20_TightTTVALooseCone_pt500.ptvarcone30_TightTTVALooseCone_pt500.ptvarcone40_TightTTVALooseCone_pt500.ptvarcone20_TightTTVALooseCone_pt1000.ptvarcone30_TightTTVALooseCone_pt1000.ptvarcone40_TightTTVALooseCone_pt1000")
HION5SlimmingHelper.ExtraVariables.append("Photons.etcone20.etcone30.etcone40.Loose")


addMETOutputs(HION5SlimmingHelper,met_ptCutList)
#HION5SlimmingHelper.AllVariables = ["AntiKt2HIJets","AntiKt4HIJets","AntiKt4HITrackJets","HIClusters"]
HION5SlimmingHelper.AllVariables += ["AntiKt4HITrackJets","BTagging_"+MainJetCollection+"AntiKt4HI",MainJetCollection+"AntiKt4HIJets","AntiKt4HIJets","BTagging_AntiKt4HIJets"]
if (HasCollection("BTagging_AntiKt4HI") and jobproperties.HIRecExampleFlags.doHIAODFix) : HION5SlimmingHelper.AllVariables+=["BTagging_AntiKt4HI"]
HION5SlimmingHelper.AllVariables += HIGlobalVars
if HIDerivationFlags.isPPb(): HION5SlimmingHelper.AllVariables += ["HIEventShape", "ForwardElectrons", "ForwardElectronClusters"]

if DerivationFrameworkHasTruth:

  HION5SlimmingHelper.AllVariables += ["AntiKt2TruthJets","AntiKt4TruthJets","AntiKt8TruthJets","AntiKt10TruthJets","egammaTruthParticles","TruthEvents","TruthVertices","TruthParticles"]

  if doWZBosonsTruth:
    HION5SlimmingHelper.AllVariables += ["AntiKt6TruthJets"]
  else:
    HION5SlimmingHelper.AllVariables += ["TruthElectrons","TruthMuons"]

    from DerivationFrameworkMCTruth.MCTruthCommon import addTruth3ContentToSlimmerTool
    addTruth3ContentToSlimmerTool(HION5SlimmingHelper)

HION5SlimmingHelper.IncludeEGammaTriggerContent = True
HION5SlimmingHelper.IncludeMuonTriggerContent = True

HION5SlimmingHelper.AppendContentToStream(HION5Stream)

#This enables L1EnergySums
OtherContent = [
    'xAOD::EnergySumRoI#*',
    'xAOD::EnergySumRoIAuxInfo#*'
]

for item in OtherContent:
    HION5Stream.AddItem(item)
