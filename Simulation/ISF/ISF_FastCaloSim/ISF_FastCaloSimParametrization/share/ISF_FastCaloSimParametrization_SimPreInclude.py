from G4AtlasApps.SimFlags import simFlags
simFlags.RecordStepInfo=True

#No vertex smearing
simFlags;simFlags.VertexFromCondDB.set_Value_and_Lock(False)

# Deactivated G4Optimizations:
#MuonFieldOnlyInCalo
simFlags.MuonFieldOnlyInCalo.set_Value_and_Lock(False)
#NRR
simFlags.ApplyNRR.set_Value_and_Lock(False)
#PRR
simFlags.ApplyPRR.set_Value_and_Lock(False)
#Frozen Showers
simFlags.LArParameterization.set_Value_and_Lock(0)
simFlags.CalibrationRun.set_On()
simFlags.CalibrationRun='DeadLAr'
