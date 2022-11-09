# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

##########################################################################################################################
# ------------------------------------------------------------
#
# ----------- Setup TRT Extension for New tracking
#
# ------------------------------------------------------------
def NewTrackingTRTExtensionCfg(flags,
                               SiTrackCollection = None,
                               ExtendedTrackCollection = None,
                               ExtendedTracksMap = None):
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    acc = TRTPreProcessingCfg(flags)
    #
    # Track extension to TRT algorithm
    #
    from InDetConfig.TRT_TrackExtensionAlgConfig import TRT_TrackExtensionAlgCfg
    acc.merge(TRT_TrackExtensionAlgCfg(flags,
                                       InputTracksLocation = SiTrackCollection,
                                       ExtendedTracksLocation = ExtendedTracksMap))

    from InDetConfig.InDetExtensionProcessorConfig import InDetExtensionProcessorCfg
    acc.merge(InDetExtensionProcessorCfg(flags,
                                         TrackName = SiTrackCollection,
                                         NewTrackName = ExtendedTrackCollection,
                                         ExtensionMap = ExtendedTracksMap))

    return acc

def NewTrackingTRTExtensionPhaseCfg(flags,
                                    SiTrackCollection = None,
                                    ExtendedTrackCollection = None,
                                    ExtendedTracksMap = None):
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    acc = TRTPreProcessingCfg(flags)
    #
    # Track extension to TRT algorithm
    #
    from InDetConfig.TRT_TrackExtensionAlgConfig import TRT_Phase_TrackExtensionAlgCfg
    acc.merge(TRT_Phase_TrackExtensionAlgCfg(flags,
                                             InputTracksLocation = SiTrackCollection,
                                             ExtendedTracksLocation = ExtendedTracksMap))

    from InDetConfig.InDetExtensionProcessorConfig import InDetExtensionProcessorCfg
    acc.merge(InDetExtensionProcessorCfg(flags,
                                         name = "InDetExtensionProcessorPhase",
                                         TrackName = SiTrackCollection,
                                         NewTrackName = ExtendedTrackCollection,
                                         ExtensionMap = ExtendedTracksMap))

    return acc

##########################################################################################################################

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    numThreads=1
    ConfigFlags.Concurrency.NumThreads=numThreads
    ConfigFlags.Concurrency.NumConcurrentEvents=numThreads

    ConfigFlags.Detector.GeometryPixel = True 
    ConfigFlags.Detector.GeometrySCT = True
    ConfigFlags.Detector.GeometryTRT = True

    # Disable calo for this test
    ConfigFlags.Detector.EnableCalo = False

    ConfigFlags.InDet.Tracking.doTRTExtension = True
    ConfigFlags.InDet.Tracking.holeSearchInGX2Fit = True

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.lock()
    ConfigFlags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(ConfigFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(ConfigFlags))

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    top_acc.merge(BeamSpotCondAlgCfg(ConfigFlags))

    if "EventInfo" not in ConfigFlags.Input.Collections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        top_acc.merge(EventInfoCnvAlgCfg(ConfigFlags))

    if ConfigFlags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(ConfigFlags))

    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    top_acc.merge(PixelReadoutGeometryCfg(ConfigFlags))
    top_acc.merge(SCT_ReadoutGeometryCfg(ConfigFlags))

    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    top_acc.merge(TRT_ReadoutGeometryCfg( ConfigFlags ))

    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    top_acc.merge(BeamPipeGeometryCfg(ConfigFlags))

    InputCollections = []

    ResolvedTracks = 'ResolvedTracks'
    InDetSpSeededTracksKey = 'SiSPSeededTracks'
    ExtendedTrackCollection = 'ExtendedTracks'
    ExtendedTracksMap = 'ExtendedTracksMap'

    #################### Additional Configuration  ########################
    #######################################################################
    ################# TRTPreProcessing Configuration ######################
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    top_acc.merge(TRTPreProcessingCfg(ConfigFlags))

    ################ TRTSegmentFinding Configuration ######################
    from InDetConfig.TRTSegmentFindingConfig import TRTSegmentFindingCfg
    top_acc.merge(TRTSegmentFindingCfg( ConfigFlags,
                                        extension = "",
                                        InputCollections = InputCollections,
                                        BarrelSegments = 'TRTSegments'))

    ############### SiliconPreProcessing Configuration ####################
    from InDetConfig.SiliconPreProcessing import InDetRecPreProcessingSiliconCfg
    top_acc.merge(InDetRecPreProcessingSiliconCfg(ConfigFlags))

    ####################### TrackingSiPattern #############################
    from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
    top_acc.merge(TrackingSiPatternCfg( ConfigFlags,
                                        InputCollections = InputCollections,
                                        ResolvedTrackCollectionKey = ResolvedTracks,
                                        SiSPSeededTrackCollectionKey = InDetSpSeededTracksKey))

    ########################### TRTExtension  #############################
    top_acc.merge(NewTrackingTRTExtensionCfg(ConfigFlags,
                                             SiTrackCollection = ResolvedTracks,
                                             ExtendedTrackCollection = ExtendedTrackCollection, 
                                             ExtendedTracksMap = ExtendedTracksMap))
    #######################################################################

    iovsvc = top_acc.getService('IOVDbSvc')
    iovsvc.OutputLevel=5
    #
    top_acc.printConfig()
    top_acc.run(25)
    top_acc.store(open("test_TRTExtensionConfig.pkl", "wb"))
