# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def V0VtxPointEstimatorCfg(config, derivation):
    V0VtxPointEstimator = CompFactory.InDet.VertexPointEstimator(name        = derivation+"_VtxPointEstimator",
                                       MaxTrkXYDiffAtVtx      = [20.,20.,20.],
                                       MaxTrkZDiffAtVtx       = [100.,100.,100.],
                                       MaxTrkXYValue          = [400.,400.,400.],
                                       MinArcLength           = [-800.,-800.,-800.],
                                       MaxArcLength           = [800.,800.,800.],
                                       MinDeltaR              = [-10000.,-10000.,-10000.],
                                       MaxDeltaR              = [10000.,10000.,10000.],
                                       MaxPhi                 = [10000., 10000., 10000.],
                                       MaxChi2OfVtxEstimation = 2000.)
    acc = ComponentAccumulator()
    acc.setPrivateTools(V0VtxPointEstimator)
    return acc

def BPHY_InDetV0FinderToolCfg(config, derivation, V0ContainerName, KshortContainerName, LambdaContainerName, LambdabarContainerName):
    from InDetConfig.InDetV0FinderConfig import InDetV0FinderToolCfg
    from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
    acc = ComponentAccumulator()
    from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg
    args = { "VertexPointEstimator" : acc.popToolsAndMerge(V0VtxPointEstimatorCfg(config, derivation)),
             "TrackToVertexTool"    : acc.popToolsAndMerge(InDetTrackToVertexCfg(config)),
             "VertexFitterTool"     : acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(config))
           }
    V0FinderTool = acc.popToolsAndMerge(InDetV0FinderToolCfg(config, name = derivation + "_InDetV0FinderTool", V0ContainerName=V0ContainerName,
        KshortContainerName=KshortContainerName,
        LambdaContainerName=LambdaContainerName,
        LambdabarContainerName= LambdabarContainerName, **args))
    acc.setPrivateTools(V0FinderTool)
    return acc
