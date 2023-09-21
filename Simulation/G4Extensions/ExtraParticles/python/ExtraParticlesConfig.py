# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from ExtraParticles import PDGHelpers

@AccumulatorCache
def ExtraParticlesPhysicsToolCfg(flags, name="ExtraParticlesPhysicsTool", **kwargs):
    result = ComponentAccumulator()
    if PDGHelpers.getPDGTABLE('PDGTABLE.MeV'): # FIXME This should be a ConfigFlag
        parser = PDGHelpers.PDGParser('PDGTABLE.MeV', #flags.ExtraParticlesPDGTABLE,
                                      '111-556,1112-9090226') #flags.ExtraParticlesRanges) # FIXME need to add these as flags?
        kwargs.setdefault("ExtraParticlesConfig", parser.createList())
    else:
        raise RuntimeError('Failed to find PDGTABLE.MeV file')
    result.setPrivateTools(CompFactory.ExtraParticlesPhysicsTool(name, **kwargs))
    return result
