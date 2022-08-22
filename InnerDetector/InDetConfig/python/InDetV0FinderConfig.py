# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def V0VKalVrtFitterCfg(config, inname, **kwargs):
    from TrkConfig.TrkVKalVrtFitterConfig import BPHY_TrkVKalVrtFitter_InDetExtrapCfg
    kwargs.setdefault("MakeExtendedVertex",      True)
    kwargs.setdefault("IterationNumber",      30)
    return BPHY_TrkVKalVrtFitter_InDetExtrapCfg(config, name=inname, **kwargs)

def InDetV0FinderToolCfg(config, name, V0ContainerName,
             KshortContainerName, LambdaContainerName, LambdabarContainerName, **kwargs):
    
    acc = ComponentAccumulator()
    if "V0Tools" not in kwargs:
        from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsNoExtrapCfg
        kwargs.setdefault("V0Tools",  acc.popToolsAndMerge(V0ToolsNoExtrapCfg(config, name+"_V0Tools")))
    acc.addPublicTool(kwargs["V0Tools"])

    if "VertexPointEstimator" not in kwargs:
        from TrkConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg
        vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(config))
        kwargs.setdefault("VertexPointEstimator",      vpest)
    acc.addPublicTool(kwargs["VertexPointEstimator"])

    if "VertexFitterTool" not in kwargs:
        from TrkV0Fitter.TrkV0FitterConfig import TrkV0VertexFitterCfg
        kwargs.setdefault("VertexFitterTool",  acc.popToolsAndMerge(TrkV0VertexFitterCfg(config)))
    acc.addPublicTool(kwargs["VertexFitterTool"])

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import  AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator",  acc.popToolsAndMerge(AtlasExtrapolatorCfg(config)))
    acc.addPublicTool(kwargs["Extrapolator"])
    if "TrackToVertexTool" not in kwargs:
        from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
        kwargs.setdefault("TrackToVertexTool",  acc.popToolsAndMerge(InDetTrackToVertexCfg(config)))
    acc.addPublicTool(kwargs["TrackToVertexTool"])
    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import V0InDetConversionTrackSelectorToolCfg
        kwargs.setdefault("TrackSelectorTool",  acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(config)))
    acc.addPublicTool(kwargs["TrackSelectorTool"])
    kwargs.setdefault("useV0Fitter",      True)
    kwargs.setdefault("V0ContainerName", V0ContainerName)
    kwargs.setdefault("KshortContainerName", KshortContainerName)
    kwargs.setdefault("LambdaContainerName", LambdaContainerName)
    kwargs.setdefault("LambdabarContainerName", LambdabarContainerName)
    kwargs.setdefault("doSimpleV0", False)
    kwargs.setdefault("useVertexCollection", True)
    if "VKVertexFitterTool" not in kwargs:
        kwargs.setdefault("VKVertexFitterTool",  acc.popToolsAndMerge(V0VKalVrtFitterCfg(config, "_BPhysVKVFitter")))
    if "KshortFitterTool" not in kwargs:
        kwargs.setdefault("KshortFitterTool", acc.popToolsAndMerge(V0VKalVrtFitterCfg(config, "_BPhysVKKVFitter",
                                                                             InputParticleMasses = [139.57,139.57],
                                                                             MassForConstraint   = 497.672)))
    if "LambdaFitterTool" not in kwargs:
        kwargs.setdefault("LambdaFitterTool", acc.popToolsAndMerge(V0VKalVrtFitterCfg(config, "_BPhysVKLFitter",
                                                                             InputParticleMasses = [938.272,139.57],
                                                                             MassForConstraint   = 1115.68)))
    if "LambdabarFitterTool" not in kwargs:
        kwargs.setdefault("LambdabarFitterTool",acc.popToolsAndMerge(V0VKalVrtFitterCfg(config, "_BPhysVKLbFitter",
                                                                             InputParticleMasses = [139.57,938.272],
                                                                             MassForConstraint   = 1115.68)))
    if "GammaFitterTool" not in kwargs:
        kwargs.setdefault("GammaFitterTool",acc.popToolsAndMerge(V0VKalVrtFitterCfg(config, "_BPhysVKGFitter",
                                                                             Robustness          = 6,
                                                                             usePhiCnst          = True,
                                                                             useThetaCnst        = True,
                                                                             InputParticleMasses = [0.511,0.511])))
    V0FinderTool = CompFactory.InDet.InDetV0FinderTool(name     = name, **kwargs)
    acc.setPrivateTools(V0FinderTool)
    return acc
