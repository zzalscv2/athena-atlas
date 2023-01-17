#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.HITS_RUN2
flags._loadDynaFlags("Detector")

# Don't fail just because G4AtlasApps isn't present in this build.
havesim = True
try:
    import G4AtlasApps # noqa: F401
except ImportError:
    havesim = False
if havesim:
    flags._loadDynaFlags("Sim")

flags.initAll()
flags.lock()
flags.dump()
