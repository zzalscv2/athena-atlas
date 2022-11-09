# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

def TRTSegmentFindingCfg(flags, extension = "",
                         InputCollections = None,
                         BarrelSegments = None):
    acc = ComponentAccumulator()

    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if InputCollections is not None:
        from InDetConfig.InDetTrackPRD_AssociationConfig import InDetTrackPRD_AssociationCfg
        acc.merge(InDetTrackPRD_AssociationCfg(flags,
                                               name = 'InDetSegmentTrackPRD_Association' + extension,
                                               AssociationMapName = 'InDetSegmentPRDtoTrackMap' + extension,
                                               TracksName = list(InputCollections)))

    #
    # --- TRT track reconstruction
    #
    from InDetConfig.TRT_TrackSegmentsFinderConfig import TRT_TrackSegmentsFinderCfg
    acc.merge(TRT_TrackSegmentsFinderCfg(flags, name = 'InDetTRT_TrackSegmentsFinder'+extension,
                                         extension = extension,
                                         SegmentsLocation = BarrelSegments,
                                         InputCollections = InputCollections))
    #
    # --- load TRT validation alg
    #
    
    if flags.InDet.doTruth and flags.Beam.Type is not BeamType.Cosmics:
        from InDetConfig.InDetSegmentDriftCircleAssValidationConfig import SegmentDriftCircleAssValidationCfg
        acc.merge(SegmentDriftCircleAssValidationCfg(flags,
                                                     name = "InDetSegmentDriftCircleAssValidation"+extension,
                                                     extension = extension))
    
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files=defaultTestFiles.RDO_RUN2

    # disable calo for this test
    flags.Detector.EnableCalo = False

    numThreads=1
    flags.Concurrency.NumThreads=numThreads
    flags.Concurrency.NumConcurrentEvents=numThreads # Might change this later, but good enough for the moment.

    flags = flags.cloneAndReplace("InDet.Tracking.ActivePass","InDet.Tracking.MainPass")

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    # NewTracking collection keys
    InputCombinedInDetTracks = []

    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    top_acc.merge(TRTPreProcessingCfg(flags))

    top_acc.merge(TRTSegmentFindingCfg( flags,
                                        extension = "",
                                        InputCollections = InputCombinedInDetTracks,
                                        BarrelSegments = 'TRTSegments'))
    #############################################################################

    iovsvc = top_acc.getService('IOVDbSvc')
    iovsvc.OutputLevel=5

    top_acc.store(open("test_TRTSegmentFinding.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(5)
        sys.exit(not sc.isSuccess())
