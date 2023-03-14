# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from G4AtlasApps.SimFlags import simFlags
simFlags.PhysicsList = "FTFP_BERT_ATL"
simFlags.TruthStrategy = "MC15aPlus"

simFlags.TRTRangeCut = 30.0
simFlags.TightMuonStepping = True

from ISF_Config.ISF_jobProperties import ISF_Flags
from AthenaCommon.Resilience import protectedInclude

if ISF_Flags.Simulator.isQuasiStable():
    protectedInclude("SimulationJobOptions/preInclude.ExtraParticles.py")
    protectedInclude("SimulationJobOptions/preInclude.G4ExtraProcesses.py")

protectedInclude("SimulationJobOptions/preInclude.BeamPipeKill.py")

if ISF_Flags.Simulator.usesFastCaloSim():
    # FastCaloSim requires the Sampling Fractions to be present
    from IOVDbSvc.CondDB import conddb
    conddb.addOverride("/TILE/OFL02/CALIB/SFR","TileOfl02CalibSfr-SIM-07")

if ISF_Flags.Simulator.isFullSim():
    protectedInclude("SimulationJobOptions/preInclude.FrozenShowersFCalOnly.py")

# enable G4 optimisations
protectedInclude("SimulationJobOptions/preInclude.G4Optimizations.py")
