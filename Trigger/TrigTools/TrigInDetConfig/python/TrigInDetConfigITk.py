#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkftfCfg(flags, roisKey, signature, signatureName):
    acc = ComponentAccumulator()
    
    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
    ITkTrackSummaryTool = acc.popToolsAndMerge(ITkTrackSummaryToolCfg(flags))
    acc.addPublicTool(ITkTrackSummaryTool)

    from InDetConfig.SiCombinatorialTrackFinderToolConfig import ITkSiCombinatorialTrackFinder_xkCfg
    CombinatorialTrackFinderTool = acc.popToolsAndMerge(ITkSiCombinatorialTrackFinder_xkCfg(flags,
                                                                                                name="ITkTrigSiComTrackFinder",
                                                                                                PixelClusterContainer='ITkTrigPixelClusters',
                                                                                                SCT_ClusterContainer='ITkTrigStripClusters'))

    from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
    ITkSiTrackMakerTool = acc.popToolsAndMerge(ITkSiTrackMaker_xkCfg(flags, name = "ITkTrigSiTrackMaker_FTF"+signature, useBremModel = False, CombinatorialTrackFinder = CombinatorialTrackFinderTool) )
    acc.addPublicTool(ITkSiTrackMakerTool)

    acc.addPublicTool( CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter" ) )

    from RegionSelector.RegSelToolConfig import (regSelTool_ITkStrip_Cfg, regSelTool_ITkPixel_Cfg)
    pixRegSelTool = acc.popToolsAndMerge( regSelTool_ITkPixel_Cfg( flags) )
    sctRegSelTool = acc.popToolsAndMerge( regSelTool_ITkStrip_Cfg( flags) )
    
    acc.addPublicTool( CompFactory.TrigL2LayerNumberToolITk( name = "TrigL2LayerNumberToolITk_FTF",UseNewLayerScheme = True) )
    acc.addPublicTool( CompFactory.TrigL2LayerNumberToolITk( "TrigL2LayerNumberToolITk_FTF" ) )

    acc.addPublicTool( CompFactory.TrigSpacePointConversionTool( "TrigSpacePointConversionTool" + signature,
                                                                    DoPhiFiltering    = True,
                                                                    UseBeamTilt       = False,
                                                                    UseNewLayerScheme = True,
                                                                    RegSelTool_Pixel  = pixRegSelTool,
                                                                    RegSelTool_SCT    = sctRegSelTool,
                                                                    PixelSP_ContainerName = "ITkPixelTrigSpacePoints",
                                                                    UseSctSpacePoints = False,
                                                                    layerNumberTool   = acc.getPublicTool("TrigL2LayerNumberToolITk_FTF") ) )

    from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinderMonitoringArg
    monTool = TrigFastTrackFinderMonitoringArg(flags, name = "trigfasttrackfinder_" + signature, doResMon=False)

    ftf = CompFactory.TrigFastTrackFinder( name = "TrigFastTrackFinder_" + signature,
                                            LayerNumberTool          = acc.getPublicTool( "TrigL2LayerNumberToolITk_FTF" ),
                                            SpacePointProviderTool   = acc.getPublicTool( "TrigSpacePointConversionTool" + signature ),
                                            TrackSummaryTool         = ITkTrackSummaryTool,
                                            initialTrackMaker        = ITkSiTrackMakerTool,
                                            trigInDetTrackFitter     = acc.getPublicTool( "TrigInDetTrackFitter" ),
                                            RoIs                     = roisKey,
                                            trigZFinder              = CompFactory.TrigZFinder(),
                                            doZFinder                = False,
                                            SeedRadBinWidth          = flags.ITk.Tracking.ActiveConfig.SeedRadBinWidth,
                                            TrackInitialD0Max        = 1000. if flags.ITk.Tracking.ActiveConfig.extension == 'cosmics' else 20.0,
                                            TracksName               = flags.ITk.Tracking.ActiveConfig.trkTracks_FTF,
                                            TripletDoPSS             = False,
                                            Triplet_D0Max            = flags.ITk.Tracking.ActiveConfig.Triplet_D0Max,
                                            Triplet_D0_PPS_Max       = flags.ITk.Tracking.ActiveConfig.Triplet_D0_PPS_Max,
                                            Triplet_MaxBufferLength  = 3,
                                            Triplet_MinPtFrac        = 1,
                                            Triplet_nMaxPhiSlice     = 53,
                                            doCloneRemoval           = flags.ITk.Tracking.ActiveConfig.doCloneRemoval,
                                            doResMon                 = flags.ITk.Tracking.ActiveConfig.doResMon,
                                            doSeedRedundancyCheck    = flags.ITk.Tracking.ActiveConfig.doSeedRedundancyCheck,
                                            pTmin                    = flags.ITk.Tracking.ActiveConfig.minPT[0],
                                            useNewLayerNumberScheme  = True,
                                            MinHits                  = 5,
                                            useGPU                   = False,
                                            DoubletDR_Max            = 270,
                                            ITkMode                  = True, 
                                            StandaloneMode           = False,
                                            MonTool = monTool)

    acc.addEventAlgo( ftf, primary=True )

    return acc

def ITktrigInDetFastTrackingCfg( inflags, roisKey, signatureName, in_view ):
    """ Generates precision fast tracking config, it is a primary config function """

    flags = inflags.cloneAndReplace("ITk.Tracking.ActiveConfig", "ITk.Tracking.MainPass")
    trigflags = flags.cloneAndReplace("ITk.Tracking.ActiveConfig", "Trigger.ITkTracking."+signatureName)
    
    #If signature specified add suffix to the name of each algorithms
    signature =  ("_" + signatureName if signatureName else '').lower()

    acc = ComponentAccumulator()

    if in_view:
        from TrigInDetConfig.TrigInDetConfig import InDetCacheNames
        verifier = CompFactory.AthViews.ViewDataVerifier( name = 'VDVInDetFTF'+signature,
                                                        DataObjects= [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                                                                        ('InDet::PixelClusterContainerCache', 'PixelTrigClustersCache'),
                                                                        ('PixelRDO_Cache', 'PixRDOCache'),
                                                                        ('InDet::SCT_ClusterContainerCache', 'SCT_ClustersCache'),
                                                                        ('SCT_RDO_Cache', 'SctRDOCache'),
                                                                        ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.PixBSErrCacheKey ),
                                                                        ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTBSErrCacheKey ),
                                                                        ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTFlaggedCondCacheKey ),
                                                                        ('SpacePointCache', 'PixelSpacePointCache'),
                                                                        ('SpacePointCache', 'SctSpacePointCache'),
                                                                        ('xAOD::EventInfo', 'EventInfo'),
                                                                        ('TrigRoiDescriptorCollection', str(roisKey)),
                                                                        ( 'TagInfo' , 'DetectorStore+ProcessingTags' )] )

        if flags.Input.isMC:
            verifier.DataObjects += [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                                    ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ) ]
            from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
            sgil_load = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                        ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ) ]
            acc.merge(SGInputLoaderCfg(flags, Load=sgil_load))

        acc.addEventAlgo(verifier)

    from InDetConfig.InDetPrepRawDataFormationConfig import ITkTrigPixelClusterizationCfg, ITkTrigStripClusterizationCfg
    acc.merge(ITkTrigPixelClusterizationCfg(flags, roisKey=roisKey, signature=signature))
    acc.merge(ITkTrigStripClusterizationCfg(flags, roisKey=roisKey, signature=signature))

    from InDetConfig.SiSpacePointFormationConfig import ITkTrigSiTrackerSpacePointFinderCfg
    acc.merge(ITkTrigSiTrackerSpacePointFinderCfg(flags, signature=signature))

    acc.merge(ITkftfCfg(trigflags, roisKey, signature, signatureName))

    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    acc.merge(ITkTrackParticleCnvAlgCfg(trigflags,
                                        name = "ITkTrigTrackParticleCnvAlg"+signature,
                                        TrackContainerName = trigflags.ITk.Tracking.ActiveConfig.trkTracks_FTF,
                                        xAODTrackParticlesFromTracksContainerName = trigflags.ITk.Tracking.ActiveConfig.tracks_FTF))
    

    if flags.Output.doWriteAOD:
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge(addToAOD(trigflags, ["xAOD::TrackParticleContainer#HLT_IDTrack_Muon_FTF"]))

    return acc

def ITkambiguitySolverAlgCfg(flags):
    acc = ComponentAccumulator()

    prefix="InDetTrigMT"

    from TrkConfig.TrkAmbiguitySolverConfig import ITkTrkAmbiguityScoreCfg, ITkTrkAmbiguitySolverCfg
    acc.merge(ITkTrkAmbiguityScoreCfg(flags, name = f"{prefix}TrkAmbiguityScore_{flags.ITk.Tracking.ActiveConfig.input_name}"))
    acc.merge(ITkTrkAmbiguitySolverCfg(flags, name  = f"{prefix}TrkAmbiguitySolver_{flags.ITk.Tracking.ActiveConfig.input_name}"))

    return acc

def ITktrigInDetPrecisionTrackingCfg( inflags, signatureName, in_view=True ):
    """ Generates precision tracking config, it is a primary config function """

    acc = ComponentAccumulator()
    flags = inflags.cloneAndReplace("ITk.Tracking.ActiveConfig", "Trigger.ITkTracking."+signatureName)

    acc.merge(ITkambiguitySolverAlgCfg(flags))

    from TrigInDetConfig.TrigInDetConfig import trackEFIDConverterCfg
    acc.merge(trackEFIDConverterCfg(flags))

    return acc
