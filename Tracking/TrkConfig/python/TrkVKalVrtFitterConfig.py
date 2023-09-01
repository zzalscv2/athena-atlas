# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVKalVrtFitter package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrkVKalVrtFitterCfg(flags, name="TrkVKalVrtFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **kwargs))
    return acc

def Conversion_TrkVKalVrtFitterCfg(
        flags, name="Conversion_TrkVKalVrtFitter", **kwargs):

    kwargs.setdefault("FirstMeasuredPoint", True)
    kwargs.setdefault("FirstMeasuredPointLimit", True)
    kwargs.setdefault("InputParticleMasses", [0.511, 0.511])
    kwargs.setdefault("IterationNumber", 30)
    kwargs.setdefault("MakeExtendedVertex", True)
    kwargs.setdefault("Robustness", 6)
    kwargs.setdefault("usePhiCnst", True)
    kwargs.setdefault("useThetaCnst", True)
    kwargs.setdefault("CovVrtForConstraint",
                      [0.015*0.015, 0., 0.015*0.015, 0., 0., 10000.*10000.])
    kwargs.setdefault("VertexForConstraint", [0., 0., 0.])

    return TrkVKalVrtFitterCfg(flags, name, **kwargs)

def BPHY_TrkVKalVrtFitterCfg(flags, name="BPHY_TrkVKalVrtFitter", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    kwargs.setdefault("FirstMeasuredPoint", False)
    kwargs.setdefault("MakeExtendedVertex", True)

    acc.setPrivateTools(acc.popToolsAndMerge(
        TrkVKalVrtFitterCfg(flags, name, **kwargs)))
    return acc

def V0VKalVrtFitterCfg(flags, name="V0VKalVrtFitter", **kwargs):
    kwargs.setdefault("MakeExtendedVertex", True)
    kwargs.setdefault("IterationNumber",    30)
    return BPHY_TrkVKalVrtFitterCfg(flags, name, **kwargs)

def JpsiV0VertexFitCfg(flags, name="JpsiV0VertexFit", **kwargs):
    kwargs.setdefault("CascadeCnstPrecision", 1e-6)
    return BPHY_TrkVKalVrtFitterCfg(flags, name, **kwargs)

def BTAG_TrkVKalVrtFitterCfg(flags, name="BTAG_TrkVKalVrtFitter",**kwargs):
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj
    myargs = kwargs.copy()
    myargs.setdefault("FirstMeasuredPoint", False)
    myargs.setdefault("FrozenVersionForBTagging", True)
    if "Extrapolator" in myargs:
       del myargs["Extrapolator"]
    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **myargs))
    return acc

