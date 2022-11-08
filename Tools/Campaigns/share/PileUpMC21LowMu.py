# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Resilience import protectedInclude
protectedInclude('Digitization/ForceUseOfPileUpTools.py')
protectedInclude('SimulationJobOptions/preInclude.PileUpBunchTrainsFill_7314_BCMS_Pattern_Flat.py')

from Digitization.DigitizationFlags import digitizationFlags
digitizationFlags.numberOfLowPtMinBias = 0.0499
digitizationFlags.numberOfHighPtMinBias = 0.0001
digitizationFlags.initialBunchCrossing = -32
digitizationFlags.finalBunchCrossing = 6
