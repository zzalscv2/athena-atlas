################################################
## Setup jobOptions CalHits.py
##
## Turns on calibration hits for LAr and Tile
################################################

from G4AtlasApps.SimFlags import simFlags
simFlags.CalibrationRun = 'LAr+Tile'

# The following optimizations are not compatible with producing valid
# CaloCalibrationHits
simFlags.ApplyPRR.set_Value_and_Lock(False)
simFlags.ApplyNRR.set_Value_and_Lock(False)
# No Frozen Showers
simFlags.LArParameterization.set_Value_and_Lock(0)
include.block("SimulationJobOptions/preInclude.FrozenShowersFCalOnly.py")

#Sanity check
from ISF_Config.ISF_jobProperties import ISF_Flags
if (simFlags.ISFRun() and ("ATLFAST" in ISF_Flags.Simulator() or "G4FastCalo" in ISF_Flags.Simulator())) or simFlags.LArParameterization() != 0 or simFlags.ApplyPRR() or simFlags.ApplyNRR():
    raise ValueError("CalibrationHit creation only supported in FullSim jobs")
