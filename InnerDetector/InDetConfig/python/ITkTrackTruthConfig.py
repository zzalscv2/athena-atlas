# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# -------------------------------------------------------------------------
#
# ------- fragment to handle track truth association
#
# -------------------------------------------------------------------------

def ITkTrackTruthCfg(flags,
                     Tracks = "CombinedITkTracks",
                     DetailedTruth = "CombinedITkTracksDetailedTruth",
                     TracksTruth = "CombinedITkTracksTruthCollection"):
    acc = ComponentAccumulator()
    #
    # --- Enable the detailed track truth
    #
    from InDetConfig.InDetTruthAlgsConfig import ITkDetailedTrackTruthMakerCfg
    acc.merge(ITkDetailedTrackTruthMakerCfg(flags, Tracks, DetailedTruth))
    #
    # --- Detailed to old TrackTruth
    #
    from TrkConfig.TrkTruthAlgsConfig import ITkTrackTruthSimilaritySelectorCfg
    acc.merge(ITkTrackTruthSimilaritySelectorCfg(flags,
                                                 DetailedTrackTruthName=DetailedTruth,
                                                 OutputName=TracksTruth))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    numThreads=1
    ConfigFlags.Concurrency.NumThreads=numThreads
    ConfigFlags.Concurrency.NumConcurrentEvents=numThreads # Might change this later, but good enough for the moment.

    ConfigFlags.Detector.GeometryITkPixel = True
    ConfigFlags.Detector.GeometryITkStrip = True

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.lock()
    ConfigFlags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(ConfigFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(ConfigFlags))

    ################## SiliconPreProcessing Configurations ###################
    from InDetConfig.SiliconPreProcessing import ITkRecPreProcessingSiliconCfg
    top_acc.merge(ITkRecPreProcessingSiliconCfg(ConfigFlags))
    
    #//// TrackingSiPatternConfig configurations from Temporary location /////
    ################# SiSPSeededTrackFinder Configurations ###################

    InputCollections = []

    SiSPSeededTrackCollectionKey = 'SiSPSeededPixelTracks'
    ResolvedTrackCollectionKey = 'ResolvedPixelTracks'
    from InDetConfig.ITkTrackingSiPatternConfig import ITkSiSPSeededTrackFinderCfg
    top_acc.merge(ITkSiSPSeededTrackFinderCfg( ConfigFlags,
                                               InputCollections = InputCollections, 
                                               SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey))
    ##########################################################################
    #################### InDetTrackTruth Configurations ######################

    InputTrackCollection = 'SiSPSeededPixelTracks'
    InputDetailedTrackTruth = 'DetailedTrackTruth'
    InputTrackCollectionTruth = 'TrackTruthCollection'
    
    top_acc.merge(ITkTrackTruthCfg(flags=ConfigFlags,
                                   Tracks = InputTrackCollection,
                                   DetailedTruth = InputDetailedTrackTruth,
                                   TracksTruth = InputTrackCollectionTruth))
    #################################################################
    top_acc.printConfig()
    top_acc.run(25)
    top_acc.store(open("test_TrackTruthConfig.pkl", "wb"))
