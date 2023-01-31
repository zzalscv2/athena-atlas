#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_RUN2
flags._loadDynaFlags("Detector")

# Don't fail just because G4AtlasApps isn't present in this build.
havesim = True
try:
    import G4AtlasApps # noqa: F401
except ImportError:
    havesim = False
if havesim:
    flags._loadDynaFlags("Sim")

# Don't fail just because Digitization isn't present in this build.
haveDigi = True
try:
    import Digitization # noqa: F401
except ImportError:
    haveDigi = False
if haveDigi:
    flags._loadDynaFlags("Digitization")

# Don't fail just because OverlayConfiguration isn't present in this build.
haveOverlay = True
try:
    import OverlayConfiguration # noqa: F401
except ImportError:
    haveOverlay = False
if haveOverlay:
    flags._loadDynaFlags("Overlay")

flags.initAll()
flags.lock()
flags.dump()
