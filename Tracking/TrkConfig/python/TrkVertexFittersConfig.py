# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexFitters package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AdaptiveMultiVertexFitterCfg(flags, name="AdaptiveMultiVertexFitter", **kwargs):
    acc = ComponentAccumulator()

    if "LinearizedTrackFactory" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            FullLinearizedTrackFactoryCfg)
        kwargs.setdefault("LinearizedTrackFactory", acc.popToolsAndMerge(
            FullLinearizedTrackFactoryCfg(flags)))

    if "ImpactPoint3dEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            ImpactPoint3dEstimatorCfg)
        kwargs.setdefault("ImpactPoint3dEstimator", acc.popToolsAndMerge(
            ImpactPoint3dEstimatorCfg(flags)))

    if "AnnealingMaker" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import DetAnnealingMakerCfg
        kwargs.setdefault("AnnealingMaker", acc.popToolsAndMerge(
            DetAnnealingMakerCfg(flags)))

    kwargs.setdefault("DoSmoothing", True)

    acc.setPrivateTools(CompFactory.Trk.AdaptiveMultiVertexFitter(name, **kwargs))
    return acc

def SequentialVertexSmootherCfg(flags, name="SequentialVertexSmoother", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.SequentialVertexSmoother())
    return acc

def AdaptiveVertexFitterCfg(flags, name="AdaptiveVertexFitter", **kwargs):
    acc = ComponentAccumulator()

    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import ZScanSeedFinderCfg
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            ZScanSeedFinderCfg(flags)))

    if "LinearizedTrackFactory" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            FullLinearizedTrackFactoryCfg)
        kwargs.setdefault("LinearizedTrackFactory", acc.popToolsAndMerge(
            FullLinearizedTrackFactoryCfg(flags)))

    if "ImpactPoint3dEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            ImpactPoint3dEstimatorCfg)
        kwargs.setdefault("ImpactPoint3dEstimator", acc.popToolsAndMerge(
            ImpactPoint3dEstimatorCfg(flags)))

    if "AnnealingMaker" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import DetAnnealingMakerCfg
        kwargs.setdefault("AnnealingMaker", acc.popToolsAndMerge(
            DetAnnealingMakerCfg(flags)))

    if "VertexSmoother" not in kwargs:
        kwargs.setdefault("VertexSmoother", acc.popToolsAndMerge(
            SequentialVertexSmootherCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.AdaptiveVertexFitter(name, **kwargs))
    return acc
