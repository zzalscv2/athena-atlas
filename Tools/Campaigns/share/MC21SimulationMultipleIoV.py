# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from G4AtlasApps.SimFlags import simFlags
simFlags.RunNumber = 410000

from AthenaCommon.Resilience import protectedInclude
protectedInclude("RunDependentSimData/configLumi_simProfile_run410000_mc21a_MultiBeamspot.py")

protectedInclude("Campaigns/MC21SimulationNoIoV.py")
