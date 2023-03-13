# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def TrigTRTHTHCounterFex(flags, name, RoIs, containerName, RNNOutputName):
    alg = CompFactory.TrigTRTHTHCounter(
        name,
        RoIs = RoIs,
        TRT_DC_ContainerName = containerName,
        RNNOutputName = RNNOutputName,
        EtaHalfWidth =  0.1,
        PhiHalfWidth = 0.1,
        doFullScan =  False,
        RoadWidth = 4,
        nBinCoarse = 14,
        nBinFine = 14,
        WedgeMinEta = 0,
        RoadMaxEta = 1.06,
        WedgeNBin = 5 )

    return alg
