#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TrigExamples import MTCalibPebConfig
from AthenaCommon.Logging import logging
log = logging.getLogger('MTCalibPeb.py')
import inspect

def calibPebCfg(flags):

    # Ugly hack to get access to the athenaHLT globals:
    opts = dict(inspect.getmembers(inspect.stack()[1][0]))["f_globals"]

    # Options which can be set by running with -c 'optname=value'
    # Flag controlling if MTCalibPeb chains are concurrent or sequential,
    concurrent = opts.get('concurrent', False)
    # Number of chains
    num_chains = opts.get('num_chains', 3)
    # Configure hypo tool options
    MTCalibPebConfig.default_options.UseRandomSeed = opts.get('UseRandomSeed', False)
    MTCalibPebConfig.default_options.RandomAcceptRate = opts.get('RandomAcceptRate', 0.01)
    MTCalibPebConfig.default_options.BurnTimePerCycleMillisec = opts.get('BurnTimePerCycleMillisec', 20)
    MTCalibPebConfig.default_options.NumBurnCycles = opts.get('NumBurnCycles', 10)
    MTCalibPebConfig.default_options.BurnTimeRandomly = opts.get('BurnTimeRandomly', True)
    MTCalibPebConfig.default_options.Crunch = opts.get('Crunch', False)
    MTCalibPebConfig.default_options.CheckDataConsistency = opts.get('CheckDataConsistency', False)
    MTCalibPebConfig.default_options.ROBAccessDict = opts.get('ROBAccessDict', MTCalibPebConfig.rob_access_dict)
    MTCalibPebConfig.default_options.TimeBetweenROBReqMillisec = opts.get('TimeBetweenROBReqMillisec', 0)
    MTCalibPebConfig.default_options.PEBROBList = opts.get('PEBROBList', [])
    MTCalibPebConfig.default_options.PEBSubDetList = opts.get('PEBSubDetList', [])
    MTCalibPebConfig.default_options.CreateRandomData = opts.get('CreateRandomData', {})
    MTCalibPebConfig.default_options.EnableL1CaloPhase1 = opts.get('EnableL1CaloPhase1', False)
    MTCalibPebConfig.default_options.EnableL1MuonPhase1 = opts.get('EnableL1MuonPhase1', True)
    MTCalibPebConfig.default_options.EnableL1CaloLegacy = opts.get('EnableL1CaloLegacy', True)

    flags.Scheduler.AutoLoadUnmetDependencies = True

    MTCalibPebConfig.set_flags(flags)
    flags.lock()

    # Configure the L1 and HLT sequences
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    cfg = ComponentAccumulator()
    cfg.merge( MTCalibPebConfig.l1_seq_cfg(flags) )
    cfg.merge( MTCalibPebConfig.hlt_seq_cfg(flags, num_chains=num_chains, concurrent=concurrent) )

    return cfg
