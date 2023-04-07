# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType

def createInDetConfigFlags():
    icf = AthConfigFlags()

    # Detector flags
    # Turn running of the truth seeded pseudo tracking only for pileup on and off.
    # Only makes sense to run on RDO file where SplitDigi was used!
    icf.addFlag("InDet.doSplitReco", False)
    # Turn on running of PRD MultiTruthMaker
    icf.addFlag("InDet.doTruth", lambda prevFlags: prevFlags.Input.isMC)

    # defines if the X1X mode is used for the offline or not
    icf.addFlag("InDet.selectSCTIntimeHits", lambda prevFlags: (
        not(prevFlags.Beam.Type is BeamType.Cosmics or \
            prevFlags.Tracking.doVtxBeamSpot)))
    icf.addFlag("InDet.useDCS", True)
    icf.addFlag("InDet.usePixelDCS", lambda prevFlags: (
        prevFlags.InDet.useDCS and prevFlags.Detector.EnablePixel))
    icf.addFlag("InDet.useSctDCS", lambda prevFlags: (
        prevFlags.InDet.useDCS and prevFlags.Detector.EnableSCT))
    # Use old (non CoolVectorPayload) SCT Conditions
    icf.addFlag("InDet.ForceCoraCool", False)
    # Use new (CoolVectorPayload) SCT Conditions
    icf.addFlag("InDet.ForceCoolVectorPayload", False)
    # Turn on SCT_ModuleVetoSvc, allowing it to be configured later
    icf.addFlag("InDet.doSCTModuleVeto", False)
    # Enable check for dead modules and FEs
    icf.addFlag("InDet.checkDeadElementsOnTrack", True)
    # Turn running of Event Info TRT Occupancy Filling Alg on and off (also whether it is used in TRT PID calculation)
    icf.addFlag("InDet.doTRTGlobalOccupancy", False)
    icf.addFlag("InDet.noTRTTiming", lambda prevFlags:
                prevFlags.Beam.Type is BeamType.SingleBeam and
                prevFlags.Detector.EnableTRT)
    icf.addFlag("InDet.doTRTPhase", lambda prevFlags:
                prevFlags.Beam.Type is BeamType.Cosmics and
                prevFlags.Detector.EnableTRT)

    icf.addFlag("InDet.Tracking.writeSeedValNtuple", False) # Turn writing of seed validation ntuple on and off
    icf.addFlag("InDet.Tracking.writeExtendedPRDInfo", False)
    # Special pass using truth information for pattern recognition, runs in parallel to/instead of the first pass
    icf.addFlag("InDet.Tracking.doPseudoTracking", False)
    # Special pass using truth information for pattern recognition, removes assumed in-efficencies applied to PseudoTracking
    icf.addFlag("InDet.Tracking.doIdealPseudoTracking", False)
    # Save SiSP tracks (input to the ambiguity solver)
    icf.addFlag("InDet.Tracking.doStoreSiSPSeededTracks", False)
    # Skip ambiguity solver in hadronic ROI
    icf.addFlag("InDet.Tracking.doSkipAmbiROI", False)


    from InDetConfig.TrackingPassFlags import (
        createTrackingPassFlags, createHighPileupTrackingPassFlags,
        createMinBiasTrackingPassFlags, createLargeD0TrackingPassFlags,
        createR3LargeD0TrackingPassFlags, createLowPtLargeD0TrackingPassFlags,
        createLowPtTrackingPassFlags, createVeryLowPtTrackingPassFlags,
        createForwardTracksTrackingPassFlags, createBeamGasTrackingPassFlags,
        createVtxLumiTrackingPassFlags, createVtxBeamSpotTrackingPassFlags, createCosmicsTrackingPassFlags,
        createHeavyIonTrackingPassFlags, createPixelTrackingPassFlags, createDisappearingTrackingPassFlags,
        createSCTTrackingPassFlags, createTRTTrackingPassFlags, createTRTStandaloneTrackingPassFlags,
        createRobustRecoTrackingPassFlags)

    # Set up for first tracking pass, updated for second passes
    icf.addFlagsCategory("InDet.Tracking.MainPass",
                         createTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.HighPileupPass",
                         createHighPileupTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.MinBiasPass",
                         createMinBiasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.LargeD0Pass",
                         createLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.R3LargeD0Pass",
                         createR3LargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.LowPtLargeD0Pass",
                         createLowPtLargeD0TrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.LowPtPass",
                         createLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.VeryLowPtPass",
                         createVeryLowPtTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.ForwardPass",
                         createForwardTracksTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.BeamGasPass",
                         createBeamGasTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.VtxLumiPass",
                         createVtxLumiTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.VtxBeamSpotPass",
                         createVtxBeamSpotTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.CosmicsPass",
                         createCosmicsTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.HeavyIonPass",
                         createHeavyIonTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.PixelPass",
                         createPixelTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.DisappearingPass",
                         createDisappearingTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.SCTPass",
                         createSCTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.TRTPass",
                         createTRTTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.TRTStandalonePass",
                         createTRTStandaloneTrackingPassFlags, prefix=True)
    icf.addFlagsCategory("InDet.Tracking.RobustRecoPass",
                         createRobustRecoTrackingPassFlags, prefix=True)

    return icf
