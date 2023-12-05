# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from Campaigns.Utils import Campaign
athenaCommonFlags.MCCampaign.set_Value_and_Lock(Campaign.MC23d.value)

from AthenaCommon.BeamFlags import jobproperties as bf
bf.Beam.numberOfCollisions.set_Value_and_Lock(60.0)

from Digitization.DigitizationFlags import digitizationFlags
from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
digitizationFlags.pixelPlanarRadiationDamageSimulationType.set_Value_and_Lock(PixelRadiationDamageSimulationType.RamoPotential.value)

from AthenaCommon.Resilience import protectedInclude
protectedInclude('LArConfiguration/LArConfigRun3Old.py')

protectedInclude('PyJobTransforms/HepMcParticleLinkVerbosity.py')
