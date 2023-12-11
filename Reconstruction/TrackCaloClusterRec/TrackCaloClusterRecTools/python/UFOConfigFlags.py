# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createUFOConfigFlags():
    flags = AthConfigFlags()

    flags.addFlag("UFO.UseCov", True)
    flags.addFlag("UFO.dR", 0.05)
    return flags
