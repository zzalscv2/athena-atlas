#!/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# file   AFP_DigitizationConfig_test.py
# author Petr Balek <petr.balek@cern.ch>
# date   2022-05-25

# brief  A script to test AFP_Digitization package. Mostly inspired by Digitization/DigitizationConfig_test.py, but includes switching AFP digitization on (it's off by default). To test it:
#           0. setup athena enviroment
#           1. run this script:
#              $ python AFP_DigitizationConfig_test.py
#           2. you may want to continue with reconstruction:
#              $ RecoSteeringTest.py --filesInput=myRDO.pool.root --RDO


if __name__ == "__main__":
    """Test AFP ComponentAccumulator Digitization configuration modules, inspired by Digitization/DigitizationConfig_test.py"""
    
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from Digitization.DigitizationSteering import DigitizationMainCfg, DigitizationMessageSvcCfg

    # Set up logging and new style config
    log.setLevel(DEBUG)

    # Configure
    ConfigFlags.Input.Files = ["test.pool.root"]
    ConfigFlags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-RUN2-09"
    ConfigFlags.GeoModel.Align.Dynamic = False
    ConfigFlags.Concurrency.NumThreads = 1
    ConfigFlags.Concurrency.NumConcurrentEvents=1
    ConfigFlags.Exec.MaxEvents=3
    ConfigFlags.Beam.NumberOfCollisions = 0.
    ConfigFlags.Output.RDOFileName = "myRDO.pool.root"

    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()

    # Construct our accumulator to run
    acc = DigitizationMainCfg(ConfigFlags)
    acc.merge(DigitizationMessageSvcCfg(ConfigFlags))

    # Dump config
    acc.getService("StoreGateSvc").Dump = True
    acc.getService("ConditionStore").Dump = True
    acc.printConfig(withDetails=True)
    ConfigFlags.dump()

    acc.foreach_component("*AFP*").OutputLevel=DEBUG

    # Execute and finish
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)
