"""ComponentAccumulator config of tools for ISF_FastCaloSimParametrization

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from FastCaloSim.FastCaloSimFactoryNew import NITimedExtrapolatorCfg

def FastCaloSimCaloExtrapolationCfg(flags, name="FastCaloSimCaloExtrapolation", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("CaloBoundaryR", [1148.0, 120.0, 41.0])
    kwargs.setdefault("CaloBoundaryZ", [3550.0, 4587.0, 4587.0])
    kwargs.setdefault("CaloMargin", 100)
    kwargs.setdefault("Extrapolator", acc.addPublicTool(acc.popToolsAndMerge(NITimedExtrapolatorCfg(flags))))
    kwargs.setdefault("CaloGeometryHelper", acc.addPublicTool(acc.popToolsAndMerge(FastCaloSimGeometryHelperCfg(flags))))
    kwargs.setdefault("CaloEntrance", 'InDet::Containers::InnerDetector')

    acc.setPrivateTools(CompFactory.FastCaloSimCaloExtrapolation(name, **kwargs))
    return acc


def FastCaloSimGeometryHelperCfg(flags, name="FastCaloSimGeometryHelper", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.FastCaloSimGeometryHelper(name, **kwargs))
    return acc
