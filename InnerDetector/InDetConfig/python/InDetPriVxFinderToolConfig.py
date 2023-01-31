# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPriVxFinderTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from InDetConfig.VertexFindingFlags import VertexSetup

def AdaptiveMultiFindingBaseCfg(flags, name="InDetAdaptiveMultiPriVxFinderTool",
                                **kwargs):
    acc = ComponentAccumulator()
    
    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import ZScanSeedFinderCfg
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            ZScanSeedFinderCfg(flags)))

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            VtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            VtxInDetTrackSelectionCfg(flags)))

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexFittersConfig import (
            AdaptiveMultiVertexFitterCfg)
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            AdaptiveMultiVertexFitterCfg(flags)))

    vtxFlags = flags.ITk.PriVertex if flags.Detector.GeometryITk \
               else flags.InDet.PriVertex

    kwargs.setdefault("useBeamConstraint", vtxFlags.useBeamConstraint)
    kwargs.setdefault("selectiontype", 0)
    kwargs.setdefault("TracksMaxZinterval", vtxFlags.maxZinterval)
    kwargs.setdefault("do3dSplitting", not vtxFlags.useBeamConstraint)
    kwargs.setdefault("useSeedConstraint", False)

    acc.setPrivateTools(
        CompFactory.InDet.InDetAdaptiveMultiPriVxFinderTool(name, **kwargs))
    return acc

def GaussAdaptiveMultiFindingCfg(flags, name="GaussInDetAdaptiveMultiPriVxFinderTool", **kwargs):
    acc = ComponentAccumulator()
    
    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import (
            TrackDensitySeedFinderCfg)
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            TrackDensitySeedFinderCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        AdaptiveMultiFindingBaseCfg(flags, name, **kwargs)))
    return acc

def TrigGaussAdaptiveMultiFindingCfg(flags, name="InDetTrigAdaptiveMultiPriVxFinderTool", signature="", **kwargs):
    acc = ComponentAccumulator()

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            TrigVtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            TrigVtxInDetTrackSelectionCfg(flags, signature = signature)))

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    config = getInDetTrigConfig(signature)

    kwargs.setdefault("useBeamConstraint", True)
    kwargs.setdefault("TracksMaxZinterval", config.TracksMaxZinterval)
    kwargs.setdefault("addSingleTrackVertices", config.addSingleTrackVertices)
    kwargs.setdefault("do3dSplitting", True) # NB: comment from original function suggests that this should be flags.InDet.doPrimaryVertex3DFinding

    acc.setPrivateTools(acc.popToolsAndMerge(
        GaussAdaptiveMultiFindingCfg(flags, name+signature, **kwargs)))
    return acc


def IterativeFindingBaseCfg(flags, name="InDetIterativePriVxFinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import ZScanSeedFinderCfg
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            ZScanSeedFinderCfg(flags)))

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            VtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            VtxInDetTrackSelectionCfg(flags)))

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

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexFittersConfig import AdaptiveVertexFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            AdaptiveVertexFitterCfg(flags, SeedFinder=kwargs["SeedFinder"],
                                    LinearizedTrackFactory=kwargs["LinearizedTrackFactory"],
                                    ImpactPoint3dEstimator=kwargs["ImpactPoint3dEstimator"])))
                                    

    vtxFlags = flags.ITk.PriVertex if flags.Detector.GeometryITk \
               else flags.InDet.PriVertex

    kwargs.setdefault("useBeamConstraint", vtxFlags.useBeamConstraint)
    kwargs.setdefault("significanceCutSeeding", 12)
    kwargs.setdefault("maximumChi2cutForSeeding", 49)
    kwargs.setdefault("maxVertices", 200)
    kwargs.setdefault("doMaxTracksCut", vtxFlags.doMaxTracksCut)
    kwargs.setdefault("MaxTracks", vtxFlags.MaxTracks)

    acc.setPrivateTools(
        CompFactory.InDet.InDetIterativePriVxFinderTool(name, **kwargs))
    return acc

def GaussIterativeFindingCfg(flags, name="GaussInDetIterativePriVxFinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import (
            TrackDensitySeedFinderCfg)
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            TrackDensitySeedFinderCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        IterativeFindingBaseCfg(flags, name, **kwargs)))
    return acc

def TrigGaussIterativeFindingCfg(flags, name="InDetTrigPriVxFinderTool", signature="", **kwargs):
    acc = ComponentAccumulator()

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexBilloirToolsConfig import FastVertexFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            FastVertexFitterCfg(flags)))

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            TrigVtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            TrigVtxInDetTrackSelectionCfg(flags, signature = signature)))

    from InDetTrigRecExample.TrigInDetConfiguredVtxCuts import ConfiguredTrigVtxCuts
    vtx_cuts = ConfiguredTrigVtxCuts()
    kwargs.setdefault("useBeamConstraint", True)
    kwargs.setdefault("maximumChi2cutForSeeding", 29)
    kwargs.setdefault("createSplitVertices", False)
    kwargs.setdefault("doMaxTracksCut", vtx_cuts.doMaxTracksCut())
    kwargs.setdefault("MaxTracks", vtx_cuts.MaxTracks())

    acc.setPrivateTools(acc.popToolsAndMerge(
        GaussIterativeFindingCfg(flags, name+signature, **kwargs)))
    return acc


def ActsGaussAdaptiveMultiFindingBaseCfg(flags, name="ActsAdaptiveMultiPriVtxFinderTool", **kwargs):
    acc = ComponentAccumulator()

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            VtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            VtxInDetTrackSelectionCfg(flags)))

    if "TrackingGeometryTool" not in kwargs:
        from ActsGeometry.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(
            ActsTrackingGeometryToolCfg(flags)))

    if "ExtrapolationTool" not in kwargs:
        from ActsGeometry.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault("ExtrapolationTool", acc.getPrimaryAndMerge(
            ActsExtrapolationToolCfg(flags)))

    vtxFlags = flags.ITk.PriVertex if flags.Detector.GeometryITk \
               else flags.InDet.PriVertex

    kwargs.setdefault("useBeamConstraint", vtxFlags.useBeamConstraint)
    kwargs.setdefault("tracksMaxZinterval", vtxFlags.maxZinterval)
    kwargs.setdefault("do3dSplitting", not vtxFlags.useBeamConstraint)

    acc.setPrivateTools(
        CompFactory.ActsTrk.AdaptiveMultiPriVtxFinderTool(name, **kwargs))
    return acc

def VertexFinderToolCfg(flags, **kwargs):

    vtxFlags = flags.ITk.PriVertex if flags.Detector.GeometryITk \
               else flags.InDet.PriVertex     

    if vtxFlags.setup == VertexSetup.GaussAMVF:
        return GaussAdaptiveMultiFindingCfg(flags)
    elif vtxFlags.setup == VertexSetup.AMVF:
        return AdaptiveMultiFindingBaseCfg(flags)
    elif vtxFlags.setup == VertexSetup.GaussIVF:
        return GaussIterativeFindingCfg(flags)
    elif vtxFlags.setup == VertexSetup.IVF:
        return IterativeFindingBaseCfg(flags)
    elif vtxFlags.setup == VertexSetup.ActsGaussAMVF:
        return ActsGaussAdaptiveMultiFindingBaseCfg(flags)
