# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def ITkFastTrackFinderStandaloneCfg(flags):
    acc = ComponentAccumulator()

    newflags = flags.cloneAndReplace("ITk.Tracking.ActivePass", "ITk.Tracking.MainPass")

    ResolvedTrackCollectionKey = 'TrigFastTrackFinder_IDTrig_Tracks'
    SiSPSeededTrackCollectionKey = 'TrigFastTrackFinder_FTF_Tracks'
    ClusterSplitProbContainer = ""

    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
    ITkTrackSummaryTool = acc.popToolsAndMerge(ITkTrackSummaryToolCfg(newflags))
    acc.addPublicTool(ITkTrackSummaryTool)

    from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
    ITkSiTrackMakerTool = acc.popToolsAndMerge( ITkSiTrackMaker_xkCfg( newflags, name = "ITkTrigSiTrackMaker_FTF" ) )
    acc.addPublicTool(ITkSiTrackMakerTool)

    acc.addPublicTool( CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter" ) )

    from RegionSelector.RegSelToolConfig import (regSelTool_ITkStrip_Cfg, regSelTool_ITkPixel_Cfg)
    pixRegSelTool = acc.popToolsAndMerge( regSelTool_ITkPixel_Cfg( newflags) )
    sctRegSelTool = acc.popToolsAndMerge( regSelTool_ITkStrip_Cfg( newflags) )

    from TrkConfig.AtlasTrackingGeometrySvcConfig import TrackingGeometrySvcCfg
    acc.merge( TrackingGeometrySvcCfg( newflags ) )
    
    acc.addPublicTool( CompFactory.TrigL2LayerNumberToolITk( name = "TrigL2LayerNumberTool_FTF",UseNewLayerScheme = True) )

    acc.addPublicTool( CompFactory.TrigSpacePointConversionTool( "TrigSpacePointConversionTool",
                                                                    DoPhiFiltering    = True,
                                                                    UseBeamTilt       = False,
                                                                    UseNewLayerScheme = True,
                                                                    RegSelTool_Pixel  = pixRegSelTool,
                                                                    RegSelTool_SCT    = sctRegSelTool,
                                                                    PixelSP_ContainerName = "ITkPixelSpacePoints",
                                                                    SCT_SP_ContainerName = "ITkStripSpacePoints",
                                                                    layerNumberTool   = acc.getPublicTool("TrigL2LayerNumberTool_FTF") ) )

    from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinderMonitoring
    monTool = TrigFastTrackFinderMonitoring(name = "TrigFastTrackFinder_", doResMon=False)

    ftf = CompFactory.TrigFastTrackFinder( name = "TrigFastTrackFinder_",
                                            LayerNumberTool          = acc.getPublicTool( "TrigL2LayerNumberTool_FTF" ),
                                            SpacePointProviderTool   = acc.getPublicTool( "TrigSpacePointConversionTool"),
                                            TrackSummaryTool         = ITkTrackSummaryTool,
                                            initialTrackMaker        = ITkSiTrackMakerTool,
                                            trigInDetTrackFitter     = acc.getPublicTool( "TrigInDetTrackFitter" ),
                                            trigZFinder              = CompFactory.TrigZFinder(),
                                            doZFinder                = False,
                                            SeedRadBinWidth          = 10,
                                            TrackInitialD0Max        = 20.0,
                                            TracksName               = SiSPSeededTrackCollectionKey,
                                            TripletDoPSS             = False,
                                            Triplet_D0Max            = 4,
                                            Triplet_D0_PPS_Max       = 1.7,
                                            Triplet_MaxBufferLength  = 3,
                                            Triplet_MinPtFrac        = 1,
                                            Triplet_nMaxPhiSlice     = 53,
                                            doCloneRemoval           = True,
                                            doResMon                 = False,
                                            doSeedRedundancyCheck    = False,
                                            pTmin                    = 1000.0,
                                            useNewLayerNumberScheme  = True,
                                            MinHits                  = 5,
                                            useGPU                   = False,
                                            DoubletDR_Max            = 270,
                                            ITkMode                  = True, # Allows ftf to use the new TrigTrackSeedGenerator for ITk
                                            StandaloneMode           = True,
                                            MonTool                  = monTool) # Allows ftf to be run as an offline algorithm with reco_tf

    acc.addEventAlgo( ftf, primary=True )

    if newflags.ITk.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        acc.merge(ITkTrackTruthCfg(newflags,
                                        Tracks=SiSPSeededTrackCollectionKey,
                                        DetailedTruth = SiSPSeededTrackCollectionKey+"DetailedTruth",
                                        TracksTruth = SiSPSeededTrackCollectionKey+"TruthCollection"))

    acc.merge(ITkTrackParticleCnvAlgCfg(newflags,
                                        name = "ITkTrackParticleCnvAlg_FTF",
                                        TrackContainerName = SiSPSeededTrackCollectionKey,
                                        OutputTrackParticleContainer = 'TrigFastTrackFinder_FTF_TrackParticles',
                                        AddTruthLink = True)
                                        )

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------
    
    acc.merge(ITkTrkAmbiguityScoreCfg( newflags,
                                        name = "ITkTrkAmbiguityScore_FTF",
                                        SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey,
                                        ClusterSplitProbContainer = ClusterSplitProbContainer))

    acc.merge(ITkTrkAmbiguitySolverCfg(newflags,
                                        name = "ITkTrkAmbiguitySolver_FTF",
                                        ResolvedTrackCollectionKey = ResolvedTrackCollectionKey))

    # ------------------------------------------------------------
    #
    # ---------- Selecting associated truth tracks
    #
    # ------------------------------------------------------------

    if newflags.ITk.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        acc.merge(ITkTrackTruthCfg(newflags,
                                        Tracks=ResolvedTrackCollectionKey,
                                        DetailedTruth = ResolvedTrackCollectionKey+"DetailedTruth",
                                        TracksTruth = ResolvedTrackCollectionKey+"TruthCollection"))

    acc.merge(ITkTrackParticleCnvAlgCfg(newflags,
                                        name = "ITkTrackParticleCnvAlg_IDTrig",
                                        TrackContainerName = ResolvedTrackCollectionKey,
                                        OutputTrackParticleContainer = 'TrigFastTrackFinder_IDTrig_TrackParticles',
                                        AddTruthLink = True)
                                        )

    # ------------------------------------------------------------
    #
    # ---------- Save tracks to AOD
    #
    # ------------------------------------------------------------

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
    toAOD = ["xAOD::TrackParticleContainer#TrigFastTrackFinder_FTF_TrackParticles"]
    toAOD += ["xAOD::TrackParticleAuxContainer#TrigFastTrackFinder_FTF_TrackParticlesAux."]
    toAOD += ["xAOD::TrackParticleContainer#TrigFastTrackFinder_IDTrig_TrackParticles"]
    toAOD += ["xAOD::TrackParticleAuxContainer#TrigFastTrackFinder_IDTrig_TrackParticlesAux."]
    acc.merge(addToAOD(newflags, toAOD))
    acc.merge(addToESD(newflags, toAOD))
    
    return acc

def ITkTrkAmbiguityScoreCfg(flags,name="ITkAmbiguityScore_FTF",SiSPSeededTrackCollectionKey=None,ClusterSplitProbContainer='',**kwargs):
    acc = ComponentAccumulator()

    ITkAmbiguityScoreProcessor = acc.popToolsAndMerge(ITkDenseEnvironmentsAmbiguityScoreProcessorToolCfg(flags,ClusterSplitProbContainer=ClusterSplitProbContainer))

    kwargs.setdefault("TrackInput",[SiSPSeededTrackCollectionKey])
    kwargs.setdefault("TrackOutput",'ScoredMapITkAmbiguityScore' + '_IDTrig')
    kwargs.setdefault("AmbiguityScoreProcessor",  ITkAmbiguityScoreProcessor)

    ITkAmbiguityScore = CompFactory.Trk.TrkAmbiguityScore(name=name+'_IDTrig', **kwargs)
    acc.addEventAlgo(ITkAmbiguityScore)
    return acc

def ITkDenseEnvironmentsAmbiguityScoreProcessorToolCfg(flags,name="ITkAmbiguityScoreProcessor_FTF",ClusterSplitProbContainer='',**kwargs):
    acc = ComponentAccumulator()

    if "ScoringTool" not in kwargs:
        if flags.Beam.Type is BeamType.Cosmics:
            from InDetConfig.InDetTrackScoringToolsConfig import ITkCosmicsScoringToolCfg
            ITkAmbiScoringTool = acc.popToolsAndMerge(ITkCosmicsScoringToolCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import ITkAmbiScoringToolCfg
            ITkAmbiScoringTool = acc.popToolsAndMerge(ITkAmbiScoringToolCfg(flags))
        kwargs.setdefault("ScoringTool", ITkAmbiScoringTool)

    if "SplitProbTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkTruthPixelClusterSplitProbToolCfg
        kwargs.setdefault("SplitProbTool", acc.popToolsAndMerge(ITkTruthPixelClusterSplitProbToolCfg(flags)) if flags.ITk.Tracking.doPixelClusterSplitting else None)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import ITkPRDtoTrackMapToolGangedPixelsCfg
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(ITkPRDtoTrackMapToolGangedPixelsCfg(flags)))

    if "AssociationToolNotGanged" not in kwargs:
        from TrkConfig.TrkAssociationToolsConfig import PRDtoTrackMapToolCfg
        kwargs.setdefault("AssociationToolNotGanged", acc.popToolsAndMerge(PRDtoTrackMapToolCfg(flags)))

    kwargs.setdefault("sharedProbCut",flags.ITk.Tracking.pixelClusterSplitProb1)
    kwargs.setdefault("sharedProbCut2",flags.ITk.Tracking.pixelClusterSplitProb2)
    kwargs.setdefault("SplitClusterMap_new", 'SplitClusterAmbiguityMap' + '_IDTrig')
    kwargs.setdefault("AssociationMapName", 'ITkPRDToTrackMap' + '_IDTrig')
    kwargs.setdefault("InputClusterSplitProbabilityName",ClusterSplitProbContainer)
    kwargs.setdefault("OutputClusterSplitProbabilityName",'SplitProb'+'_IDTrig')

    acc.setPrivateTools(CompFactory.Trk.DenseEnvironmentsAmbiguityScoreProcessorTool(name+'_IDTrig', **kwargs))
    return acc
    
def ITkTrkAmbiguitySolverCfg(flags,name="ITkAmbiguitySolver_FTF",ResolvedTrackCollectionKey=None, **kwargs):
    acc = ComponentAccumulator()

    ITkAmbiguityProcessor = acc.popToolsAndMerge(ITkDenseEnvironmentsAmbiguityProcessorToolCfg(flags))

    kwargs.setdefault("TrackInput", 'ScoredMapITkAmbiguityScore' + '_IDTrig')
    kwargs.setdefault("TrackOutput", ResolvedTrackCollectionKey)
    kwargs.setdefault("AmbiguityProcessor", ITkAmbiguityProcessor)

    ITkAmbiguitySolver = CompFactory.Trk.TrkAmbiguitySolver(name=name+'_IDTrig', **kwargs)
    acc.addEventAlgo(ITkAmbiguitySolver)
    return acc


def ITkDenseEnvironmentsAmbiguityProcessorToolCfg(flags,name="ITkAmbiguityProcessor_FTF",**kwargs):
    acc = ComponentAccumulator()

    if flags.Beam.Type is BeamType.Cosmics:
        from InDetConfig.InDetTrackScoringToolsConfig import ITkCosmicsScoringToolCfg
        ITkAmbiScoringTool = acc.popToolsAndMerge(ITkCosmicsScoringToolCfg(flags))
    else:
        from InDetConfig.InDetTrackScoringToolsConfig import ITkAmbiScoringToolCfg
        ITkAmbiScoringTool = acc.popToolsAndMerge(ITkAmbiScoringToolCfg(flags))

    if "Fitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import ITkTrackFitterAmbiCfg
        fitter_list = []
        ITkTrackFitterAmbi = acc.popToolsAndMerge(ITkTrackFitterAmbiCfg(flags,name='ITkTrackFitterAmbi'+'_IDTrig'))
        fitter_list.append(ITkTrackFitterAmbi)
        kwargs.setdefault("Fitter", fitter_list)


    from InDetConfig.InDetAssociationToolsConfig import ITkPRDtoTrackMapToolGangedPixelsCfg
    ITkPRDtoTrackMapToolGangedPixels = acc.popToolsAndMerge(ITkPRDtoTrackMapToolGangedPixelsCfg(flags))

    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolAmbiCfg
    ambi_track_summary_tool = acc.getPrimaryAndMerge(ITkTrackSummaryToolAmbiCfg(flags,name="ITkAmbiguityProcessorSplitProbTrackSummaryTool" + '_IDTrig'))

    from InDetConfig.InDetAmbiTrackSelectionToolConfig import ITkAmbiTrackSelectionToolCfg
    ITkAmbiTrackSelectionTool = acc.popToolsAndMerge(ITkAmbiTrackSelectionToolCfg(flags))

    kwargs.setdefault("AssociationTool", ITkPRDtoTrackMapToolGangedPixels)
    kwargs.setdefault("AssociationMapName", 'ITkPRDToTrackMap' + '_IDTrig')
    kwargs.setdefault("TrackSummaryTool", ambi_track_summary_tool)
    kwargs.setdefault("ScoringTool", ITkAmbiScoringTool)
    kwargs.setdefault("SelectionTool", ITkAmbiTrackSelectionTool)
    kwargs.setdefault("InputClusterSplitProbabilityName",'SplitProb'+'_IDTrig')
    kwargs.setdefault("OutputClusterSplitProbabilityName",'ITkAmbiguityProcessorSplitProb'+'_IDTrig')
    kwargs.setdefault("SuppressHoleSearch", False)
    kwargs.setdefault("tryBremFit", flags.ITk.Tracking.doBremRecovery and flags.Detector.EnableCalo and '_IDTrig' == "")
    kwargs.setdefault("caloSeededBrem", flags.ITk.Tracking.doCaloSeededBrem and flags.Detector.EnableCalo)
    kwargs.setdefault("pTminBrem", flags.ITk.Tracking.ActivePass.minPTBrem[0])
    kwargs.setdefault("RefitPrds", True)
    kwargs.setdefault("KeepHolesFromBeforeRefit", False)

    ProcessorTool = CompFactory.Trk.DenseEnvironmentsAmbiguityProcessorTool
    ITkAmbiguityProcessor = ProcessorTool(name=name+'_IDTrig', **kwargs)
    acc.setPrivateTools(ITkAmbiguityProcessor)
    return acc


def ITkTrackParticleCnvAlgCfg(flags, name="ITkTrackParticleCnvAlg", TrackContainerName="CombinedITkTracks", OutputTrackParticleContainer="InDetTrackParticles", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("ConvertTracks", True)
    kwargs.setdefault("ConvertTrackParticles", False)
    kwargs.setdefault("TrackContainerName", TrackContainerName)
    kwargs.setdefault("xAODContainerName", OutputTrackParticleContainer)
    kwargs.setdefault("xAODTrackParticlesFromTracksContainerName", OutputTrackParticleContainer)

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import ITkTrackParticleCreatorToolCfg
        kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(ITkTrackParticleCreatorToolCfg(flags)))

    if "TrackCollectionCnvTool" not in kwargs:
        kwargs.setdefault("TrackCollectionCnvTool", result.popToolsAndMerge(ITkTrackCollectionCnvToolCfg(flags)))

    if flags.ITk.Tracking.doTruth:
        kwargs.setdefault("TrackTruthContainerName", kwargs["TrackContainerName"]+"TruthCollection")
        kwargs.setdefault("AddTruthLink", True)
        if "MCTruthClassifier" not in kwargs:
            from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
            kwargs.setdefault("MCTruthClassifier", result.popToolsAndMerge(MCTruthClassifierCfg(flags)))
    else:
        kwargs.setdefault("AddTruthLink", False)

    result.addEventAlgo(CompFactory.xAODMaker.TrackParticleCnvAlg(name, **kwargs))
    return result


def ITkTrackCollectionCnvToolCfg(flags, name="ITkTrackCollectionCnvTool", **kwargs):
    result = ComponentAccumulator()

    if "TrackParticleCreator" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import ITkTrackParticleCreatorToolCfg
        ITkTrackParticleCreator = result.getPrimaryAndMerge(ITkTrackParticleCreatorToolCfg(flags))
        result.addPublicTool(ITkTrackParticleCreator)
        kwargs.setdefault("TrackParticleCreator", ITkTrackParticleCreator)

    result.setPrivateTools(CompFactory.xAODMaker.TrackCollectionCnvTool(name, **kwargs))
    return result


