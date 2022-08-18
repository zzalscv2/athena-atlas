# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
from ExtraParticles.PDGHelpers import getPDGTABLE, PDGParser
from G4AtlasApps.SimFlags import simFlags

pdgParser = None
if getPDGTABLE(simFlags.ExtraParticlesPDGTABLE.get_Value()):
    pdgParser = PDGParser(simFlags.ExtraParticlesPDGTABLE.get_Value(),
                          simFlags.ExtraParticlesRanges.get_Value())
else:
    print("ERROR Failed to find PDGTABLE.MEV file.")


def getExtraParticlesPhysicsTool(name="ExtraParticlesPhysicsTool", **kwargs):
    kwargs.setdefault("ExtraParticlesConfig", pdgParser.createList())
    return CfgMgr.ExtraParticlesPhysicsTool(name, **kwargs)
