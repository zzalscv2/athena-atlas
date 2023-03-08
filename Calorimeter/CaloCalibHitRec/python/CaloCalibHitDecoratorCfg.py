# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CaloCalibHitDecoratorCfg(flags):
    result=ComponentAccumulator()

    result.addEventAlgo(CompFactory.CaloCalibClusterTruthMapMakerAlgorithm())

    CaloCalibClusterDecoratorAlgorithm = CompFactory.CaloCalibClusterDecoratorAlgorithm()

    CaloCalibClusterDecoratorAlgorithm.TruthAttributerTool = CompFactory.CaloCalibClusterTruthAttributerTool()
    CaloCalibClusterDecoratorAlgorithm.CaloClusterWriteDecorHandleKey_NLeadingTruthParticles="CaloCalTopoClusters."+flags.Calo.TopoCluster.CalibrationHitDecorationName
    result.addEventAlgo(CaloCalibClusterDecoratorAlgorithm)

    return result
