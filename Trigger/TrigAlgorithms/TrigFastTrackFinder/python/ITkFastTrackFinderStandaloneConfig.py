# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkFastTrackFinderStandaloneCfg(flags):
    acc = ComponentAccumulator()

    newflags = flags.cloneAndReplace("ITk.Tracking.ActiveConfig", "ITk.Tracking.FTFPass")
    
    ResolvedTrackCollectionKey = 'TrigFastTrackFinder_IDTrig_Tracks'
    SiSPSeededTrackCollectionKey = 'TrigFastTrackFinder_FTF_Tracks'

    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
    ITkTrackSummaryTool = acc.popToolsAndMerge(ITkTrackSummaryToolCfg(newflags, name = "ITkTrackSummaryTool_FTF", doHolesInDet = False))
    acc.addPublicTool(ITkTrackSummaryTool)
    
    from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
    ITkSiTrackMakerTool = acc.popToolsAndMerge( ITkSiTrackMaker_xkCfg( newflags, name = "ITkTrigSiTrackMaker_FTF" ) )
    acc.addPublicTool(ITkSiTrackMakerTool)

    acc.addPublicTool( CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter" ) )

    from RegionSelector.RegSelToolConfig import (regSelTool_ITkStrip_Cfg, regSelTool_ITkPixel_Cfg)
    pixRegSelTool = acc.popToolsAndMerge( regSelTool_ITkPixel_Cfg( newflags) )
    sctRegSelTool = acc.popToolsAndMerge( regSelTool_ITkStrip_Cfg( newflags) )
    
    acc.addPublicTool( CompFactory.TrigL2LayerNumberToolITk( name = "TrigL2LayerNumberTool_FTF",UseNewLayerScheme = True) )

    acc.addPublicTool( CompFactory.TrigSpacePointConversionTool( "TrigSpacePointConversionTool",
                                                                    DoPhiFiltering    = True,
                                                                    UseBeamTilt       = False,
                                                                    UseNewLayerScheme = True,
                                                                    RegSelTool_Pixel  = pixRegSelTool,
                                                                    RegSelTool_SCT    = sctRegSelTool,
                                                                    PixelSP_ContainerName = "ITkPixelSpacePoints",
                                                                    UseSctSpacePoints = False,
                                                                    layerNumberTool   = acc.getPublicTool("TrigL2LayerNumberTool_FTF") ) )

    from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderMonitoringArg
    from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig
    acc.merge(TriggerHistSvcConfig(newflags))
    monTool = TrigFastTrackFinderMonitoringArg(flags, name = "FullScan", doResMon=False)
    
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
                                            Triplet_MaxBufferLength  = 1,
                                            Triplet_MinPtFrac        = 0.7,
                                            Triplet_nMaxPhiSlice     = 53,
                                            doCloneRemoval           = True,
                                            UseTrigSeedML            = 1,
                                            doResMon                 = False,
                                            doSeedRedundancyCheck    = True,
                                            pTmin                    = 1000.0,
                                            useNewLayerNumberScheme  = True,
                                            UseEtaBinning            = True,
                                            MinHits                  = 5,
                                            useGPU                   = False,
                                            DoubletDR_Max            = 270,
                                            ITkMode                  = True, # Allows ftf to use the new TrigTrackSeedGenerator for ITk
                                            StandaloneMode           = True,
                                            doTrackRefit             = False,
                                            MonTool                  = monTool) # Allows ftf to be run as an offline algorithm with reco_tf

    acc.addEventAlgo( ftf, primary=True )

    if newflags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        acc.merge(ITkTrackTruthCfg(newflags,
                                        Tracks=SiSPSeededTrackCollectionKey,
                                        DetailedTruth = SiSPSeededTrackCollectionKey+"DetailedTruth",
                                        TracksTruth = SiSPSeededTrackCollectionKey+"TruthCollection"))

    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    acc.merge(ITkTrackParticleCnvAlgCfg(newflags,
                                        name = "ITkTrackParticleCnvAlg_FTF",
                                        TrackContainerName = SiSPSeededTrackCollectionKey,
                                        xAODTrackParticlesFromTracksContainerName = 'TrigFastTrackFinder_FTF_TrackParticles'))

    # ------------------------------------------------------------
    #
    # ---------- Ambiguity solving
    #
    # ------------------------------------------------------------
    
    from TrkConfig.TrkAmbiguitySolverConfig import ITkTrkAmbiguityScoreCfg

    acc.merge(ITkTrkAmbiguityScoreCfg( newflags,
                                       name = "ITkTrkAmbiguityScore_FTF",
                                       SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey))

    from TrkConfig.TrkAmbiguitySolverConfig import ITkTrkAmbiguitySolverCfg
    acc.merge(ITkTrkAmbiguitySolverCfg(newflags,
                                        name = "ITkTrkAmbiguitySolver_FTF",
                                        ResolvedTrackCollectionKey = ResolvedTrackCollectionKey))

    # ------------------------------------------------------------
    #
    # ---------- Selecting associated truth tracks
    #
    # ------------------------------------------------------------

    if newflags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        acc.merge(ITkTrackTruthCfg(newflags,
                                        Tracks=ResolvedTrackCollectionKey,
                                        DetailedTruth = ResolvedTrackCollectionKey+"DetailedTruth",
                                        TracksTruth = ResolvedTrackCollectionKey+"TruthCollection"))

    acc.merge(ITkTrackParticleCnvAlgCfg(newflags,
                                        name = "ITkTrackParticleCnvAlg_IDTrig",
                                        TrackContainerName = ResolvedTrackCollectionKey,
                                        xAODTrackParticlesFromTracksContainerName = 'TrigFastTrackFinder_IDTrig_TrackParticles'))

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


