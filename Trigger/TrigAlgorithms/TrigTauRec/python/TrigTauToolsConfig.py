# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import mm    

def trigTauVertexFinderCfg(flags,name=''):
    acc = ComponentAccumulator()

    # Algorithm that overwrites numTrack() and charge() of tauJets in container
    TauVertexFinder = CompFactory.TauVertexFinder(name=name,
                                      UseTJVA                        =        False,
                                      AssociatedTracks               = "GhostTrack",
                                      InDetTrackSelectionToolForTJVA =           "",
                                      Key_trackPartInputContainer    =           "",
                                      Key_vertexInputContainer       =           "",
                                      OnlineMaxTransverseDistance    =       2.5*mm,   
                                      OnlineMaxZ0SinTheta            =       3.0*mm,
                                      TVATool                        =           "", 
                                      )

    acc.setPrivateTools(TauVertexFinder)
    return acc

def trigTauTrackFinderCfg(flags,name=''):
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg

    from TrkConfig.TrkVertexFitterUtilsConfig import AtlasFullLinearizedTrackFactoryCfg,AtlasTrackToVertexIPEstimatorCfg

    TrigTauExtrapolatorTool           = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags,'TrigTau_theAtlasExtrapolator'))
    acc.addPublicTool(TrigTauExtrapolatorTool)
    TrigTauTrackToVertexTool          = acc.popToolsAndMerge(TrackToVertexCfg(flags,'TrigTau_TrackToVertexTool',Extrapolator=TrigTauExtrapolatorTool))
    acc.addPublicTool(TrigTauTrackToVertexTool )
    TrigTauParticleCaloExtensionTool  = acc.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags,'TrigTau_ParticleCaloExtensionTool',Extrapolator=TrigTauExtrapolatorTool))
    acc.addPublicTool(TrigTauParticleCaloExtensionTool)
    TrigTauFullLinearizedTrackFactory = acc.popToolsAndMerge(AtlasFullLinearizedTrackFactoryCfg(flags,'TrigTau_TauFullLinearizedTrackFactory',Extrapolator=TrigTauExtrapolatorTool))
    acc.addPublicTool(TrigTauFullLinearizedTrackFactory)
    TrigTauTrackToVertexIPEstimator   = acc.popToolsAndMerge(AtlasTrackToVertexIPEstimatorCfg(flags,'TrigTau_TauTrackToVertexIPEstimator',Extrapolator=TrigTauExtrapolatorTool,LinearizedTrackFactory=TrigTauFullLinearizedTrackFactory))
    acc.addPublicTool(TrigTauTrackToVertexIPEstimator)

    from InDetConfig.InDetTrackSelectorToolConfig import InDetTrackSelectorToolCfg
    from InDetConfig.InDetTrackSelectorToolConfig import InDetTrigTRTDriftCircleCutToolCfg

    TrigTauInDetTrackSelectorTool     = acc.popToolsAndMerge(InDetTrackSelectorToolCfg(flags,'TrigTau_InDetTrackSelectorTool',
                                                                                       TrtDCCutTool       = acc.popToolsAndMerge(InDetTrigTRTDriftCircleCutToolCfg(flags,'InDetTrigTRTDriftCircleCut')), 
                                                                                       Extrapolator       = TrigTauExtrapolatorTool,
                                                                                       pTMin              = 1000.0,
                                                                                       IPd0Max            = 2.,
                                                                                       IPz0Max            = 9999.,
                                                                                       nHitBLayer         = 0,
                                                                                       nHitBLayerPlusPix  = 0,
                                                                                       nHitPix            = 2, # PixelHits + PixelDeadSensors
                                                                                       nHitSct            = 0, # SCTHits + SCTDeadSensors
                                                                                       nHitSi             = 7, # PixelHits + SCTHits + PixelDeadSensors + SCTDeadSensors
                                                                                       nHitTrt            = 0,
                                                                                       fitChi2OnNdfMax    = 99999,
                                                                                       useTrackSummaryInfo= True,
                                                                                       useSharedHitInfo   = False,
                                                                                       useTrackQualityInfo= True,
                                                                                       TrackSummaryTool   = ""))
    acc.addPublicTool(TrigTauInDetTrackSelectorTool)


    TauTrackFinder = CompFactory.TauTrackFinder(name=name,
                                    MaxJetDrTau                     = 0.2,
                                    MaxJetDrWide                    = 0.4,
                                    TrackSelectorToolTau            = TrigTauInDetTrackSelectorTool,
                                    TrackToVertexTool               = TrigTauTrackToVertexTool,
                                    Key_trackPartInputContainer     = "",
                                    maxDeltaZ0wrtLeadTrk            = 0.75*mm, #in mm
                                    removeTracksOutsideZ0wrtLeadTrk = True,
                                    ParticleCaloExtensionTool       = TrigTauParticleCaloExtensionTool,
                                    BypassSelector                  = False,
                                    BypassExtrapolator              = True,
                                    tauParticleCache                = "",
                                    TrackToVertexIPEstimator        = TrigTauTrackToVertexIPEstimator,
                                    )

                                    
    acc.setPrivateTools(TauTrackFinder)
    return acc

