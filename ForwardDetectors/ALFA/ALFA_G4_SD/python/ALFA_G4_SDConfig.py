# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ALFA_SensitiveDetectorCfg(flags, name="ALFA_SensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    volumeNames = [ "ALFA::logALFA_FiberU" , "ALFA::logALFA_FiberV" , "ALFA::logOD_FiberActive" ,
                    "ALFA::B7L1_GVS" , "ALFA::A7L1_GVS" , "ALFA::A7R1_GVS" , "ALFA::B7R1_GVS" ]
    for i in range(64):
        volumeNames += [ "ALFA::logALFA_FiberU["+str(i)+"]" ]
        volumeNames += [ "ALFA::logALFA_FiberV["+str(i)+"]" ]
    kwargs.setdefault("LogicalVolumeNames", volumeNames)
    kwargs.setdefault("OutputCollectionNames", ["ALFA_HitCollection", "ALFA_ODHitCollection"])
    result.setPrivateTools(CompFactory.ALFA_SensitiveDetectorTool(name, **kwargs))
    return result
