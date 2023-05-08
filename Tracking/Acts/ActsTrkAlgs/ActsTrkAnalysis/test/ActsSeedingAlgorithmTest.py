#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaCommon.Logging import logging
    # import os

    flags = initConfigFlags()
    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = False
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.DQ.useTrigger = False
    flags.ITk.doTruth = False
    flags.Exec.MaxEvents = 2
    flags.Output.HISTFileName = "ActsMonitoringOutput.root"

    # flags.Input.Files = ["../mc15_14TeV.600012.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.RDO.e8185_s3856_r13998/RDO.30640759._000008.pool.root.1"]

    flags.lock()

    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from InDetConfig.SiliconPreProcessing import ITkRecPreProcessingSiliconCfg
    acc.merge(ITkRecPreProcessingSiliconCfg(flags))

    from InDetConfig.InDetPrepRawDataFormationConfig import ITkInDetToXAODClusterConversionCfg
    acc.merge(ITkInDetToXAODClusterConversionCfg(flags))

    from SiSpacePointFormation.SiSpacePointFormationConfig import TrkToXAODSpacePointConversionCfg
    acc.merge(TrkToXAODSpacePointConversionCfg(flags))

    from InDetConfig.ITkTrackRecoConfig import CombinedTrackingPassFlagSets
    flags_set = CombinedTrackingPassFlagSets(flags)

    print(flags_set[0].dump())

    from ActsConfig.ActsTrkAnalysisConfig import ActsTrkSeedingAlgorithmAnalysisAlgCfg
    acc.merge(ActsTrkSeedingAlgorithmAnalysisAlgCfg(flags_set[0]))

    mlog = logging.getLogger("SeedingAlgorithmAnalysis")
    mlog.info("Configuring  SeedingAlgorithmAnalysis: ")
    printProperties(
      mlog,
      acc.getEventAlgo("ActsTrkSeedingAlgorithmAnalysis"),
      nestLevel=2,
      printDefaults=True,
    )

    flags.dump()

    # debug printout
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
