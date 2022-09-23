## Only for MuHad tau algos for AOD  derivation 
from AthenaCommon.SystemOfUnits import *
from AthenaCommon.Logging import logging

## anyway the conventional tauRecTools are the base
import tauRec.TauAlgorithmsHolder as taualgs

## Assuming TauRecBuilder has been finished in upstream giving us xAOD::TauJets,
## these algos are supposed to re-process merely upon AOD

def getTauAxis():
     
    from DiTauRec.DiTauRecConf import MuHadAxisSetter
    TauAxisSetter = MuHadAxisSetter(  name = "MuHadAxisSetter", 
                        ClusterCone = 0.2 ,
                        AxisCorrection = True
                      )
                                    
    return TauAxisSetter

def getInDetTrackSelectorToolRelaxed():

    #Configures tau track selector tool (should eventually check whether an existing one is available)
    from InDetTrackSelectorTool.InDetTrackSelectorToolConf import InDet__InDetDetailedTrackSelectorTool
    InDetTrackSelectorTool = InDet__InDetDetailedTrackSelectorTool(name = "InDetTrackSelectorToolMuHad",
                                                                pTMin                = 600.,  #1000.
                                                                IPd0Max              = 5.0 ,  # 1. was default but not used, tuned for low pt DiTau 
                                                                IPz0Max              = 10.0 ,  # 1.5 was default but not used, tuned for low pt DiTau
                                                                useTrackSummaryInfo  = True,
                                                                nHitBLayer           = 0,
                                                                nHitPix              = 1,  # PixelHits + PixelDeadSensors, default 2, tuned for low pt DiTau 
                                                                nHitSct              = 0,  # SCTHits + SCTDeadSensors

                                                                nHitSi               = 7,  # PixelHits + SCTHits + PixelDeadSensors + SCTDeadSensors
                                                                nHitTrt              = 0,  # nTRTHits
                                                                useSharedHitInfo     = False,
                                                                nSharedBLayer        = 99999,
                                                                nSharedPix           = 99999,
                                                                nSharedSct           = 99999,
                                                                nSharedSi            = 99999,
                                                                useTrackQualityInfo  = True,
                                                                fitChi2OnNdfMax      = 99999,
                                                                TrackSummaryTool     = None,
                                                                Extrapolator         = taualgs.getAtlasExtrapolator())

    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += InDetTrackSelectorTool
    return InDetTrackSelectorTool

def getTauTrackFinder():

    from DiTauRec.DiTauRecConf import MuHadTrackFinder
    TauTrackFinder = MuHadTrackFinder(name = "MuonRemovedTauTrackFinder",
                                     MaxJetDrTau = 0.2,
                                     MaxJetDrWide          = 0.4,
                                     TrackSelectorToolTau  = getInDetTrackSelectorToolRelaxed(),
                                     TrackParticleContainer    = "InDetTrackParticles",
                                     TrackToVertexTool         = taualgs.getTrackToVertexTool(),
                                     ParticleCaloExtensionTool = taualgs.getParticleCaloExtensionTool(),
                                     TauTrackParticleContainer = "MuRmTauTracks", 
                                     BypassSelector            = False ,
                                     removeDuplicateCoreTracks = True ,
                                     MuonIDqualityCut1P        = 1,
                                     MuonIDqualityCut3P        = 2,
                                     MuonIDqualityCutRing      = 1 
                                     )

    return TauTrackFinder

def getClusterSubStructVariable( useTrackClassifier = False  ) :

    from DiTauRec.DiTauRecConf import  MuHadClusterSubStructVariables
    TauSubstructureVariables = MuHadClusterSubStructVariables(  name = "MuonRemovedTauClusterSubstructureVariables",
                                                          # parameters for CaloIsoCorrected variable
                                                          maxPileUpCorrection = 4000., #MeV
                                                          pileUpAlpha = 1.0,
                                                          VertexCorrection = True,
                                                          CellCone = 0.4 ,
                                                          TrkClassifyDone = useTrackClassifier ,
                                                          OnlyCoreTrack = True  )
    
    return TauSubstructureVariables

def getTauVertexVariables():
    from DiTauRec.DiTauRecConf import MuHadVertexVariables
    TauVertexVariables = MuHadVertexVariables(  name = "MuonRemovedTauVertexVariables",
                                            TrackToVertexIPEstimator = taualgs.getTauTrackToVertexIPEstimator(),
                                            VertexFitter = taualgs.getTauAdaptiveVertexFitter(),
                                            SeedFinder = taualgs.getTauCrossDistancesSeedFinder(),
                                            XAODConverter = "Trk::VxCandidateXAODVertex/VertexInternalEdmFactory" 
    # ATM only needed in case old API is used
                                           )
    
    return TauVertexVariables

