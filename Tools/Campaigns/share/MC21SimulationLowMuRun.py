# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from G4AtlasApps.SimFlags import simFlags
simFlags.RunNumber = 420000

from AthenaCommon.Resilience import protectedInclude
protectedInclude("Campaigns/MC21SimulationNoIoV.py")
