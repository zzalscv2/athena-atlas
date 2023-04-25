# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiZvertexTool_xk package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def SiZvertexMaker_xkCfg(flags, name="InDetZvertexMaker", **kwargs) :
    acc = ComponentAccumulator()

    kwargs.setdefault("Zmax", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("Zmin", -flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("minRatio", 0.17)

    if "SeedMakerTool" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import (
            SiSpacePointsSeedMakerCfg)
        kwargs.setdefault("SeedMakerTool", acc.popToolsAndMerge(
            SiSpacePointsSeedMakerCfg(flags)))

    if flags.Reco.EnableHI:
        kwargs.setdefault("HistSize", 2000)
        kwargs.setdefault("minContent", 30)

    acc.setPrivateTools(CompFactory.InDet.SiZvertexMaker_xk(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc
