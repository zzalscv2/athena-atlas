# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

def fromRunArgs(runArgs):

    from AthenaCommon.Logging import logging
    log = logging.getLogger('RAWtoTLA_AOD')
    log.info('****************** STARTING TLA RAW Decoding (RAWtoTLA_AOD) *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    import time
    timeStart = time.time()



    # some basic settings here...
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    # Input
    if hasattr(runArgs, 'inputBSFile'):
        log.warning("Enters the inputBSFile if")
        ConfigFlags.Input.Files = runArgs.inputBSFile

    # Output 
    if hasattr(runArgs, 'outputTLA_AODFile'):
        ConfigFlags.Output.AODFileName = runArgs.outputTLA_AODFile
        log.info("---------- Configured TLA AOD output")


    # Set non-default flags 
    ConfigFlags.Trigger.DecodeHLT = False

    # process pre-include/exec
    processPreInclude(runArgs, ConfigFlags)
    processPreExec(runArgs, ConfigFlags)
    # Lock flags
    ConfigFlags.lock()

    log.info("Configuring according to flag values listed below")
    ConfigFlags.dump()
    
    # import the main config
    from TrigTLAMonitoring.decodeBS_TLA_AOD import setupDecodeCfgCA as TLADecodeConfig
    cfg = TLADecodeConfig(ConfigFlags)

    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    cfg.merge(PoolWriteCfg(ConfigFlags))

    # process post-include/exec
    processPostInclude(runArgs, ConfigFlags, cfg)
    processPostExec(runArgs, ConfigFlags, cfg)

    # run the job
    import sys
    sys.exit(cfg.run().isFailure())

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run RAWtoTLA_AOD_skeleton in %d seconds", timeFinal - timeStart)

    import sys
    sys.exit(not sc.isSuccess())
