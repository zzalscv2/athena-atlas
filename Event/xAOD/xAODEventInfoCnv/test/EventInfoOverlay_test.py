#!/usr/bin/env python
"""Run tests for EventInfo overlay

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from OverlayConfiguration.OverlayTestHelpers import \
    CommonTestArgumentParser, postprocessAndLockFlags, printAndRun
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoOverlayCfg

# Argument parsing
parser = CommonTestArgumentParser("EventInfoOverlay_test.py")
args = parser.parse_args()

# Configure
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_BKG_RUN2
flags.Input.SecondaryFiles = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/Special/TestCase_xAODEventInfo.root"]
flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
flags.Overlay.DataOverlay = False
flags.Output.RDOFileName = "myRDO.pool.root"
flags.Output.RDO_SGNLFileName = "myRDO_SGNL.pool.root"

postprocessAndLockFlags(flags, args)

# Function tests
accAlg = EventInfoOverlayCfg(flags)
# reset to prevent errors on deletion
accAlg.__init__()

# Construct our accumulator to run
acc = MainServicesCfg(flags)
acc.merge(PoolReadCfg(flags))

# Print and run
sys.exit(printAndRun(acc, flags, args))
