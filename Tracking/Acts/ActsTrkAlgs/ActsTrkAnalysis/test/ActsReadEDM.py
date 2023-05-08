#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Detector.EnableITkPixel = True
    flags.Detector.EnableITkStrip = True
    flags.DQ.useTrigger = False
    flags.Output.HISTFileName = "ActsMonitoringOutput.root" 

    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = -1

    flags.addFlag("readClusters", False)
    flags.addFlag("readSpacePoints", False)
    flags.fillFromArgs()
    
    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))
    
    if flags.readClusters:
        from ActsConfig.ActsTrkAnalysisConfig import ActsTrkClusterAnalysisCfg
        acc.merge(ActsTrkClusterAnalysisCfg(flags))

    if flags.readSpacePoints:
        from ActsConfig.ActsTrkAnalysisConfig import ActsTrkSpacePointAnalysisCfg
        acc.merge(ActsTrkSpacePointAnalysisCfg(flags))

    acc.printConfig()
    status = acc.run()
    if status.isFailure():
        print("Problem while reading Acts EDM objects from AOD input file ...")
        import sys
        sys.exit(-1)
    
