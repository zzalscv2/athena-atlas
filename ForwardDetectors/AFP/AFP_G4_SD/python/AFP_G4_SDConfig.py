# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AFP_SensitiveDetectorCfg(flags, name="AFP_SensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    volumeList=[] #["AFP::AFP*_LogSIDSensor*" , "AFP::AFP*_LogSIDVacuumSensor*" , "AFP::AFP*_LogTDSensor*"]
    for det in range(4):
        for num in range(4):
            volumeList += ["AFP::AFP0"+str(det)+"_LogSIDSensor["+str(num)+"]"]
    for det in range(4):
        volumeList += ["AFP::AFP0"+str(det)+"_LogSIDVacuumSensor[11]"]
    for det in [0,3]:
        for Q in range(1,2):
            for num in range(11,45):
                volumeList += ["AFP::AFP0"+str(det)+"_Q"+str(Q)+"_LogTDSensor["+str(num)+"]"]
                volumeList += ["AFP::AFP0"+str(det)+"_Q"+str(Q)+"_LogRadiator["+str(num)+"]"]
                volumeList += ["AFP::AFP0"+str(det)+"_Q"+str(Q)+"_LGuide["+str(num)+"]"]
    kwargs.setdefault("LogicalVolumeNames", volumeList)
    kwargs.setdefault("OutputCollectionNames", ["AFP_TDSimHitCollection", "AFP_SIDSimHitCollection"])
    result.setPrivateTools(CompFactory.AFP_SensitiveDetectorTool(name, **kwargs))
    return result


def AFP_SiDSensitiveDetectorCfg(flags, name="AFP_SiDSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    volumeList=[] #["AFP::AFP*_LogSIDSensor*" , "AFP::AFP*_LogSIDVacuumSensor*"]
    for det in range(4):
        for num in range(4):
            volumeList += ["AFP::AFP0"+str(det)+"_LogSIDSensor["+str(num)+"]"]
    for det in range(4):
        volumeList += ["AFP::AFP0"+str(det)+"_LogSIDVacuumSensor[11]"]
    kwargs.setdefault("LogicalVolumeNames", volumeList)
    kwargs.setdefault("OutputCollectionNames", ["AFP_SIDSimHitCollection"])
    result.setPrivateTools(CompFactory.AFP_SiDSensitiveDetectorTool(name, **kwargs))
    return result


def AFP_TDSensitiveDetectorCfg(flags, name="AFP_TDSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    volumeList=[] #["AFP::AFP*_LogTDSensor*"]
    for det in [0,3]:
        for Q in range(1,2):
            for num in range(11,45):
                volumeList += ["AFP::AFP0"+str(det)+"_Q"+str(Q)+"_LogTDSensor["+str(num)+"]"]
    kwargs.setdefault("LogicalVolumeNames", volumeList)
    kwargs.setdefault("OutputCollectionNames", ["AFP_TDSimHitCollection"])
    result.setPrivateTools(CompFactory.AFP_TDSensitiveDetectorTool(name, **kwargs))
    return result
