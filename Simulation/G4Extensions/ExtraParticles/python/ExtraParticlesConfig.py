# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
from ExtraParticles.PDGHelpers import getOriginalPDGTableName, PDGParser
from G4AtlasApps.SimFlags import simFlags

def getExtraParticlesPhysicsTool(name="ExtraParticlesPhysicsTool", **kwargs):
    pdgParser = PDGParser(getOriginalPDGTableName(simFlags.ExtraParticlesPDGTABLE.get_Value()),
        simFlags.ExtraParticlesRanges.get_Value() )
    kwargs.setdefault("ExtraParticlesConfig", pdgParser.createList())
    return CfgMgr.ExtraParticlesPhysicsTool(name, **kwargs)
