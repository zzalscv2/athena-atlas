#!/usr/bin/env python
"""Run tests on HGTD_OverlayConfig.py

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from HGTD_Overlay.HGTD_OverlayConfig import HGTD_OverlayCfg
from OverlayConfiguration.OverlayTestHelpers import \
    CommonTestArgumentParser, overlayTestFlags, postprocessAndLockFlags, printAndRun
from OverlayCopyAlgs.OverlayCopyAlgsConfig import CopyMcEventCollectionCfg
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoOverlayCfg

# Argument parsing
parser = CommonTestArgumentParser("HGTD_OverlayConfig_test.py")
args = parser.parse_args()

# Configure
flags = initConfigFlags()
overlayTestFlags(flags, args)
postprocessAndLockFlags(flags, args)

# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))

# Add event and truth overlay (needed downstream)
acc.merge(EventInfoOverlayCfg(flags))
acc.merge(CopyMcEventCollectionCfg(flags))

# Add HGTD overlay
acc.merge(HGTD_OverlayCfg(flags))

# Dump the pickle
with open("HGTD_OverlayCfg.pkl", "wb") as f:
    acc.store(f)

# Print and run
sys.exit(printAndRun(acc, flags, args))
