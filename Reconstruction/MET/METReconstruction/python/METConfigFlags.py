# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import HIMode
def createMETConfigFlags():
    metConfigFlags=AthConfigFlags()
    metConfigFlags.addFlag("MET.UseTracks",True)
    metConfigFlags.addFlag("MET.DoPFlow",True)
    metConfigFlags.addFlag("MET.UseFELinks",True)
    metConfigFlags.addFlag("MET.WritetoAOD", 
                           lambda prevFlags: prevFlags.Reco.HIMode in [HIMode.UPC,HIMode.HIP])
    return metConfigFlags
