print ('SINGLE PARTICLE GENERATOR')

## Run ParticleGun
import AthenaCommon.AtlasUnixGeneratorJob

from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()

import ParticleGun as PG
from G4AtlasApps.SimFlags import simFlags
pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = simFlags.RandomSeedOffset.get_Value())
pg.sampler.pid = 13
pg.sampler.mom = PG.EEtaMPhiSampler(energy=10000, eta=[-1,1])
topSeq += pg

include("G4AtlasApps/fragment.SimCopyWeights.py")
