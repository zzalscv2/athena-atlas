#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA

# Load Sim flags if available
flagsAvailableSim = True
try:
    import SimulationConfig # noqa: F401
except ImportError:
    flagsAvailableSim = False
if flagsAvailableSim:
    flags._loadDynaFlags("Sim")

# Load GeoModel flags
flags._loadDynaFlags("GeoModel")

# Init and print
flags.initAll()
flags.lock()
flags.dump()
