# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LUCID_SensitiveDetectorCfg(flags, name="LUCID_SensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("LogicalVolumeNames", ["LUCID::lvPmt"])
    kwargs.setdefault("OutputCollectionNames", ["LucidSimHitsVector"])
    result.setPrivateTools(CompFactory.LUCID_SensitiveDetectorTool(name, **kwargs))
    return result
