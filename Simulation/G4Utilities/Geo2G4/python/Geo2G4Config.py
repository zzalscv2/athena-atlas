# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def Geo2G4SvcCfg(flags, **kwargs):
    result = ComponentAccumulator()
    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.TestBeam:
        kwargs.setdefault("GetTopTransform", False)
    result.addService(CompFactory.Geo2G4Svc(**kwargs), create=True, primary=True)
    return result
