# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Resilience import protectedInclude
protectedInclude('Digitization/ForceUseOfPileUpTools.py')
protectedInclude('SimulationJobOptions/preInclude.PileUpBunchTrainsFill_7314_BCMS_Pattern_Flat.py')

if 'userRunLumiOverride' in dir():
    protectedInclude('RunDependentSimData/configLumi_muRange.py')
else:
    protectedInclude('RunDependentSimData/configLumi_run450000_mc23d_MultiBeamspot.py') 

from Digitization.DigitizationFlags import digitizationFlags
digitizationFlags.numberOfLowPtMinBias = 95.313 
digitizationFlags.numberOfHighPtMinBias = 0.187 
digitizationFlags.initialBunchCrossing = -32
digitizationFlags.finalBunchCrossing = 6
