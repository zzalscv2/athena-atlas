# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg


def fromRunArgs(runArgs):
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = True

    from AthenaCommon.Logging import logging
    logDerivation = logging.getLogger('PhysicsValidation')
    logDerivation.info('****************** STARTING NTUP_PHYSVAL *****************')

    logDerivation.info('**** Transformation run arguments')
    logDerivation.info(str(runArgs))

    logDerivation.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    commonRunArgsToFlags(runArgs, ConfigFlags)

    # Switch on PerfMon
    ConfigFlags.PerfMon.doFullMonMT = True

    # Input types
    if not hasattr(runArgs, 'inputDAOD_PHYSVALFile'):
        raise ValueError('Input must be provided using --inputDAOD_PHYSVALFile')
    ConfigFlags.Input.Files = runArgs.inputDAOD_PHYSVALFile

    # Physics Validation
    if hasattr(runArgs, 'outputNTUP_PHYSVALFile'):
        logDerivation.info('Will produce NTUP_PHYSVAL file')
        ConfigFlags.PhysVal.OutputFileName = runArgs.outputNTUP_PHYSVALFile

        if not (hasattr(runArgs, 'inputAODFile') or hasattr(runArgs, 'inputDAOD_PHYSVALFile')):
            logDerivation.error('NTUP_PHYSVAL requires AOD or DAOD_PHYSVAL input')
            raise ValueError('Incorrect inputs for NTUP_PHYSVAL')

        if not hasattr(runArgs, 'validationFlags'):
            raise ValueError('--validationFlags is mandatory')

        for flag in runArgs.validationFlags:
            if flag == 'doPFlow_FlowElements':  # backwards compatibility
                flag = 'doPFlow'

            name = f'PhysVal.{flag}'
            if not ConfigFlags.hasFlag(name):
                raise ValueError(f"Unknown validation flag '{name}'")

            logDerivation.info("Enabling validation flag '%s'", name)
            ConfigFlags._set(name, True)
    else:
        raise ValueError('Output file name needs to be set using --outputNTUP_PHYSVALFile')

    # Pre-include
    processPreInclude(runArgs, ConfigFlags)

    # Pre-exec
    processPreExec(runArgs, ConfigFlags)

    # Lock flags
    ConfigFlags.lock()

    # The main services configuration
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    # Run NTUP_PHYSVAL making
    from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
    cfg.merge(PhysValMonitoringCfg(ConfigFlags))

    # PerfMonSD
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
