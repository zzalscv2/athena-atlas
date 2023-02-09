# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from G4AtlasApps.SimFlags import simFlags
simFlags.RunNumber = 410000

from AthenaCommon.Resilience import protectedInclude
protectedInclude("RunDependentSimData/configLumi_simProfile_run410000_mc23a_MultiBeamspot.py")

protectedInclude("Campaigns/MC23SimulationNoIoV.py")
