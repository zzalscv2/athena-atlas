#!/usr/bin/env python
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from glob import glob
from AthenaConfiguration.ComponentFactory import CompFactory

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for runPFPhysVal configuration')
    parser.add_argument("--filesInput")
    parser.add_argument("--outputFile", help='Name of output file',default="M_output.root")
    return parser.parse_args()

# Parse the arguments 
MyArgs = GetCustomAthArgs()

from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.Input.Files = []
for path in MyArgs.filesInput.split(','):
    ConfigFlags.Input.Files += glob(path)

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(ConfigFlags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(ConfigFlags))

ConfigFlags.lock()

from PFODQA.PFPhysValConfig import PFODQACf
acc.merge(PFODQACf(ConfigFlags))

# finally, set up the infrastructure for writing our output
histSvc = CompFactory.THistSvc()
histSvc.Output += ["PhysVal DATAFILE='"+MyArgs.outputFile+"' OPT='RECREATE'"]
acc.addService(histSvc)

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run(maxEvents=-1)
 
# Success should be 0
import sys
sys.exit(not sc.isSuccess())
