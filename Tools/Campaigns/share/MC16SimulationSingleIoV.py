# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from G4AtlasApps.SimFlags import simFlags
simFlags.RunNumber = 284500

from AthenaCommon.Resilience import protectedInclude
protectedInclude("Campaigns/MC16SimulationNoIoV.py")
