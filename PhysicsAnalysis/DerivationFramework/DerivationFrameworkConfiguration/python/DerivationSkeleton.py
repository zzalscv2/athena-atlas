# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DerivationFrameworkConfiguration import DerivationConfigList
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logDerivation = logging.getLogger('Derivation')
    logDerivation.info('****************** STARTING DERIVATION *****************')

    logDerivation.info('**** Transformation run arguments')
    logDerivation.info(str(runArgs))

    logDerivation.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    commonRunArgsToFlags(runArgs, ConfigFlags)

    ConfigFlags.Common.ProductionStep = ProductionStep.Derivation

    # Switch on PerfMon
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(ConfigFlags, runArgs)
    
    # Input types
    allowedInputTypes = [ 'AOD', 'DAOD_PHYS', 'EVNT' ]
    availableInputTypes = [ hasattr(runArgs, f'input{inputType}File') for inputType in allowedInputTypes ]
    if sum(availableInputTypes) != 1:
        raise ValueError('Input must be exactly one of the following types: inputAODFile, inputEVNTFile, inputDAOD_PHYSFile')
    idx = availableInputTypes.index(True)
    ConfigFlags.Input.Files = getattr(runArgs, f'input{allowedInputTypes[idx]}File')

    # Output formats
    formats = []
    if hasattr(runArgs, 'formats'):
        formats = runArgs.formats
        logDerivation.info('Will attempt to make the following derived formats: {0}'.format(formats))
    else:
        logDerivation.error('Derivation job started, but with no output formats specified - aborting')
        raise ValueError('No derived formats specified')

    # Output files
    for runArg in dir(runArgs):
        if 'output' in runArg and 'File' in runArg and 'NTUP_PHYSVAL' not in runArg:
            outputFileName = getattr(runArgs, runArg)
            flagString = f'Output.{runArg.strip("output")}Name'
            ConfigFlags.addFlag(flagString, outputFileName)
            ConfigFlags.Output.doWriteDAOD = True

    # Pre-include
    processPreInclude(runArgs, ConfigFlags)

    # Pre-exec
    processPreExec(runArgs, ConfigFlags)

    # Lock flags
    ConfigFlags.lock()

    # The D(2)AOD building configuration
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(ConfigFlags)

    # Pool file reading and writing
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(ConfigFlags))
    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    cfg.merge(PoolWriteCfg(ConfigFlags))

    for formatName in formats:
        derivationConfig = getattr(DerivationConfigList, f'{formatName}Cfg')
        cfg.merge(derivationConfig(ConfigFlags))

    # Pass-through mode (ignore skimming and accept all events)
    if hasattr(runArgs, 'passThrough'):
        logDerivation.info('Pass-through mode was requested. All events will be written to the output.')
        for algo in cfg.getEventAlgos():
            if isinstance(algo, CompFactory.DerivationFramework.DerivationKernel):
                algo.SkimmingTools = []

    # PerfMonSD
    if ConfigFlags.PerfMon.doFullMonMT or ConfigFlags.PerfMon.doFastMonMT:
       from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
       cfg.merge(PerfMonMTSvcCfg(ConfigFlags))

    # Set EventPrintoutInterval to 100 events
    cfg.getService(cfg.getAppProps()['EventLoop']).EventPrintoutInterval = 100

    # Post-include
    processPostInclude(runArgs, ConfigFlags, cfg)

    # Post-exec
    processPostExec(runArgs, ConfigFlags, cfg)

    # Run the final configuration
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
