# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType, LHCPeriod

def createTrackingConfigFlags():
    icf = AthConfigFlags()

    def doLargeD0(flags):
        if flags.GeoModel.Run<=LHCPeriod.Run3:
           return not(flags.Beam.Type in [BeamType.SingleBeam, \
                                          BeamType.Cosmics] or \
                      flags.Reco.EnableHI or \
                      flags.Tracking.doHighPileup or \
                      flags.Tracking.doVtxLumi or \
                      flags.Tracking.doVtxBeamSpot)
        else: # LRT disabled by default for Run4 for now
            return False


    icf.addFlag("Tracking.doLargeD0", doLargeD0)
    icf.addFlag("Tracking.storeSeparateLargeD0Container", True)

    # The following flags are only used in InDet configurations for now
    # No corresponding ITk config is available yet

    # Turn running of doLargeD0 second pass down to 100 MeV on and off Turn running of doLargeD0 second pass on and off
    icf.addFlag("Tracking.doLowPtLargeD0", False)
    # Turn running of high pile-up reconstruction on and off
    icf.addFlag("Tracking.doHighPileup", False)
    # Special reconstruction for vertex lumi measurement
    icf.addFlag("Tracking.doVtxLumi", False)
    # Special reconstruction for vertex beamspot measurement
    icf.addFlag("Tracking.doVtxBeamSpot", False)

    return icf
