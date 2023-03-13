# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Resilience import protectedInclude
protectedInclude('Digitization/ForceUseOfPileUpTools.py')
protectedInclude('SimulationJobOptions/preInclude.PileUpBunchTrainsFill_7314_BCMS_Pattern_Flat.py')

if 'userRunLumiOverride' in dir():
    protectedInclude('RunDependentSimData/configLumi_muRange.py')
else:
    protectedInclude('RunDependentSimData/configLumi_run450000_mc23c_MultiBeamspot.py')

from Digitization.DigitizationFlags import digitizationFlags
digitizationFlags.numberOfLowPtMinBias = 90.323
digitizationFlags.numberOfHighPtMinBias = 0.177
digitizationFlags.initialBunchCrossing = -32
digitizationFlags.finalBunchCrossing = 6
