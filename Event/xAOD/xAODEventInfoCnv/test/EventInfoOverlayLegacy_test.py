#!/usr/bin/env python
"""Run tests for EventInfo overlay with legacy inputs

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
parser = CommonTestArgumentParser("EventInfoOverlayLegacy_test.py")
args = parser.parse_args()

# Configure
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_BKG_RUN2
# use old HITS on purpose
flags.Input.SecondaryFiles = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1"]
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

# Add event info overlay
acc.merge(EventInfoOverlayCfg(flags))

# Print and run
sys.exit(printAndRun(acc, flags, args))
