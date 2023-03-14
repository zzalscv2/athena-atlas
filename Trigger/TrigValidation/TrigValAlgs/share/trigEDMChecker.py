#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Script to run the TrigEDM(Aux)Checker algorithms.
#
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
from TrigValAlgs.TrigValAlgsConfig import TrigEDMAuxCheckerCfg, TrigEDMCheckerCfg
import sys

# Setup flags
flags = initConfigFlags()
flags.fillFromArgs()
flags.lock()

# Central services
cfg = MainServicesCfg(flags)
cfg.merge(PoolReadCfg(flags))
cfg.merge(TriggerRecoCfg(flags))

# EDMCheckers
cfg.merge(TrigEDMCheckerCfg(flags, doDumpAll=False))
cfg.merge(TrigEDMAuxCheckerCfg(flags))

# Final tweaks and run
cfg.getService("MessageSvc").enableSuppression = False
sc = cfg.run()

sys.exit(sc.isFailure())
