# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys

from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.Enums import ProductionStep
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from SimuJobTransforms.CommonSimulationSteering import specialConfigPreInclude, specialConfigPostInclude


def setOverlayInputFiles(runArgs, configFlags, log):
    hasRDO_BKGInput = hasattr(runArgs, 'inputRDO_BKGFile')
    hasBS_SKIMInput = hasattr(runArgs, 'inputBS_SKIMFile')
    hasEVNT_Input   = hasattr(runArgs, 'inputEVNTFile')

    if configFlags.Common.ProductionStep == ProductionStep.Overlay and not hasattr(runArgs, 'inputHITSFile'):
        raise RuntimeError('No input HITS file defined')

    if hasRDO_BKGInput and hasBS_SKIMInput:
        raise RuntimeError('Both RDO_BKG and BS_SKIM are defined')
    if not hasRDO_BKGInput and not hasBS_SKIMInput:
        raise RuntimeError('Define one of RDO_BKG and BS_SKIM file types')

    if hasattr(runArgs, 'skipSecondaryEvents'):
        configFlags.Overlay.SkipSecondaryEvents = runArgs.skipSecondaryEvents

    if hasRDO_BKGInput:
        log.info('Running MC+MC overlay')
        configFlags.Overlay.DataOverlay = False
        configFlags.Input.isMC = True
        configFlags.Input.Files = runArgs.inputRDO_BKGFile
        if configFlags.Common.ProductionStep == ProductionStep.Overlay:
            configFlags.Input.SecondaryFiles = runArgs.inputHITSFile
        elif configFlags.Common.ProductionStep == ProductionStep.FastChain:
            if not hasEVNT_Input:
                raise RuntimeError('No input EVNT file defined')
            else:
                configFlags.Input.SecondaryFiles = runArgs.inputEVNTFile
        else:
            raise RuntimeError('No secondaryFiles are defined')

        # take MCChannelNumber from secondary input:
        configFlags.Input.MCChannelNumber = GetFileMD(configFlags.Input.SecondaryFiles).get("mc_channel_number", 0)

        # runNumber is MC channel number in reco
        if hasattr(runArgs, 'runNumber'):
            if configFlags.Input.MCChannelNumber != runArgs.runNumber:
                log.warning('Got different MC channel number (%d) from runNumber than from metadata (%d)', runArgs.runNumber, configFlags.Input.MCChannelNumber)
                configFlags.Input.MCChannelNumber = runArgs.runNumber
            else:
                log.info('MC channel number: %d', configFlags.Input.MCChannelNumber)
    else:
        log.info('Running MC+data overlay')
        configFlags.Overlay.DataOverlay = True
        configFlags.Input.isMC = False
        configFlags.Input.Files = runArgs.inputHITSFile
        configFlags.Input.SecondaryFiles = runArgs.inputBS_SKIMFile


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logOverlay = logging.getLogger('Overlay')
    logOverlay.info('****************** STARTING OVERLAY *****************')

    logOverlay.info('**** Transformation run arguments')
    logOverlay.info(str(runArgs))

    logOverlay.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    commonRunArgsToFlags(runArgs, ConfigFlags)

    ConfigFlags.Common.ProductionStep = ProductionStep.Overlay

    # Setting input files for Overlay
    setOverlayInputFiles(runArgs, ConfigFlags, logOverlay)

    # Setting output files for Overlay
    if hasattr(runArgs, 'outputRDOFile'):
        if runArgs.outputRDOFile == 'None':
            ConfigFlags.Output.RDOFileName = ''
        else:
            ConfigFlags.Output.RDOFileName = runArgs.outputRDOFile
    else:
        raise RuntimeError('No output RDO file defined')

    if hasattr(runArgs, 'outputRDO_SGNLFile'):
        ConfigFlags.Output.RDO_SGNLFileName = runArgs.outputRDO_SGNLFile

    # Autoconfigure enabled subdetectors
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        detectors = None

    # Setup digitization flags
    from Digitization.DigitizationConfigFlags import digitizationRunArgsToFlags
    digitizationRunArgsToFlags(runArgs, ConfigFlags)

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(ConfigFlags, detectors, use_metadata=True, toggle_geometry=True)

    # Disable LVL1 trigger if triggerConfig explicitly set to 'NONE'
    if hasattr(runArgs, 'triggerConfig') and runArgs.triggerConfig == 'NONE':
        ConfigFlags.Detector.EnableL1Calo = False

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(ConfigFlags, runArgs)

    # Special Configuration preInclude
    specialConfigPreInclude(ConfigFlags)

    # Pre-include
    processPreInclude(runArgs, ConfigFlags)

    # Pre-exec
    processPreExec(runArgs, ConfigFlags)

    # Lock flags
    ConfigFlags.lock()

    # Main overlay steering
    from OverlayConfiguration.OverlaySteering import OverlayMainCfg
    cfg = OverlayMainCfg(ConfigFlags)

    # Special message service configuration
    from Digitization.DigitizationSteering import DigitizationMessageSvcCfg
    cfg.merge(DigitizationMessageSvcCfg(ConfigFlags))

    # Special Configuration postInclude
    specialConfigPostInclude(ConfigFlags, cfg)

    # Post-include
    processPostInclude(runArgs, ConfigFlags, cfg)

    # Post-exec
    processPostExec(runArgs, ConfigFlags, cfg)

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(ConfigFlags, runArgs))

    # Run the final configuration
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
