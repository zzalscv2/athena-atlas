# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createCaloRingerConfigFlags():

# CaloRinger flags
    caloRingercf = AthConfigFlags()

    caloRingercf.addFlag('CaloRinger.buildPhotonRings', False)
    caloRingercf.addFlag('CaloRinger.buildPhotonAsymRings', False)
    caloRingercf.addFlag('CaloRinger.buildElectronRings', True)
    caloRingercf.addFlag('CaloRinger.buildElectronAsymRings', False)
    caloRingercf.addFlag('CaloRinger.minElectronEnergy', 14)
    caloRingercf.addFlag('CaloRinger.minPhotonEnergy', 14)
    caloRingercf.addFlag('CaloRinger.useShowerShapeBarycenter', False)
    caloRingercf.addFlag('CaloRinger.doTransverseEnergy', True)

    return caloRingercf


if __name__ == "__main__":

    flags = createCaloRingerConfigFlags()
    flags.dump()
