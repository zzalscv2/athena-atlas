# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

#todo? add in the explanatory text from previous implementation

def createTestBeamConfigFlags():
    tbcf = AthConfigFlags()
    tbcf.addFlag("TestBeam.Eta", None)
    tbcf.addFlag("TestBeam.Theta", None)
    tbcf.addFlag("TestBeam.Phi", None)
    tbcf.addFlag("TestBeam.Y", None)
    tbcf.addFlag("TestBeam.Z", None)

    return tbcf


def testBeamRunArgsToFlags(runArgs, flags):
    if hasattr(runArgs,"Eta") and ( hasattr(runArgs,"Theta") or hasattr(runArgs,"Z") ):
        raise RuntimeError("Eta cannot be specified at the same time as Theta and Z.")

    if hasattr(runArgs,"Eta"):
        flags.TestBeam.Eta = runArgs.Eta

    if hasattr(runArgs,"Theta") or hasattr(runArgs,"Z"):
        if hasattr(runArgs,"Theta"):
            flags.TestBeam.Theta = runArgs.Theta
        else:
            # Z specified on the commmand-line without Theta
            flags.TestBeam.Theta=90

        if hasattr(runArgs,"Z"):
            flags.TestBeam.Z = runArgs.Z
        else:
            # Theta specified on the commmand-line without Z
            flags.TestBeam.Z = 2550.0

    if hasattr(runArgs,"Phi"):
        flags.TestBeam.Phi=runArgs.Phi

    if hasattr(runArgs,"Y"):
        flags.TestBeam.Y=runArgs.Y
