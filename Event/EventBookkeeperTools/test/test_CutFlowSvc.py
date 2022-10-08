#!/usr/bin/env python
"""Test CutFlowSvc

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys
from argparse import ArgumentParser

from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg, CutFlowOutputList

# Argument parsing
parser = ArgumentParser(prog='test_CutFlowSvc')
parser.add_argument('input', type=str, nargs='?',
                    help='Specify the input file')
parser.add_argument("-d", "--data", default=False,
                    action="store_true", help="Run with data")
parser.add_argument('-t', '--threads', default=0, type=int,
                    help='The number of concurrent threads to run. 0 uses serial Athena.')
parser.add_argument('-p', '--processes', default=0, type=int,
                    help='The number of concurrent processes to run. 0 disables Athena MP.')
parser.add_argument("--sharedWriter", default=False,
                    action="store_true", help="Run with shared writer")
args = parser.parse_args()

if args.input:
  ConfigFlags.Input.Files = [args.input]
elif args.data:
  ConfigFlags.Input.Files = defaultTestFiles.RAW
else:
  ConfigFlags.Input.Files = defaultTestFiles.AOD_MC

# Flags relating to multithreaded execution
threads = args.threads
procs = args.processes
maxEvents = 200
ConfigFlags.Concurrency.NumThreads = threads
ConfigFlags.Concurrency.NumProcs = procs
if threads > 0:
  maxEvents = max(maxEvents, 10 * threads)
  ConfigFlags.Scheduler.ShowDataDeps = True
  ConfigFlags.Scheduler.ShowDataFlow = True
  ConfigFlags.Scheduler.ShowControlFlow = True
  ConfigFlags.Concurrency.NumConcurrentEvents = threads
suffix = f"_test_{threads}_{procs}"
if procs > 0:
  maxEvents = max(maxEvents, 10 * procs)
  if args.data:
    suffix += "_data"
  if args.sharedWriter:
    maxEvents *= 5
    ConfigFlags.MP.UseSharedWriter = True
    suffix += "_sharedWriter"
  ConfigFlags.MP.WorkerTopDir = f"athenaMP_workers{suffix}"
  ConfigFlags.MP.EventOrdersFile = f"athenamp_eventorders{suffix}.txt"

ConfigFlags.Output.AODFileName = f"testAOD{suffix}.pool.root"

ConfigFlags.lock()

# Setup service
acc = MainServicesCfg(ConfigFlags)
from AthenaConfiguration.Enums import Format
if ConfigFlags.Input.Format is Format.BS:
  from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
  acc.merge(ByteStreamReadCfg(ConfigFlags))
else:
  acc.merge(PoolReadCfg(ConfigFlags))

if 'EventInfo' not in ConfigFlags.Input.Collections:
  from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
  acc.merge(EventInfoCnvAlgCfg(ConfigFlags, disableBeamSpot=True))

acc.merge(CutFlowSvcCfg(ConfigFlags))
acc.addEventAlgo(CompFactory.TestFilterReentrantAlg("TestReentrant1", FilterKey="TestReentrant1", Modulo=2))
acc.addEventAlgo(CompFactory.TestFilterReentrantAlg("TestReentrant2", FilterKey="TestReentrant2", Modulo=4))

acc.merge(OutputStreamCfg(ConfigFlags, "AOD", MetadataItemList=CutFlowOutputList(ConfigFlags)))

# Debugging
from AthenaCommon.Constants import VERBOSE
acc.getService('CutFlowSvc').OutputLevel = VERBOSE
acc.getPublicTool('BookkeeperTool').OutputLevel = VERBOSE
acc.getEventAlgo('AllExecutedEventsCounterAlg').OutputLevel = VERBOSE

# Execute and finish
sc = acc.run(maxEvents=maxEvents)

# Success should be 0
sys.exit(not sc.isSuccess())
