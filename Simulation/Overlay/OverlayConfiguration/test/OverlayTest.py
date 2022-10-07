#!/usr/bin/env python
"""Run tests for MC+MC or MC+data overlay

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaConfiguration.AllConfigFlags import ConfigFlags

from Digitization.DigitizationSteering import DigitizationMessageSvcCfg
from OverlayConfiguration.OverlaySteering import OverlayMainCfg
from OverlayConfiguration.OverlayTestHelpers import \
    CommonTestArgumentParser, OverlayJobOptsDumperCfg, \
    defaultTestFlags, postprocessAndLockFlags, printAndRun

# Argument parsing
parser = CommonTestArgumentParser("OverlayTest.py")
parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                    help="Specify the list of detectors")
parser.add_argument("--profile", default=False, action="store_true",
                    help="Profile using VTune")
parser.add_argument("--dependencies", default=False, action="store_true",
                    help="Dependency check")
parser.add_argument("--dump", default=False, action="store_true",
                    help="Dump job options")
args = parser.parse_args()

# Some info about the job
print()
print("Overlay: {}".format("MC+data" if args.data else "MC+MC"))
print(f"Run: {args.run}")
print(f"Number of threads: {args.threads}")
if not args.detectors:
    print("Running complete detector")
else:
    print("Running with: {}".format(", ".join(args.detectors)))
print()
if args.profile:
    print("Profiling...")
    print()

ConfigFlags.Scheduler.AutoLoadUnmetDependencies = False
if args.dependencies:
    ConfigFlags.Input.FailOnUnknownCollections = True
    print("Checking dependencies...")
    print()

# Configure
defaultTestFlags(ConfigFlags, args)
postprocessAndLockFlags(ConfigFlags, args)

# Construct our accumulator to run
acc = OverlayMainCfg(ConfigFlags)
if args.profile:
    from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    acc.merge(VTuneProfilerServiceCfg(ConfigFlags))
if args.dump:
    acc.merge(OverlayJobOptsDumperCfg(ConfigFlags))
acc.merge(DigitizationMessageSvcCfg(ConfigFlags))

# Count algorithm misses
if ConfigFlags.Concurrency.NumThreads > 0:
    acc.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True

# Dependency check
if args.dependencies:
    acc.getEventAlgo("OutputStreamRDO").ExtraInputs += [tuple(l.split('#')) for l in acc.getEventAlgo("OutputStreamRDO").ItemList if '*' not in l and 'Aux' not in l]
    acc.getService("AthenaHiveEventLoopMgr").DependencyCheck = True

# dump pickle
with open("ConfigOverlay.pkl", "wb") as f:
    acc.store(f)

# Print and run
sys.exit(printAndRun(acc, ConfigFlags, args))
