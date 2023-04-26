# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# -------------------------------------------------------------------------
#
# ------- fragment to handle track truth association
#
# -------------------------------------------------------------------------

def ITkTrackTruthCfg(flags,
                     Tracks = "CombinedITkTracks",
                     DetailedTruth = "DetailedTrackTruth",
                     TracksTruth = "TrackTruthCollection"):
    acc = ComponentAccumulator()
    #
    # --- Enable the detailed track truth
    #
    from InDetConfig.InDetTruthAlgsConfig import ITkDetailedTrackTruthMakerCfg
    acc.merge(ITkDetailedTrackTruthMakerCfg(flags,
                                            TrackCollectionName = Tracks,
                                            DetailedTrackTruthName = DetailedTruth))
    #
    # --- Detailed to old TrackTruth
    #
    from TrkConfig.TrkTruthAlgsConfig import ITkTrackTruthSimilaritySelectorCfg
    acc.merge(ITkTrackTruthSimilaritySelectorCfg(flags,
                                                 DetailedTrackTruthName = DetailedTruth,
                                                 OutputName = TracksTruth))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    numThreads=1
    flags.Concurrency.NumThreads=numThreads
    flags.Concurrency.NumConcurrentEvents=numThreads # Might change this later, but good enough for the moment.

    flags.Detector.GeometryITkPixel = True
    flags.Detector.GeometryITkStrip = True

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    ################## SiliconPreProcessing Configurations ###################
    from InDetConfig.SiliconPreProcessing import ITkRecPreProcessingSiliconCfg
    top_acc.merge(ITkRecPreProcessingSiliconCfg(flags))
    
    #//// TrackingSiPatternConfig configurations from Temporary location /////
    ################# SiSPSeededTrackFinder Configurations ###################

    InputCollections = []

    SiSPSeededTrackCollectionKey = 'SiSPSeededPixelTracks'
    ResolvedTrackCollectionKey = 'ResolvedPixelTracks'
    from InDetConfig.SiSPSeededTrackFinderConfig import ITkSiSPSeededTrackFinderCfg
    top_acc.merge(ITkSiSPSeededTrackFinderCfg( flags,
                                               InputCollections = InputCollections, 
                                               SiSPSeededTrackCollectionKey = SiSPSeededTrackCollectionKey))
    ##########################################################################
    #################### InDetTrackTruth Configurations ######################

    InputTrackCollection = 'SiSPSeededPixelTracks'
    InputDetailedTrackTruth = 'DetailedTrackTruth'
    InputTrackCollectionTruth = 'TrackTruthCollection'
    
    top_acc.merge(ITkTrackTruthCfg(flags,
                                   Tracks = InputTrackCollection,
                                   DetailedTruth = InputDetailedTrackTruth,
                                   TracksTruth = InputTrackCollectionTruth))
    #################################################################
    top_acc.printConfig()
    top_acc.run(25)
    top_acc.store(open("test_TrackTruthConfig.pkl", "wb"))
