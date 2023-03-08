# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVKalVrtFitter package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrkVKalVrtFitterCfg(flags, name="TrkVKalVrtFitter", **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc = AtlasFieldCacheCondAlgCfg(flags) # To produce AtlasFieldCacheCondObj

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.TrkVKalVrtFitter(name, **kwargs))
    return acc

def SecVx_TrkVKalVrtFitterCfg(flags, name="SecVx_TrkVKalVrtFitter", **kwargs):
    kwargs.setdefault("FirstMeasuredPoint",
                      flags.Tracking.SecVertex.Fitter.FirstMeasuredPoint)
    kwargs.setdefault("FirstMeasuredPointLimit",
                      flags.Tracking.SecVertex.Fitter.FirstMeasuredPointLimit)
    kwargs.setdefault("InputParticleMasses",
                      flags.Tracking.SecVertex.Fitter.InputParticleMasses)
    kwargs.setdefault("IterationNumber",
                      flags.Tracking.SecVertex.Fitter.IterationNumber)
    kwargs.setdefault("MakeExtendedVertex",
                      flags.Tracking.SecVertex.Fitter.MakeExtendedVertex)
    kwargs.setdefault("Robustness",
                      flags.Tracking.SecVertex.Fitter.Robustness)
    kwargs.setdefault("usePhiCnst",
                      flags.Tracking.SecVertex.Fitter.usePhiCnst)
    kwargs.setdefault("useThetaCnst",
                      flags.Tracking.SecVertex.Fitter.useThetaCnst)
    kwargs.setdefault("CovVrtForConstraint",
                      flags.Tracking.SecVertex.Fitter.CovVrtForConstraint)
    kwargs.setdefault("VertexForConstraint",
                      flags.Tracking.SecVertex.Fitter.VertexForConstraint)

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
    acc = ComponentAccumulator()

    kwargs.setdefault("MakeExtendedVertex", True)
    kwargs.setdefault("IterationNumber",    30)

    acc.setPrivateTools(acc.popToolsAndMerge(
        BPHY_TrkVKalVrtFitterCfg(flags, name, **kwargs)))
    return acc
