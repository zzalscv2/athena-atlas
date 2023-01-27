#!/usr/bin/env python
"""Run tests on SCT_DigitizationConfig.py

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
import sys
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from PixelDigitization.PixelDigitizationConfig import PixelDigitizationHSCfg

# Set up logging and new style config
log.setLevel(DEBUG)
# Configure
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.HITS_RUN2
flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
flags.GeoModel.Align.Dynamic = False
flags.GeoModel.Type = "BrlIncl4.0_ref"
flags.Beam.NumberOfCollisions = 0.
flags.lock()
# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))
acc.merge(PixelDigitizationHSCfg(flags))
# Dump config
acc.getService("StoreGateSvc").Dump = True
acc.getService("ConditionStore").Dump = True
acc.printConfig(withDetails=True)
flags.dump()
# Execute and finish
sc = acc.run(maxEvents=3)
# Success should be 0
sys.exit(not sc.isSuccess())
