#!/usr/bin/env python
"""Run tests on OverlayCopyAlgsConfig.py

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from OverlayConfiguration.OverlayTestHelpers import \
    CommonTestArgumentParser, overlayTestFlags, postprocessAndLockFlags, printAndRun
from OverlayCopyAlgs.OverlayCopyAlgsConfig import \
    CopyCaloCalibrationHitContainersCfg, CopyJetTruthInfoCfg, CopyMcEventCollectionCfg, \
    CopyTrackRecordCollectionsCfg
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoOverlayCfg

# Argument parsing
parser = CommonTestArgumentParser("OverlayCopyAlgs_test.py")
args = parser.parse_args()

# Configure
flags = initConfigFlags()
overlayTestFlags(flags, args)
postprocessAndLockFlags(flags, args)

# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))

# Add event info overlay (needed downstream)
acc.merge(EventInfoOverlayCfg(flags))

# Add truth overlay
acc.merge(CopyMcEventCollectionCfg(flags))
acc.merge(CopyJetTruthInfoCfg(flags))
acc.merge(CopyCaloCalibrationHitContainersCfg(flags))
acc.merge(CopyTrackRecordCollectionsCfg(flags))

# Print and run
sys.exit(printAndRun(acc, flags, args))
