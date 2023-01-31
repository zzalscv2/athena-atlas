#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# A minimal job that demonstrates what PerfMonMTSvc does
if __name__ == '__main__':

    # Import the common flags/services
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg

    # Set the necessary configuration flags
    # Process 100 events in 1 thread/slot and do full monitoring
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = 100
    flags.Concurrency.NumThreads = 1
    flags.PerfMon.doFullMonMT = True
    flags.PerfMon.OutputJSON = 'perfmonmt_test.json'
    flags.lock()

    # Set up the configuration and add the relevant services
    cfg = MainServicesCfg(flags)
    cfg.merge(PerfMonMTSvcCfg(flags))

    # Burn 100 +/- 1 ms per event
    CpuCruncherAlg = CompFactory.getComp('PerfMonTest::CpuCruncherAlg')
    cfg.addEventAlgo(CpuCruncherAlg('CpuCruncherAlg', MeanCpu = 100, RmsCpu = 1), sequenceName = 'AthAlgSeq')

    # Leak 10k ints per event, i.e. 40 KB
    LeakyAlg = CompFactory.getComp('PerfMonTest::LeakyAlg')
    cfg.addEventAlgo(LeakyAlg('LeakyAlg', LeakSize = 10000), sequenceName = 'AthAlgSeq')

    # Print the configuration and dump the flags
    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    # Run the job
    sc = cfg.run()

    # Exit as appropriate
    import sys
    sys.exit(not sc.isSuccess())
