# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexAnalysisUtils package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def V0ToolsCfg(flags, name="V0Tools", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        AtlasExtrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        acc.addPublicTool(AtlasExtrapolator)
        kwargs.setdefault("Extrapolator", AtlasExtrapolator)

    acc.setPrivateTools(CompFactory.Trk.V0Tools(name, **kwargs))
    return acc

def V0ToolsNoExtrapCfg(flags, name="V0Tools_NoExtrap", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("Extrapolator", None)
    kwargs.setdefault("DisableExtrapolator", True)
    acc.setPrivateTools(CompFactory.Trk.V0Tools(name, **kwargs))
    return acc
