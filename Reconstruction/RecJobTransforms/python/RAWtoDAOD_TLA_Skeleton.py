# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

def fromRunArgs(runArgs):

    from AthenaCommon.Logging import logging
    log = logging.getLogger('RAWtoDAOD_TLA')
    log.info('****************** STARTING TLA RAW Decoding (RAWtoDAOD_TLA) *****************')

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
    if hasattr(runArgs, 'outputDAOD_TLAFile'):
        ConfigFlags.Output.AODFileName = runArgs.outputDAOD_TLAFile
        log.info("---------- Configured DAOD_TLA output")


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

    # process post-include/exec
    processPostInclude(runArgs, ConfigFlags, cfg)
    processPostExec(runArgs, ConfigFlags, cfg)

    # run the job
    import sys
    sys.exit(cfg.run().isFailure())

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run RAWtoDAOD_TLA_skeleton in %d seconds", timeFinal - timeStart)

    import sys
    sys.exit(not sc.isSuccess())
