#!/usr/bin/env python
"""Run tests on PixelOverlayConfig.py

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from InDetOverlay.PixelOverlayConfig import PixelOverlayCfg
from OverlayConfiguration.OverlayTestHelpers import \
    CommonTestArgumentParser, overlayTestFlags, postprocessAndLockFlags, printAndRun
from OverlayCopyAlgs.OverlayCopyAlgsConfig import CopyMcEventCollectionCfg
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoOverlayCfg

# Argument parsing
parser = CommonTestArgumentParser("PixelOverlayConfig_test.py")
args = parser.parse_args()

# Configure
flags = initConfigFlags()
overlayTestFlags(flags, args)
flags.Output.RDOFileName = 'mcOverlayRDO_Pixel.pool.root'
postprocessAndLockFlags(flags, args)

# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))

# Add event and truth overlay (needed downstream)
acc.merge(EventInfoOverlayCfg(flags))
acc.merge(CopyMcEventCollectionCfg(flags))

# Add Pixel overlay
acc.merge(PixelOverlayCfg(flags))

# Dump the pickle
with open("PixelOverlayCfg.pkl", "wb") as f:
    acc.store(f)

# Print and run
sys.exit(printAndRun(acc, flags, args))
