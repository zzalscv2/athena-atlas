# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from SimuJobTransforms.CommonSimulationSteering import specialConfigPreInclude, specialConfigPostInclude


# temporarily force no global config flags
from AthenaConfiguration import AllConfigFlags
del AllConfigFlags.ConfigFlags


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logFastChain = logging.getLogger('FastChainSkeleton')
    logFastChain.info('****************** STARTING FastChain Simulation *****************')

    logFastChain.info('**** Transformation run arguments')
    logFastChain.info(str(runArgs))

    logFastChain.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from SimulationConfig.SimEnums import SimulationFlavour
    commonRunArgsToFlags(runArgs, flags)

    # Set ProductionStep
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.FastChain

    # Set simulator
    if hasattr(runArgs, 'simulator'):
        flags.Sim.ISF.Simulator = SimulationFlavour(runArgs.simulator)

    # This is ISF
    flags.Sim.ISFRun = True

    # Set input files
    if hasattr(runArgs, 'inputRDO_BKGFile') or hasattr(runArgs, 'inputBS_SKIMFile'):
        # Set inputs for Overlay
        from OverlayConfiguration.OverlaySkeleton import setOverlayInputFiles
        setOverlayInputFiles(runArgs, flags, logFastChain)
        flags.Overlay.FastChain = True
    else:
        # Setting input files for FastChain without overlay
        if hasattr(runArgs, 'inputEVNTFile'):
            flags.Input.Files = runArgs.inputEVNTFile
        else:
            raise RuntimeError('No input EVNT file defined')

    # Setting output files (including for Overlay) for FastChain
    if hasattr(runArgs, 'outputHITSFile'):
        flags.Output.HITSFileName = runArgs.outputHITSFile

    if hasattr(runArgs, 'outputRDOFile'):
        if runArgs.outputRDOFile == 'None':
            flags.Output.RDOFileName = ''
        else:
            flags.Output.RDOFileName = runArgs.outputRDOFile
    else:
        raise RuntimeError('No outputRDOFile defined')

    if flags.Overlay.FastChain:
        if hasattr(runArgs, 'outputRDO_SGNLFile'):
            flags.Output.RDO_SGNLFileName = runArgs.outputRDO_SGNLFile

    if hasattr(runArgs, 'conditionsTag'):
        flags.IOVDb.GlobalTag = runArgs.conditionsTag

    # Generate detector list (must be after input setting)
    from SimuJobTransforms.SimulationHelpers import getDetectorsFromRunArgs
    detectors = getDetectorsFromRunArgs(flags, runArgs)

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, toggle_geometry=True)

    # Common simulation runtime arguments
    from SimulationConfig.SimConfigFlags import simulationRunArgsToFlags
    simulationRunArgsToFlags(runArgs, flags)

    # Setup digitization flags
    from Digitization.DigitizationConfigFlags import digitizationRunArgsToFlags
    digitizationRunArgsToFlags(runArgs, flags)

    # Setup flags for pile-up
    if not flags.Overlay.FastChain:
        # Setup common digitization flags
        from Digitization.DigitizationConfigFlags import setupDigitizationFlags
        setupDigitizationFlags(runArgs, flags)
        logFastChain.info('Running with pile-up: %s', flags.Digitization.PileUp)

    # Disable LVL1 trigger if triggerConfig explicitly set to 'NONE'
    if hasattr(runArgs, 'triggerConfig') and runArgs.triggerConfig == 'NONE':
        flags.Detector.EnableL1Calo = False

    # Setup perfmon flags from runargs
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Special Configuration preInclude
    specialConfigPreInclude(flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    if not flags.Overlay.FastChain:
        # Load pile-up stuff after pre-include/exec to ensure everything is up-to-date
        from Digitization.DigitizationConfigFlags import pileupRunArgsToFlags
        pileupRunArgsToFlags(runArgs, flags)

        # Setup pile-up profile
        if flags.Digitization.PileUp:
            from RunDependentSimComps.PileUpUtils import setupPileUpProfile
            setupPileUpProfile(flags)

    flags.Sim.DoFullChain = True
    # For jobs running (MC) Overlay we take the run number from the
    # presampled RDOs, so we don't actually need to override the run
    # number.
    flags.Input.OverrideRunNumber = not flags.Overlay.FastChain
    
    # To respect --athenaopts 
    flags.fillFromArgs()

    # Moving here so that it is ahead of flags being locked. Need to
    # iterate on exact best position w.r.t. above calls
    # Handle metadata correctly
    if flags.Overlay.FastChain:
        from OverlayConfiguration.OverlayMetadata import fastChainOverlayMetadataCheck
        fastChainOverlayMetadataCheck(flags)

    # Lock flags
    flags.lock()

    if flags.Digitization.PileUp:
        from Digitization.PileUpConfig import PileUpEventLoopMgrCfg
        cfg = MainServicesCfg(flags, LoopMgr="PileUpEventLoopMgr")
        cfg.merge(PileUpEventLoopMgrCfg(flags))
    else:
        cfg = MainServicesCfg(flags)

    cfg.merge(PoolReadCfg(flags))

    # Simulation
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(flags))

    if not flags.Digitization.PileUp and ( (not flags.Overlay.FastChain and "xAOD::EventInfo#EventInfo" in flags.Input.TypedCollections) \
                                           or (flags.Overlay.FastChain and "xAOD::EventInfo#EventInfo" in flags.Input.SecondaryTypedCollections) ):
        # Make sure signal EventInfo is rebuilt from event context
        # TODO: this is probably not needed, but keeping it to be in sync with standard simulation
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
        cfg.merge(EventInfoUpdateFromContextAlgCfg(flags))

    if flags.Overlay.FastChain:
        # CopyMcEventCollection should be before Kernel
        from OverlayCopyAlgs.OverlayCopyAlgsConfig import CopyMcEventCollectionCfg
        cfg.merge(CopyMcEventCollectionCfg(flags))

    from ISF_Config.ISF_MainConfig import ISF_KernelCfg
    cfg.merge(ISF_KernelCfg(flags))

    # Main Overlay Steering
    if flags.Overlay.FastChain:
        from OverlayConfiguration.OverlaySteering import OverlayMainContentCfg
        cfg.merge(OverlayMainContentCfg(flags))
    else:
        from Digitization.DigitizationSteering import DigitizationMainContentCfg
        cfg.merge(DigitizationMainContentCfg(flags))

    # Special message service configuration
    from Digitization.DigitizationSteering import DigitizationMessageSvcCfg
    cfg.merge(DigitizationMessageSvcCfg(flags))

    # Special Configuration postInclude
    specialConfigPostInclude(flags, cfg)

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
