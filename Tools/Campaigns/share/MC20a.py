# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.BeamFlags import jobproperties as bf
bf.Beam.numberOfCollisions.set_Value_and_Lock(20.0)

from LArDigitization.LArDigitizationFlags import jobproperties as lar
lar.LArDigitizationFlags.useEmecIwHighGain.set_Value_and_Lock(True)

from AthenaCommon.Resilience import protectedInclude
protectedInclude('LArConfiguration/LArConfigRun2Old.py')
