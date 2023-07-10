# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPriVxFinderTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.VertexFindingFlags import VertexSetup

def AdaptiveMultiFindingBaseCfg(flags,
                                name="InDetAdaptiveMultiPriVxFinderTool",
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

    kwargs.setdefault("useBeamConstraint",
                      flags.Tracking.PriVertex.useBeamConstraint)
    kwargs.setdefault("selectiontype", 0)
    kwargs.setdefault("TracksMaxZinterval",
                      flags.Tracking.PriVertex.maxZinterval)
    kwargs.setdefault("do3dSplitting",
                      not flags.Tracking.PriVertex.useBeamConstraint)
    kwargs.setdefault("useSeedConstraint", False)

    acc.setPrivateTools(
        CompFactory.InDet.InDetAdaptiveMultiPriVxFinderTool(name, **kwargs))
    return acc

def GaussAdaptiveMultiFindingCfg(flags,
                                 name="GaussInDetAdaptiveMultiPriVxFinderTool",
                                 **kwargs):
    acc = ComponentAccumulator()
    
    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import (
            TrackDensitySeedFinderCfg)
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            TrackDensitySeedFinderCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        AdaptiveMultiFindingBaseCfg(flags, name, **kwargs)))
    return acc

def TrigGaussAdaptiveMultiFindingCfg(
        flags,
        name="InDetTrigAdaptiveMultiPriVxFinderTool",
        **kwargs):
    acc = ComponentAccumulator()

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            TrigVtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            TrigVtxInDetTrackSelectionCfg(flags)))

    kwargs.setdefault("useBeamConstraint", True)
    kwargs.setdefault("TracksMaxZinterval", flags.Tracking.ActiveConfig.TracksMaxZinterval)
    kwargs.setdefault("addSingleTrackVertices", flags.Tracking.ActiveConfig.addSingleTrackVertices)
    kwargs.setdefault("do3dSplitting", True) # NB: comment from original function suggests that this should be flags.InDet.doPrimaryVertex3DFinding

    acc.setPrivateTools(acc.popToolsAndMerge(
        GaussAdaptiveMultiFindingCfg(flags, name+flags.Tracking.ActiveConfig.input_name, **kwargs)))
    return acc

def IterativeFindingBaseCfg(flags,
                            name="InDetIterativePriVxFinderTool",
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
            AdaptiveVertexFitterCfg(
                flags,
                SeedFinder=kwargs["SeedFinder"],
                LinearizedTrackFactory=kwargs["LinearizedTrackFactory"],
                ImpactPoint3dEstimator=kwargs["ImpactPoint3dEstimator"])))

    kwargs.setdefault("useBeamConstraint",
                      flags.Tracking.PriVertex.useBeamConstraint)
    kwargs.setdefault("significanceCutSeeding", 12)
    kwargs.setdefault("maximumChi2cutForSeeding", 49)
    kwargs.setdefault("maxVertices", flags.Tracking.PriVertex.maxVertices)
    kwargs.setdefault("doMaxTracksCut", flags.Tracking.PriVertex.doMaxTracksCut)
    kwargs.setdefault("MaxTracks", flags.Tracking.PriVertex.maxTracks)

    acc.setPrivateTools(
        CompFactory.InDet.InDetIterativePriVxFinderTool(name, **kwargs))
    return acc

def FastIterativeFindingCfg(flags,
                            name="FastInDetIterativePriVxFinderTool",
                            **kwargs):
    acc = ComponentAccumulator()

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexBilloirToolsConfig import FastVertexFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            FastVertexFitterCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        IterativeFindingBaseCfg(flags, name, **kwargs)))
    return acc

def GaussIterativeFindingCfg(flags,
                             name="GaussInDetIterativePriVxFinderTool",
                             **kwargs):
    acc = ComponentAccumulator()

    if "SeedFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderToolsConfig import (
            TrackDensitySeedFinderCfg)
        kwargs.setdefault("SeedFinder", acc.popToolsAndMerge(
            TrackDensitySeedFinderCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(
        IterativeFindingBaseCfg(flags, name, **kwargs)))
    return acc

def TrigGaussIterativeFindingCfg(flags,
                                 name="InDetTrigPriVxFinderTool",
                                 **kwargs):
    acc = ComponentAccumulator()

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVertexBilloirToolsConfig import FastVertexFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            FastVertexFitterCfg(flags)))

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            TrigVtxInDetTrackSelectionCfg)
        kwargs.setdefault("TrackSelector", acc.popToolsAndMerge(
            TrigVtxInDetTrackSelectionCfg(flags)))

    from InDetTrigRecExample.TrigInDetConfiguredVtxCuts import (
        ConfiguredTrigVtxCuts)
    vtx_cuts = ConfiguredTrigVtxCuts()
    kwargs.setdefault("useBeamConstraint", True)
    kwargs.setdefault("maximumChi2cutForSeeding", 29)
    kwargs.setdefault("createSplitVertices", False)
    kwargs.setdefault("doMaxTracksCut", vtx_cuts.doMaxTracksCut())
    kwargs.setdefault("MaxTracks", vtx_cuts.MaxTracks())

    acc.setPrivateTools(acc.popToolsAndMerge(
        GaussIterativeFindingCfg(flags, name+flags.Tracking.ActiveConfig.input_name, **kwargs)))
    return acc



def VertexFinderToolCfg(flags, **kwargs):

    if flags.Tracking.PriVertex.setup == VertexSetup.IVF:
        return IterativeFindingBaseCfg(flags, **kwargs)
    elif flags.Tracking.PriVertex.setup == VertexSetup.FastIVF:
        return FastIterativeFindingCfg(flags, **kwargs)
    elif flags.Tracking.PriVertex.setup == VertexSetup.ActsGaussAMVF:
        from ActsConfig.ActsTrkPriVxFinderConfig import ActsGaussAdaptiveMultiFindingCfg
        return ActsGaussAdaptiveMultiFindingCfg(flags, **kwargs)

def TrigVertexFinderToolCfg(flags, **kwargs):
    from ActsConfig.ActsTrkPriVxFinderConfig import TrigActsGaussAdaptiveMultiFindingCfg

    if flags.Tracking.ActiveConfig.adaptiveVertex:
        if flags.Tracking.ActiveConfig.actsVertex:
            return TrigActsGaussAdaptiveMultiFindingCfg(flags, **kwargs)
        else:
            return TrigGaussAdaptiveMultiFindingCfg(flags, **kwargs)
    else:
        return TrigGaussIterativeFindingCfg(flags, **kwargs)
