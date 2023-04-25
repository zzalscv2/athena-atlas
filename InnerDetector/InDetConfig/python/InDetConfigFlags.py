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

    return icf
