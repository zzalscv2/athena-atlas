#!/usr/bin/env python
"""Run tests on RPC_DigitizationConfigNew.py

Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
"""
import sys
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaCommon.Configurable import Configurable
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaConfiguration.MainServicesConfig import MainServicesSerialCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from Digitization.DigitizationConfigFlags import createDigitizationCfgFlags
from OverlayCommonAlgs.OverlayConfigFlags import createOverlayCfgFlags
# RPC imports
from RPC_Digitization.RPC_DigitizationConfigNew import (
    RPC_RangeToolCfg, RPC_DigitizationToolCfg, RPC_DigitizerCfg,
    RPC_OverlayDigitizationToolCfg, RPC_OverlayDigitizerCfg,
)

# Set up logging and new style config
log.setLevel(DEBUG)
Configurable.configurableRun3Behavior = True
# Configure
ConfigFlags.Input.Files = defaultTestFiles.HITS
ConfigFlags.Output.RDOFileName = "myRDO.pool.root"
ConfigFlags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
ConfigFlags.join(createDigitizationCfgFlags())
ConfigFlags.join(createOverlayCfgFlags())
ConfigFlags.lock()
# Construct our accumulator to run
acc = MainServicesSerialCfg()
acc.merge(PoolReadCfg(ConfigFlags))
acc.merge(RPC_DigitizerCfg(ConfigFlags))
# Add configuration to write HITS pool file
ItemList = [
    "MuonSimDataCollection#*",
    "RpcPadContainer#*",
]
acc.merge(OutputStreamCfg(ConfigFlags, "RDO", ItemList=ItemList))
# Dump config
acc.getService("StoreGateSvc").Dump = True
acc.getService("ConditionStore").Dump = True
acc.printConfig(withDetails=True)
ConfigFlags.dump()
# Execute and finish
sc = acc.run(maxEvents=3)
# Success should be 0
sys.exit(not sc.isSuccess())

