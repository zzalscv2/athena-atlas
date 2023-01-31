# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def CaloCellsHandlerToolCfg(flags):
    cfg = ComponentAccumulator()
    cfg.setPrivateTools(CompFactory.CaloCellsHandlerTool())
    return cfg


def GepClusteringAlgCfg(flags, name='GepClusteringAlg',
                        TopoClAlg='CaloWFS',
                        outputCaloClustersKey='CaloWFSTopoClusters',
                        OutputLevel=None):

    cfg = ComponentAccumulator()

    tool = cfg.popToolsAndMerge(CaloCellsHandlerToolCfg(flags))
    alg = CompFactory.GepClusteringAlg(
        name,
        TopoClAlg=TopoClAlg,
        outputCaloClustersKey=outputCaloClustersKey,
        CaloCellHandler=tool
    )

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)
    return cfg
    
                     
