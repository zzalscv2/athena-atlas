# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
from SimuJobTransforms.CommonSimulationSteering import specialConfigPreInclude, specialConfigPostInclude


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logFastChain = logging.getLogger('FastChainSkeleton')
    logFastChain.info('****************** STARTING FastChain Simulation *****************')

    logFastChain.info('**** Transformation run arguments')
    logFastChain.info(str(runArgs))

    logFastChain.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from SimulationConfig.SimEnums import SimulationFlavour
    commonRunArgsToFlags(runArgs, ConfigFlags)

    # Set ProductionStep
    from AthenaConfiguration.Enums import ProductionStep
    ConfigFlags.Common.ProductionStep = ProductionStep.FastChain

    # Set simulator
    if hasattr(runArgs, 'simulator'):
        ConfigFlags.Sim.ISF.Simulator = SimulationFlavour(runArgs.simulator)

    # This is ISF
    ConfigFlags.Sim.ISFRun = True

    # Set input files
    if hasattr(runArgs, 'inputRDO_BKGFile') or hasattr(runArgs, 'inputBS_SKIMFile'):
        # Set inputs for Overlay
        from OverlayConfiguration.OverlaySkeleton import setOverlayInputFiles
        setOverlayInputFiles(runArgs, ConfigFlags, logFastChain)
        ConfigFlags.Overlay.FastChain = True
    else:
        # Setting input files for FastChain without overlay
        if hasattr(runArgs, 'inputEVNTFile'):
            ConfigFlags.Input.Files = runArgs.inputEVNTFile
        else:
            raise RuntimeError('No input EVNT file defined')

    # Setting output files (including for Overlay) for FastChain
    if hasattr(runArgs, 'outputHITSFile'):
        ConfigFlags.Output.HITSFileName = runArgs.outputHITSFile

    if hasattr(runArgs, 'outputRDOFile'):
        if runArgs.outputRDOFile == 'None':
            ConfigFlags.Output.RDOFileName = ''
        else:
            ConfigFlags.Output.RDOFileName = runArgs.outputRDOFile
    else:
        raise RuntimeError('No outputRDOFile defined')

    if ConfigFlags.Overlay.FastChain:
        if hasattr(runArgs, 'outputRDO_SGNLFile'):
            ConfigFlags.Output.RDO_SGNLFileName = runArgs.outputRDO_SGNLFile

    if hasattr(runArgs, 'conditionsTag'):
        ConfigFlags.IOVDb.GlobalTag = runArgs.conditionsTag

    # Generate detector list (must be after input setting)
    from SimuJobTransforms.SimulationHelpers import getDetectorsFromRunArgs
    detectors = getDetectorsFromRunArgs(ConfigFlags, runArgs)

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(ConfigFlags, detectors, toggle_geometry=True)

    # Common simulation runtime arguments
    from SimulationConfig.SimConfigFlags import simulationRunArgsToFlags
    simulationRunArgsToFlags(runArgs, ConfigFlags)

    # Setup digitization flags
    from Digitization.DigitizationConfigFlags import digitizationRunArgsToFlags
    digitizationRunArgsToFlags(runArgs, ConfigFlags)

    # Setup flags for pile-up
    if not ConfigFlags.Overlay.FastChain:
        # Setup common digitization flags
        from Digitization.DigitizationConfigFlags import setupDigitizationFlags
        setupDigitizationFlags(runArgs, ConfigFlags)
        logFastChain.info('Running with pile-up: %s', ConfigFlags.Digitization.PileUp)

    # Disable LVL1 trigger if triggerConfig explicitly set to 'NONE'
    if hasattr(runArgs, 'triggerConfig') and runArgs.triggerConfig == 'NONE':
        ConfigFlags.Detector.EnableL1Calo = False

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(ConfigFlags, runArgs)

    # Pre-include
    processPreInclude(runArgs, ConfigFlags)

    # Special Configuration preInclude
    specialConfigPreInclude(ConfigFlags)

    # Pre-exec
    processPreExec(runArgs, ConfigFlags)

    if not ConfigFlags.Overlay.FastChain:
        # Load pile-up stuff after pre-include/exec to ensure everything is up-to-date
        from Digitization.DigitizationConfigFlags import pileupRunArgsToFlags
        pileupRunArgsToFlags(runArgs, ConfigFlags)

        # Setup pile-up profile
        if ConfigFlags.Digitization.PileUp:
            from RunDependentSimComps.PileUpUtils import setupPileUpProfile
            setupPileUpProfile(ConfigFlags)

    ConfigFlags.Sim.DoFullChain = True

    # Lock flags
    ConfigFlags.lock()

    if ConfigFlags.Digitization.PileUp:
        from Digitization.PileUpConfig import PileUpEventLoopMgrCfg
        cfg = MainServicesCfg(ConfigFlags, LoopMgr="PileUpEventLoopMgr")
        cfg.merge(PileUpEventLoopMgrCfg(ConfigFlags))
    else:
        cfg = MainServicesCfg(ConfigFlags)

    cfg.merge(PoolReadCfg(ConfigFlags))
    cfg.merge(PoolWriteCfg(ConfigFlags))

    # Simulation
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(ConfigFlags))

    if (not ConfigFlags.Overlay.FastChain and "xAOD::EventInfo#EventInfo" in ConfigFlags.Input.TypedCollections) \
        or (ConfigFlags.Overlay.FastChain and "xAOD::EventInfo#EventInfo" in ConfigFlags.Input.SecondaryTypedCollections):
        # Make sure signal EventInfo is rebuilt from event context
        # TODO: this is probably not needed, but keeping it to be in sync with standard simulation
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
        cfg.merge(EventInfoUpdateFromContextAlgCfg(ConfigFlags))

    if ConfigFlags.Overlay.FastChain:
        # CopyMcEventCollection should be before Kernel
        from OverlayCopyAlgs.OverlayCopyAlgsConfig import CopyMcEventCollectionCfg
        cfg.merge(CopyMcEventCollectionCfg(ConfigFlags))

    from ISF_Config.ISF_MainConfig import ISF_KernelCfg
    cfg.merge(ISF_KernelCfg(ConfigFlags))

    # Main Overlay Steering
    if ConfigFlags.Overlay.FastChain:
        from OverlayConfiguration.OverlaySteering import OverlayMainContentCfg
        cfg.merge(OverlayMainContentCfg(ConfigFlags))
    else:
        from Digitization.DigitizationSteering import DigitizationMainContentCfg
        cfg.merge(DigitizationMainContentCfg(ConfigFlags))

    # Special message service configuration
    from Digitization.DigitizationSteering import DigitizationMessageSvcCfg
    cfg.merge(DigitizationMessageSvcCfg(ConfigFlags))

    # Special Configuration postInclude
    specialConfigPostInclude(ConfigFlags, cfg)

    # Post-include
    processPostInclude(runArgs, ConfigFlags, cfg)

    # Post-exec
    processPostExec(runArgs, ConfigFlags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
