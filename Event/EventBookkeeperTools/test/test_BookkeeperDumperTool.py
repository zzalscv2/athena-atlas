#!/usr/bin/env python
"""Dump CutBookkeepers

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
import sys
from argparse import ArgumentParser

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

from EventBookkeeperTools.EventBookkeeperToolsConfig import BookkeeperDumperToolCfg

# Argument parsing
parser = ArgumentParser(prog='test_BookkeeperDumpertool')
parser.add_argument('input', type=str, nargs='?',
                    help='Specify the input file')
args = parser.parse_args()

flags = initConfigFlags()
if args.input:
    flags.Input.Files = [args.input]
else:
    flags.Input.Files = defaultTestFiles.AOD_MC
flags.lock()

# Setup tools
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))
acc.merge(BookkeeperDumperToolCfg(flags))

# Execute and finish
sc = acc.run(maxEvents=1)

# Success should be 0
sys.exit(not sc.isSuccess())
