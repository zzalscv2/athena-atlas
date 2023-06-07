#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.

# Cut-and-paste from RunHitAnalysis.py

import sys

from argparse import ArgumentParser

# Argument parsing
parser = ArgumentParser("egammaMonitoring")
parser.add_argument("-m", "--maxEvents", default=60000, type=int,
                    help="The number of events to run. -1 runs all events.")
parser.add_argument("-p", "--particleType", default='electron', type=str,
                    help="The particle type. electron or gamma")
parser.add_argument("-fwd", "--addFwd", default='False', type=str,
                    help="Also run on fwd electrons")
args = parser.parse_args()

addFwd = True if args.addFwd == 'True' else False
if args.particleType == 'gamma' and addFwd:
    print('Fwd electrons can only be read in an electron run')
    addFwd = False

# Configure
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import ProductionStep
flags = initConfigFlags()
flags.Common.ProductionStep = ProductionStep.Simulation
flags.Input.Files = ['Nightly_AOD.pool.root']
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
setupDetectorFlags(flags, ['LAr','Tile'], use_metadata=True, toggle_geometry=True)
# to be checked. Without this, I got
#DataProxy         WARNING accessData: conversion failed for data object 254546453//LAR/Align
# Returning NULL DataObject pointer  
#GeoModelSvc.LAr...  ERROR  Could not retrieve LAr DetCondKeyTrans 
flags.LAr.doAlign = False
flags.lock()

# Construct our accumulator to run
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from LArGeoAlgsNV.LArGMConfig import LArGMCfg
acc.merge(LArGMCfg(flags))
from TileGeoModel.TileGMConfig import TileGMCfg
acc.merge(TileGMCfg(flags)) 
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))
from egammaValidation.egammaMonitoringConfig import egammaMonitoringCfg
acc.merge(egammaMonitoringCfg(flags, particleType = args.particleType, addFwd = addFwd))

# Execute and finish
sc = acc.run(maxEvents=args.maxEvents)

# Success should be 0
sys.exit(not sc.isSuccess())

