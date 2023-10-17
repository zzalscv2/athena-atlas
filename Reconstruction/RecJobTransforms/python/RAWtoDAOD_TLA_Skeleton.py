# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from RecJobTransforms.RecoSteering import RecoSteering

from AthenaCommon.Logging import logging
log = logging.getLogger('RAWtoDAOD_TLA')


def configureFlags(runArgs):
    # some basic settings here...
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)
    from RecJobTransforms.RecoConfigFlags import recoRunArgsToFlags
    recoRunArgsToFlags(runArgs, flags)

    # Input
    if hasattr(runArgs, 'inputBSFile'):
        log.warning("Enters the inputBSFile if")
        flags.Input.Files = runArgs.inputBSFile

    # Output 
    if hasattr(runArgs, 'outputDAOD_TLAFile'):
        flags.Output.AODFileName = runArgs.outputDAOD_TLAFile
        log.info("---------- Configured DAOD_TLA output")


    # Set non-default flags 
    flags.Trigger.decodeHLT=False
    flags.Trigger.doLVL1=False
    flags.Trigger.DecisionMakerValidation.Execute = False
    flags.Trigger.doNavigationSlimming = False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doCTP=False
    flags.Trigger.AODEDMSet='PhysicsTLA'

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep=ProductionStep.Reconstruction

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import disableDetectors, allDetectors
    disableDetectors(
        flags, toggle_geometry=True,
        detectors=allDetectors,
    )

    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # process pre-include/exec
    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    return flags



def fromRunArgs(runArgs):

    log.info('****************** STARTING TLA RAW Decoding (RAWtoDAOD_TLA) *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    import time
    timeStart = time.time()

    flags = configureFlags(runArgs)
    log.info("Configuring according to flag values listed below")
    flags.dump()

    cfg = RecoSteering(flags)

    # import the TLA decoding
    cfg.flagPerfmonDomain('Trigger')
    from TrigTLAMonitoring.decodeBS_TLA_AOD import outputCfg
    cfg.merge( outputCfg(flags) )

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    from AthenaCommon.Constants import INFO
    if flags.Exec.OutputLevel <= INFO:
        cfg.printConfig()

    # Run the final accumulator
    sc = cfg.run()
    timeFinal = time.time()
    log.info("Run RAWtoDAOD_TLA_skeleton in %d seconds", timeFinal - timeStart)

    import sys
    sys.exit(sc.isFailure())
