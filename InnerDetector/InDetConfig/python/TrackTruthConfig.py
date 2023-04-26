# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# -------------------------------------------------------------------------
#
# ------- fragment to handle track truth association
#
# -------------------------------------------------------------------------


def InDetTrackTruthCfg(flags,
                       Tracks="CombinedInDetTracks",
                       DetailedTruth="DetailedTrackTruth",
                       TracksTruth="TrackTruthCollection"):
    acc = ComponentAccumulator()
    #
    # --- Enable the detailed track truth
    #
    from InDetConfig.InDetTruthAlgsConfig import InDetDetailedTrackTruthMakerCfg
    acc.merge(InDetDetailedTrackTruthMakerCfg(
        flags,
        TrackCollectionName=Tracks,
        DetailedTrackTruthName=DetailedTruth))
    #
    # --- Detailed to old TrackTruth
    #
    from TrkConfig.TrkTruthAlgsConfig import TrackTruthSimilaritySelectorCfg
    acc.merge(TrackTruthSimilaritySelectorCfg(
        flags,
        DetailedTrackTruthName=DetailedTruth,
        OutputName=TracksTruth))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    numThreads = 1
    flags.Concurrency.NumThreads = numThreads
    # Might change this later, but good enough for the moment.
    flags.Concurrency.NumConcurrentEvents = numThreads

    flags.Detector.GeometryPixel = True
    flags.Detector.GeometrySCT = True
    flags.Detector.GeometryTRT = True

    flags.Tracking.doPixelClusterSplitting = True

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    ################## SiliconPreProcessing Configurations ###################
    from InDetConfig.SiliconPreProcessing import InDetRecPreProcessingSiliconCfg
    top_acc.merge(InDetRecPreProcessingSiliconCfg(flags))
    #################### TRTPreProcessing Configurations #####################
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    top_acc.merge(TRTPreProcessingCfg(flags))

    # //// TrackingSiPatternConfig configurations from Temporary location /////
    ################# SiSPSeededTrackFinder Configurations ###################

    InputCollections = []

    SiSPSeededTrackCollectionKey = 'SiSPSeededPixelTracks'
    ResolvedTrackCollectionKey = 'ResolvedPixelTracks'
    from InDetConfig.SiSPSeededTrackFinderConfig import SiSPSeededTrackFinderCfg
    top_acc.merge(SiSPSeededTrackFinderCfg(
        flags,
        InputCollections=InputCollections,
        SiSPSeededTrackCollectionKey=SiSPSeededTrackCollectionKey))
    ##########################################################################
    #################### InDetTrackTruth Configurations ######################

    InputTrackCollection = 'SiSPSeededPixelTracks'
    InputDetailedTrackTruth = 'DetailedTrackTruth'
    InputTrackCollectionTruth = 'TrackTruthCollection'

    top_acc.merge(InDetTrackTruthCfg(
        flags,
        Tracks=InputTrackCollection,
        DetailedTruth=InputDetailedTrackTruth,
        TracksTruth=InputTrackCollectionTruth))
    #################################################################
    top_acc.printConfig()
    top_acc.run(25)
    top_acc.store(open("test_TrackTruthConfig.pkl", "wb"))
