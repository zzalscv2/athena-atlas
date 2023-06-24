# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# configuration flags for the egamma derivations

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createEGammaDFConfigFlags():
    egdcf = AthConfigFlags()
    egdcf.addFlag("Derivation.Egamma.doTrackThinning", True)
    egdcf.addFlag("Derivation.Egamma.doEventInfoSlimming", False)
    egdcf.addFlag("Derivation.Egamma.addTriggerMatching", False)
    # ECIDS currently not supported in R22
    # For the moment just turn off, remove from code when
    # final decision is taken on whether to revive it or not
    egdcf.addFlag("Derivation.Egamma.addECIDS", False)
    return egdcf
