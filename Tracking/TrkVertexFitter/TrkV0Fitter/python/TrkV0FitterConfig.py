# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkV0Fitter package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetTrkV0VertexFitterCfg(config, name="InDetTrkV0VertexFitter", **kwargs):
    acc = ComponentAccumulator()
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(InDetExtrapolatorCfg(config)))
    acc.setPrivateTools(CompFactory.Trk.TrkV0VertexFitter(name, **kwargs))
    return acc

def TrkV0VertexFitterCfg(config, name="TrkV0VertexFitter", **kwargs):
    acc = ComponentAccumulator()
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(AtlasExtrapolatorCfg(config)))
    acc.setPrivateTools(CompFactory.Trk.TrkV0VertexFitter(name, **kwargs))
    return acc

