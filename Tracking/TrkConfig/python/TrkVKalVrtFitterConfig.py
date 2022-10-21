# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVKalVrtFitter package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def VSI_VKalVrtFitterCfg(flags, name="VSI_TrkVKalVrtFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("IterationNumber", flags.InDet.SecVertex.Fitter.IterationNumber)

    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **kwargs))
    return acc

def TrkVKalVrtFitterCfg(flags, name="TrkVKalVrtFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("FirstMeasuredPoint", flags.InDet.SecVertex.Fitter.FirstMeasuredPoint)
    kwargs.setdefault("FirstMeasuredPointLimit", flags.InDet.SecVertex.Fitter.FirstMeasuredPointLimit)
    kwargs.setdefault("InputParticleMasses", flags.InDet.SecVertex.Fitter.InputParticleMasses)
    kwargs.setdefault("IterationNumber", flags.InDet.SecVertex.Fitter.IterationNumber)
    kwargs.setdefault("MakeExtendedVertex", flags.InDet.SecVertex.Fitter.MakeExtendedVertex)
    kwargs.setdefault("Robustness", flags.InDet.SecVertex.Fitter.Robustness)
    kwargs.setdefault("usePhiCnst", flags.InDet.SecVertex.Fitter.usePhiCnst)
    kwargs.setdefault("useThetaCnst", flags.InDet.SecVertex.Fitter.useThetaCnst)
    kwargs.setdefault("CovVrtForConstraint", flags.InDet.SecVertex.Fitter.CovVrtForConstraint)
    kwargs.setdefault("VertexForConstraint", flags.InDet.SecVertex.Fitter.VertexForConstraint)

    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **kwargs))
    return acc

def BPHY_TrkVKalVrtFitterCfg(flags, name="BPHY_TrkVKalVrtFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    kwargs.setdefault("FirstMeasuredPoint", False)
    kwargs.setdefault("MakeExtendedVertex", True)

    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **kwargs))
    return acc

def BPHY_TrkVKalVrtFitter_InDetExtrapCfg(flags, name="BPHY_TrkVKalVrtFitter_InDetExtrap", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        BPHY_TrkVKalVrtFitterCfg(flags, name, **kwargs)))
    return acc