def getTauJetRNNEvaluator( useTrackClassifier = False, NetworkFile1P="", NetworkFile3P="", 
       OutputVarname="RNNJetScore", MinChargedTracks=1, MaxTracks=10, MaxClusters=6, MaxClusterDR=1.0, InputLayerScalar="scalar", 
       InputLayerTracks="tracks", InputLayerClusters="clusters", OutputLayer="rnnid_output", OutputNode="sig_prob"):

    from DiTauRec.DiTauRecConf import MuHadJetRNNEvaluator
    myTauJetRNNEvaluator = MuHadJetRNNEvaluator( name="MuHadJetRNNEvaluator" ,
                                              NetworkFile1P=NetworkFile1P,
                                              NetworkFile3P=NetworkFile3P,
                                              OutputVarname=OutputVarname,
                                              MinChargedTracks=MinChargedTracks,
                                              MaxTracks=MaxTracks,
                                              MaxClusters=MaxClusters,
                                              MaxClusterDR=MaxClusterDR,
                                              InputLayerScalar=InputLayerScalar,
                                              InputLayerTracks=InputLayerTracks,
                                              InputLayerClusters=InputLayerClusters,
                                              OutputLayer=OutputLayer,
                                              OutputNode=OutputNode, 
                                              TrkClassifyDone = useTrackClassifier
                                               )

    return myTauJetRNNEvaluator

def getElectronVetoVars():
    from DiTauRec.DiTauRecConf import MuHadElectronVetoVariables
    TauElectronVetoVariables = MuHadElectronVetoVariables( name = "MuHadElectronVetoVars",
                                                         CellCorrection = True,
                                                         ParticleCaloExtensionTool = taualgs.getParticleCaloExtensionTool()
                                                       )
    return TauElectronVetoVariables

def getTauIDVarCalculator():

    from DiTauRec.DiTauRecConf import  MuHadIDVarCalculator            
    myTauIDVarCalculator = MuHadIDVarCalculator( name = "MuHadIDVarCalculator" ) 
    return myTauIDVarCalculator

def addParticleTruth( kernel=None, doComp = False, augmentationTools=[] ):
 
    from AthenaCommon.AppMgr import ToolSvc

    from DerivationFrameworkMCTruth.MCTruthCommon import  addTruthJets, addPVCollection 
    from DerivationFrameworkMCTruth.MCTruthCommon import  addTruthCollectionNavigationDecorations

    addTruthJets( kernel )
    addPVCollection( kernel )

    from MCTruthClassifier.MCTruthClassifierConf import MCTruthClassifier
    DFCommonTruthClassifier = MCTruthClassifier(name = "DFCommonTruthClassifier",
                                          ParticleCaloExtensionTool = "") 
    ToolSvc += DFCommonTruthClassifier

## prepare Truth Muons 
    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthCollectionMaker
    TruthMuonTool = DerivationFramework__TruthCollectionMaker(
         name                    = 'TruthMuonTool',
         NewCollectionName       = 'TruthMuons',
         ParticleSelectionString = '(abs(TruthParticles.pdgId) == 13) && '
                                   '(TruthParticles.status == 1) && '
                                   '(TruthParticles.barcode < 200000)',
         KeepNavigationInfo      = True                                    )
    ToolSvc += TruthMuonTool
    augmentationTools.append(TruthMuonTool)

    TruthElecTool = DerivationFramework__TruthCollectionMaker(
         name                    = 'TruthElecTool',
         NewCollectionName       = 'TruthElectrons',
         ## electrons are complicated ... not sure these selections are good enough
         ParticleSelectionString = '(abs(TruthParticles.pdgId) == 11) && '
                                   '(TruthParticles.status == 1) && '
                                   '(TruthParticles.barcode < 200000)',
         KeepNavigationInfo      = True                                    )
    ToolSvc += TruthElecTool
    augmentationTools.append( TruthElecTool )

## Prepare truth taus
    from tauRecTools.tauRecToolsConf import tauRecTools__BuildTruthTaus
    btt = tauRecTools__BuildTruthTaus( WriteTruthTaus=True ,
                                       WriteInvisibleFourMomentum=True,
                                       WriteVisibleNeutralFourMomentum = True,
                                       MCTruthClassifierTool= ToolSvc.DFCommonTruthClassifier
                                     )
    ToolSvc += btt
    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthCollectionMakerTau
    DFCommonTruthTauCollectionMaker = DerivationFramework__TruthCollectionMakerTau()
    DFCommonTruthTauCollectionMaker.BuildTruthTaus = btt
    ToolSvc += DFCommonTruthTauCollectionMaker
    augmentationTools.append( DFCommonTruthTauCollectionMaker ) 

