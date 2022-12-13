# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def CaloCellsHandlerToolCfg(flags):
    cfg = ComponentAccumulator()
    cfg.setPrivateTools(CompFactory.CaloCellsHandlerTool())
    return cfg


def GepClusterTimingAlgCfg(flags,
                           name='GepClusterTimingAlg',
                           inCaloClustersKey='CaloTopoClusters',
                           outCaloClustersKey='Clusters420Timing',
                           lambdaCalDivide=317,
                           qualityCut=0.02,
                           timeCutLargeQ=5,
                           timeCutSmallQ=15,
                           maxEtaForCut=5.0,
                           OutputLevel=None):

    cfg = ComponentAccumulator()

    alg = CompFactory.GepClusterTimingAlg(
        name=name,
        inCaloClustersKey=inCaloClustersKey,
        outCaloClustersKey=outCaloClustersKey,
        lambdaCalDivide=lambdaCalDivide,
        qualityCut=qualityCut,
        timeCutLargeQ=timeCutLargeQ,
        timeCutSmallQ=timeCutSmallQ,
        maxEtaForCut=maxEtaForCut
    )

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel
        
    cfg.addEventAlgo(alg)
    return cfg
    
                     
