# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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
    acc.merge(TRT_TrackExtensionAlgCfg(
        flags,
        InputTracksLocation = SiTrackCollection,
        ExtendedTracksLocation = ExtendedTracksMap))

    from InDetConfig.InDetExtensionProcessorConfig import (
        InDetExtensionProcessorCfg)
    acc.merge(InDetExtensionProcessorCfg(flags,
                                         TrackName = SiTrackCollection,
                                         NewTrackName = ExtendedTrackCollection,
                                         ExtensionMap = ExtendedTracksMap))

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        acc.merge(InDetTrackTruthCfg(
            flags,
            Tracks = ExtendedTrackCollection,
            DetailedTruth = ExtendedTrackCollection+"DetailedTruth",
            TracksTruth = ExtendedTrackCollection+"TruthCollection"))


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
    from InDetConfig.TRT_TrackExtensionAlgConfig import (
        TRT_Phase_TrackExtensionAlgCfg)
    acc.merge(TRT_Phase_TrackExtensionAlgCfg(
        flags,
        InputTracksLocation = SiTrackCollection,
        ExtendedTracksLocation = ExtendedTracksMap))

    from InDetConfig.InDetExtensionProcessorConfig import (
        InDetExtensionProcessorCfg)
    acc.merge(InDetExtensionProcessorCfg(flags,
                                         name = "InDetExtensionProcessorPhase",
                                         TrackName = SiTrackCollection,
                                         NewTrackName = ExtendedTrackCollection,
                                         ExtensionMap = ExtendedTracksMap))

    return acc

##########################################################################################################################

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    numThreads=1
    flags.Concurrency.NumThreads=numThreads
    flags.Concurrency.NumConcurrentEvents=numThreads

    flags.Detector.GeometryPixel = True 
    flags.Detector.GeometrySCT = True
    flags.Detector.GeometryTRT = True

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    flags.Tracking.doTRTExtension = True

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    top_acc.merge(BeamSpotCondAlgCfg(flags))

    if "EventInfo" not in flags.Input.Collections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        top_acc.merge(EventInfoCnvAlgCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(flags))

    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    top_acc.merge(PixelReadoutGeometryCfg(flags))
    top_acc.merge(SCT_ReadoutGeometryCfg(flags))

    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    top_acc.merge(TRT_ReadoutGeometryCfg( flags ))

    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    top_acc.merge(BeamPipeGeometryCfg(flags))

    InputCollections = []

    ResolvedTracks = 'ResolvedTracks'
    InDetSpSeededTracksKey = 'SiSPSeededTracks'
    ExtendedTrackCollection = 'ExtendedTracks'
    ExtendedTracksMap = 'ExtendedTracksMap'

    #################### Additional Configuration  ########################
    #######################################################################
    ################# TRTPreProcessing Configuration ######################
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    top_acc.merge(TRTPreProcessingCfg(flags))

    ################ TRTSegmentFinding Configuration ######################
    from InDetConfig.TRTSegmentFindingConfig import TRTSegmentFindingCfg
    top_acc.merge(TRTSegmentFindingCfg(flags))

    ############### SiliconPreProcessing Configuration ####################
    from InDetConfig.SiliconPreProcessing import InDetRecPreProcessingSiliconCfg
    top_acc.merge(InDetRecPreProcessingSiliconCfg(flags))

    ####################### TrackingSiPattern #############################
    from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
    top_acc.merge(TrackingSiPatternCfg(
        flags,
        InputCollections = InputCollections,
        ResolvedTrackCollectionKey = ResolvedTracks,
        SiSPSeededTrackCollectionKey = InDetSpSeededTracksKey))

    ########################### TRTExtension  #############################
    top_acc.merge(NewTrackingTRTExtensionCfg(
        flags,
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
