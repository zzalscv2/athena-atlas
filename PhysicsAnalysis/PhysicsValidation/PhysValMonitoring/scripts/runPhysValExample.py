#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for PhysValExample configuration')
    parser.add_argument("--filesInput")
    parser.add_argument("--outputFile", help='Name of output file', default="M_output.root")
    return parser.parse_args()

# Parse the arguments
MyArgs = GetCustomAthArgs()

from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.Input.Files = []
for path in MyArgs.filesInput.split(','):
    ConfigFlags.Input.Files += glob(path)
ConfigFlags.PhysVal.OutputFileName = MyArgs.outputFile

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(ConfigFlags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(ConfigFlags))

ConfigFlags.lock()

from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg, PhysValExampleCfg
acc.merge(PhysValMonitoringCfg(ConfigFlags, tools=[acc.popToolsAndMerge(PhysValExampleCfg(ConfigFlags))]))

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run(maxEvents=-1)

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
