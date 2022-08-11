#!/usr/bin/env python
#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AutoConfigFlags import GetFileMD

from AthenaCommon.Logging import logging
log = logging.getLogger('RunEBWeightsComputation.py')


def ebComputingAlg(flags, itemsMap = {}):
    acc = ComponentAccumulator()

    ebAlg = CompFactory.EnhancedBiasWeightCompAlg()
    ebAlg.ChainToItemMap = itemsMap
    ebAlg.OutputLevel = ConfigFlags.Exec.OutputLevel
    acc.addEventAlgo(ebAlg)

    return acc


# Read the keys from the COOL database
def getConfigKeys(inputFile):
    run = GetFileMD(inputFile)['runNumbers'][0]
    lb = GetFileMD(inputFile)['lumiBlockNumbers'][0]

    from TrigConfigSvc.TrigConfigSvcCfg import getTrigConfFromCool
    return getTrigConfFromCool(run, lb)


# Read the seeds of low and medium chain, not available in the menu (they are hlt seeded)
def readHLTSeeds(smk=-1, db=""):
    from TrigConfIO.HLTTriggerConfigAccess import HLTJobOptionsAccess
    joData = HLTJobOptionsAccess(dbalias = db, smkey = smk)

    import ast

    chainToItem = {}
    chainToItem["HLT_eb_low_L1RD2_FILLED"] = ast.literal_eval(joData.properties("EnhancedBiasHypo.HLT_eb_low_L1RD2_FILLED")["L1ItemNames"])
    chainToItem["HLT_eb_medium_L1RD2_FILLED"] = ast.literal_eval(joData.properties("EnhancedBiasHypo.HLT_eb_medium_L1RD2_FILLED")["L1ItemNames"])

    return chainToItem
    

if __name__=='__main__':
    import sys
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('--maxEvents', type=int, help='Maximum number of events to process')
    parser.add_argument('--skipEvents',type=int, help='Number of events to skip')
    parser.add_argument('--loglevel', type=int, default=3, help='Verbosity level: 1 - VERBOSE, 2 - DEBUG, 3 - INFO')
    parser.add_argument('flags', nargs='*', help='Config flag overrides')  
    args = parser.parse_args()

    log.setLevel(args.loglevel)

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    ConfigFlags.fillFromArgs(args.flags)
    ConfigFlags.Trigger.triggerConfig = 'DB'
    ConfigFlags.Exec.OutputLevel = args.loglevel
    ConfigFlags.Trigger.doNavigationSlimming = False
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    cfg = MainServicesCfg(ConfigFlags)

    isRunningFromAOD = True if len(ConfigFlags.Input.Collections) else False
    if isRunningFromAOD:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(ConfigFlags))
    else:
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
        cfg.merge(TriggerRecoCfg(ConfigFlags))

    configKeys = getConfigKeys(ConfigFlags.Input.Files)
    itemsMap = readHLTSeeds(smk = configKeys["SMK"], db = configKeys["DB"])
    cfg.merge(ebComputingAlg(ConfigFlags, itemsMap))

    eventLoop = CompFactory.AthenaEventLoopMgr()
    eventLoop.EventPrintoutInterval = 1000
    cfg.addService(eventLoop)

    # If you want to turn on more detailed messages ...
    # exampleMonitorAcc.getEventAlgo('ExampleMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    sc = cfg.run(args.maxEvents, args.loglevel)
    sys.exit(0 if sc.isSuccess() else 1)
