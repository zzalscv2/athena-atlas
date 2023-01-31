#!/usr/bin/env python
"""Run tests for old EventInfo to xAOD::EventInfo conversion

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaCommon.Debugging import DbgStage
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg

# Set up logging
log.setLevel(DEBUG)

# Argument parsing
from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("-n", "--maxEvents",  default=3, type=int, help="The number of events to run. 0 skips execution")
parser.add_argument("-t", "--threads", default=1, type=int, help="The number of concurrent threads to run. 0 uses serial Athena.")
parser.add_argument("-b", "--noBeamSpot", default=False, action="store_true", help="Don't try to use beamspot information in the conversion test")
parser.add_argument("-V", "--verboseAccumulators", default=False, action="store_true", help="Print full details of the AlgSequence for each accumulator")
parser.add_argument("-d", "--debug", default='', type=str,
                    choices=DbgStage.allowed_values,
                    help="Debugging flag: " + ','.join (DbgStage.allowed_values))
                    
args = parser.parse_args()

# Configure
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.HITS_RUN2
flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
flags.Output.HITSFileName = "myHITS.pool.root"

# Flags relating to multithreaded execution
flags.Concurrency.NumThreads = args.threads
if args.threads > 0:
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Concurrency.NumConcurrentEvents = args.threads

flags.lock()

# Function tests
accAlg = EventInfoCnvAlgCfg(flags, disableBeamSpot=args.noBeamSpot)
# reset to prevent errors on deletion
accAlg.__init__()

# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))

# Add event info overlay
acc.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=args.noBeamSpot))

# Add output
acc.merge(OutputStreamCfg(flags, "HITS"))

# Dump config
if args.verboseAccumulators:
    acc.printConfig(withDetails=True)
acc.getService("StoreGateSvc").Dump = True
flags.dump()

if args.debug:
    acc.setDebugStage (args.debug)

# Dump config summary
acc.printConfig(withDetails=False)

# Execute and finish
sc = acc.run(maxEvents=args.maxEvents)

# Success should be 0
sys.exit(not sc.isSuccess())