def tauVertexVariablesCfg(flags,name=''):
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    from TrkConfig.TrkVertexFittersConfig import TauAdaptiveVertexFitterCfg,SequentialVertexSmootherCfg
    from TrkConfig.TrkVertexSeedFinderToolsConfig import CrossDistancesSeedFinderCfg
    from TrkConfig.TrkVertexSeedFinderUtilsConfig import SeedNewtonTrkDistanceFinderCfg
    from TrkConfig.TrkVertexFitterUtilsConfig import AtlasImpactPoint3dEstimatorCfg,TauDetAnnealingMakerCfg,AtlasFullLinearizedTrackFactoryCfg

    TrigTauExtrapolatorTool           = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags,'TrigTau_theAtlasExtrapolator'))
    TrigTauSeedNewtonTrkDistanceFinder = acc.popToolsAndMerge(SeedNewtonTrkDistanceFinderCfg(flags,'TrigTau_TauSeedNewtonTrkDistanceFinder'))
    acc.addPublicTool(TrigTauSeedNewtonTrkDistanceFinder)
    TrigTauCrossDistancesSeedFinder = acc.popToolsAndMerge(CrossDistancesSeedFinderCfg(flags,'TauCrossDistancesSeedFinder',TrkDistanceFinder=TrigTauSeedNewtonTrkDistanceFinder))
    acc.addPublicTool(TrigTauCrossDistancesSeedFinder)
    TrigTauFullLinearizedTrackFactory = acc.popToolsAndMerge(AtlasFullLinearizedTrackFactoryCfg(flags,'TrigTau_TauFullLinearizedTrackFactory',Extrapolator=TrigTauExtrapolatorTool))
    acc.addPublicTool(TrigTauFullLinearizedTrackFactory)
    TrigTauImpactPoint3dEstimator = acc.popToolsAndMerge(AtlasImpactPoint3dEstimatorCfg(flags,'TrigTau_TauTrkImpactPoint3dEstimator'))
    acc.addPublicTool(TrigTauImpactPoint3dEstimator)
    TrigTauAnnealingMaker = acc.popToolsAndMerge(TauDetAnnealingMakerCfg(flags,'TrigTau_TauDetAnnealingMaker'))
    acc.addPublicTool(TrigTauAnnealingMaker)
    TrigTauVertexSmoother = acc.popToolsAndMerge(SequentialVertexSmootherCfg(flags,'TrigTau_TauSequentialVertexSmoother'))
    acc.addPublicTool(TrigTauVertexSmoother)

    TrigTauAdaptiveVertexFitter = acc.popToolsAndMerge(TauAdaptiveVertexFitterCfg(flags,'TrigTau_TauAdaptiveVertexFitter',
                                                                                        SeedFinder = TrigTauCrossDistancesSeedFinder,
                                                                                        LinearizedTrackFactory=TrigTauFullLinearizedTrackFactory,
                                                                                        ImpactPoint3dEstimator=TrigTauImpactPoint3dEstimator,
                                                                                        AnnealingMaker=TrigTauAnnealingMaker,
                                                                                        VertexSmoother=TrigTauVertexSmoother))
    acc.addPublicTool(TrigTauAdaptiveVertexFitter)

    TauVertexVariables = CompFactory.TauVertexVariables(name=name,
                                                        VertexFitter =  TrigTauAdaptiveVertexFitter ,
                                                        SeedFinder =    TrigTauCrossDistancesSeedFinder  )
    acc.setPrivateTools(TauVertexVariables)
    return acc
    
