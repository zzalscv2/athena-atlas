# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from Campaigns.Utils import Campaign
athenaCommonFlags.MCCampaign.set_Value_and_Lock(Campaign.MC16d.value)

from AthenaCommon.BeamFlags import jobproperties as bf
bf.Beam.numberOfCollisions.set_Value_and_Lock(20.0)

from AthenaCommon.Resilience import protectedInclude
protectedInclude('LArConfiguration/LArConfigRun2Old.py')

protectedInclude('PyJobTransforms/HepMcParticleLinkVerbosity.py')