## Link Truth to reco for Muons and electrons
    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthLinkRepointTool

    elec_relinktau = DerivationFramework__TruthLinkRepointTool("ElMiniCollectionTruthLinkToolMuHad",
                             RecoCollection="Electrons", TargetCollections=[ "TruthMuons", "TruthElectrons" ]
                                                           )
    ToolSvc += elec_relinktau
    augmentationTools.append( elec_relinktau )

    muon_relinktau = DerivationFramework__TruthLinkRepointTool("MuMiniCollectionTruthLinkToolMuHad",
                             RecoCollection="Muons", TargetCollections=[ "TruthMuons", "TruthElectrons" ]
                                                           )
    ToolSvc += muon_relinktau
    augmentationTools.append( muon_relinktau )

## redo Links to TruthParticle Container
    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthNavigationDecorator
    TruthNavigationDecoratortau = DerivationFramework__TruthNavigationDecorator( name='TruthNavigationDecoratorMuHad',
                         InputCollections= [ "TruthElectrons", "TruthMuons", "TruthTaus" ] )
    ToolSvc += TruthNavigationDecoratortau 
    augmentationTools.append( TruthNavigationDecoratortau )

    from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__TruthClassificationDecorator
    TauClassificationDecorator = DerivationFramework__TruthClassificationDecorator(
                                         name              = "TauClassificationDecorator",
                                         ParticlesKey      = "TruthParticles",
                                         MCTruthClassifier = ToolSvc.DFCommonTruthClassifier 
                                                                                      )
    ToolSvc += TauClassificationDecorator
    augmentationTools.append(TauClassificationDecorator)

    return 

def getMetReMaker( sequence, suffix ) :

    from METReconstruction.METRecoFlags import metFlags
    from METReconstruction.METRecoConfig import BuildConfig, RefConfig, METConfig, getMETRecoAlg

    MetConf_Reco = {}

    rf_builders = [BuildConfig('Ele'),
                   BuildConfig('Gamma'),
                   BuildConfig('Tau', 'RefTau', "MuRmTauJets" ),
                   BuildConfig('Jet'),
                   BuildConfig('Muon'),
                   BuildConfig('SoftClus'),
                   BuildConfig('SoftTrk')]
    rf_refiners = [ RefConfig( 'TrackFilter','PVSoftTrk' ) ]
    MetConf_Reco[ suffix ] = METConfig(
                                  suffix,
                                  rf_builders,
                                  rf_refiners,
                                  doSum=True,
                                  doTracks=True,
                                  doRegions=False
                               )

    MetConf_Reco[ suffix ].builders['Tau'].MinPt     = 15e3
    MetConf_Reco[ suffix ].builders['Tau'].MaxEta    = 2.5
## careful no muon veto is necessary anymore since we've tried to remove it.
    MetConf_Reco[ suffix ].builders['Tau'].IsTauFlag = 28   # JetRNNSigVeryLoose,  vs 30 for JetRNNSigMedium
    MetConf_Reco[ suffix ].builders['Tau'].ElVeto    = 22   # EleBDTLoose adopted in downstream analysis, vs 23 for EleBDTMedium
    MetConf_Reco[ suffix ].builders['Tau'].MinWet    = 0.5

    for key,cfg in MetConf_Reco.iteritems():
        metFlags.METConfigs()[ suffix ] = cfg
        metFlags.METOutputList().append( suffix )
        metFlags.METOutputList().append( suffix + "Reference" )

    sequence += getMETRecoAlg( 'MET_' + suffix, MetConf_Reco ) 

    MetConf_Assoc = {}
    from METReconstruction.METAssocConfig import METAssocConfig,AssocConfig,getMETAssocAlg

    associators = [ AssocConfig( 'PFlowJet', 'AntiKt4EMPFlowJets' ),
                    AssocConfig('Muon', 'Muons'),
                    AssocConfig('Ele', 'Electrons'),
                    AssocConfig('Gamma', 'Photons'),
                    AssocConfig( 'Tau', 'MuRmTauJets' ),
                    AssocConfig('Soft')]

    MetConf_Assoc[ suffix ] = METAssocConfig( suffix,
                               associators,
                               doPFlow= False )

    metFlags.METAssocConfigs()[ suffix ] = MetConf_Assoc[ suffix ]
    assocAlg = getMETAssocAlg( 'METAssoc_' + suffix , MetConf_Assoc )
    sequence += assocAlg
 
    return 
