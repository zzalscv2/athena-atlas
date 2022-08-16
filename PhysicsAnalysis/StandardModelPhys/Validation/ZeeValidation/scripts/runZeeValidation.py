#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for ZeeValidation configuration')
    parser.add_argument("--filesInput", required=True)
    parser.add_argument("--outputFile", help='Name of output file',default="M_output.root")
    return parser.parse_args()

# Parse the arguments
MyArgs = GetCustomAthArgs()

from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.Input.Files = []
for path in MyArgs.filesInput.split(','):
    ConfigFlags.Input.Files += glob(path)
ConfigFlags.PhysVal.OutputFileName = MyArgs.outputFile

ConfigFlags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(ConfigFlags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(ConfigFlags))

from ZeeValidation.ZeeValidationMonToolConfig import PhysValZeeCfg
from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
acc.merge(PhysValMonitoringCfg(ConfigFlags, tools=[acc.popToolsAndMerge(PhysValZeeCfg(ConfigFlags))]))

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run(maxEvents=-1)

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
