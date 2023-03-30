# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkFastTrackFinderStandaloneCfg(flags, SiSPSeededTrackCollectionKey = None):
    acc = ComponentAccumulator()

    from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolNoHoleSearchCfg
    ITkTrackSummaryTool = acc.popToolsAndMerge(ITkTrackSummaryToolNoHoleSearchCfg(flags))
    acc.addPublicTool(ITkTrackSummaryTool)

    from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
    ITkSiTrackMakerTool = acc.popToolsAndMerge(ITkSiTrackMaker_xkCfg(flags))
    acc.addPublicTool(ITkSiTrackMakerTool)

    acc.addPublicTool( CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter" ) )

    from RegionSelector.RegSelToolConfig import (regSelTool_ITkStrip_Cfg, regSelTool_ITkPixel_Cfg)
    pixRegSelTool = acc.popToolsAndMerge( regSelTool_ITkPixel_Cfg(flags) )
    sctRegSelTool = acc.popToolsAndMerge( regSelTool_ITkStrip_Cfg(flags) )
    
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
    acc.merge(TriggerHistSvcConfig(flags))
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
                                            Triplet_D0Max            = 4,
                                            Triplet_MaxBufferLength  = 1,
                                            Triplet_MinPtFrac        = 0.7,
                                            UseTrigSeedML            = 1,
                                            doResMon                 = False,
                                            doSeedRedundancyCheck    = True,
                                            pTmin                    = flags.ITk.Tracking.ActiveConfig.minPT[0],
                                            useNewLayerNumberScheme  = True,
                                            MinHits                  = 3,
                                            useGPU                   = False,
                                            ITkMode                  = True, # Allows ftf to use the new TrigTrackSeedGenerator for ITk
                                            StandaloneMode           = True, # Allows ftf to be run as an offline algorithm with reco_tf
                                            doTrackRefit             = False,
                                            FreeClustersCut          = 1,
                                            MonTool                  = monTool)

    acc.addEventAlgo( ftf, primary=True )
    
    return acc