def trigTauJetRNNEvaluatorCfg(flags,name='',LLP = False):

    acc = ComponentAccumulator()

    name += '_LLP' if LLP else ''

    (NetworkFile0P, NetworkFile1P, NetworkFile3P) = \
        flags.Trigger.Offline.Tau.TauJetRNNConfigLLP if LLP \
        else flags.Trigger.Offline.Tau.TauJetRNNConfig


    MyTauJetRNNEvaluator = CompFactory.TauJetRNNEvaluator(name = name,
                                            NetworkFile0P = NetworkFile0P,
                                            NetworkFile1P = NetworkFile1P,
                                            NetworkFile3P = NetworkFile3P,
                                            OutputVarname = "RNNJetScore",
                                            MaxTracks = 10,
                                            MaxClusters = 6,
                                            MaxClusterDR = 1.0,
                                            VertexCorrection = False,
                                            TrackClassification = False,
                                            InputLayerScalar = "scalar",
                                            InputLayerTracks = "tracks",
                                            InputLayerClusters = "clusters",
                                            OutputLayer = "rnnid_output",
                                            OutputNode = "sig_prob")


    acc.setPrivateTools(MyTauJetRNNEvaluator)
    return acc

def trigTauWPDecoratorJetRNNCfg(flags,name='',LLP = False):
    import PyUtils.RootUtils as ru
    ROOT = ru.import_root()
    import cppyy
    cppyy.load_library('libxAODTau_cDict')

    acc = ComponentAccumulator()
    name += '_LLP' if LLP else ''

    # currently the target efficiencies are the same for regular tau ID and LLP tau ID
    # if we need different WPs, we can define new flags

    (flatteningFile0Prong, flatteningFile1Prong, flatteningFile3Prong) = \
        flags.Trigger.Offline.Tau.TauJetRNNWPConfigLLP if LLP \
        else flags.Trigger.Offline.Tau.TauJetRNNWPConfig

    (targetEff0Prong, targetEff1Prong, targetEff3Prong) = \
        flags.Trigger.Offline.Tau.TauJetRNNLLPTargetEff if LLP \
        else flags.Trigger.Offline.Tau.TauJetRNNTargetEff

    MyTauWPDecorator =CompFactory.TauWPDecorator( name=name,
                                     flatteningFile0Prong = flatteningFile0Prong,
                                     flatteningFile1Prong = flatteningFile1Prong,
                                     flatteningFile3Prong = flatteningFile3Prong,
                                     CutEnumVals =
                                     [ ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigVeryLoose, ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigLoose,
                                       ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigMedium, ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigTight ],
                                     SigEff0P = targetEff0Prong,
                                     SigEff1P = targetEff1Prong,
                                     SigEff3P = targetEff3Prong,
                                     ScoreName = "RNNJetScore",
                                     NewScoreName = "RNNJetScoreSigTrans",
                                     DefineWPs = True )

    acc.setPrivateTools(MyTauWPDecorator)
    return acc
