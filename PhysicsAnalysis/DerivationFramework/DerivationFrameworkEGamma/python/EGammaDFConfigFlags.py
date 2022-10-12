# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# configuration flags for the egamma derivations

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createEGammaDFConfigFlags():
    egdcf = AthConfigFlags()
    egdcf.addFlag('Derivation.Egamma.doTrackThinning', True)
    egdcf.addFlag('Derivation.Egamma.doEventInfoSlimming', False)
    egdcf.addFlag('Derivation.Egamma.addTriggerMatching', False)  
    return egdcf
 
