# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkV0Fitter package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrkV0VertexFitterCfg(flags, name="TrkV0VertexFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("MaxIterations", 10)
    kwargs.setdefault("Use_deltaR", False)
    
    acc.setPrivateTools(CompFactory.Trk.TrkV0VertexFitter(name, **kwargs))
    return acc

def TrkV0VertexFitter_InDetExtrCfg(flags, name="TrkV0VertexFitter_InDetExtr", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(TrkV0VertexFitterCfg(flags, name, **kwargs)))
    return acc
