# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetConversionFinderTools package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ConversionPostSelectorCfg(flags, name="ConversionPostSelector", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MaxChi2Vtx",
                      flags.Tracking.SecVertex.SecVtxPost.MaxChi2Vtx)
    kwargs.setdefault("MaxInvariantMass",
                      flags.Tracking.SecVertex.SecVtxPost.MaxInvariantMass)
    kwargs.setdefault("MaxPhiVtxTrk",
                      flags.Tracking.SecVertex.SecVtxPost.MaxPhiVtxTrk)
    kwargs.setdefault("MaxdR",
                      flags.Tracking.SecVertex.SecVtxPost.MaxdR)
    kwargs.setdefault("MinFitMomentum",
                      flags.Tracking.SecVertex.SecVtxPost.MinFitMomentum)
    kwargs.setdefault("MinPt",
                      flags.Tracking.SecVertex.SecVtxPost.MinPt)
    kwargs.setdefault("MinRadius",
                      flags.Tracking.SecVertex.SecVtxPost.MinRadius)

    acc.setPrivateTools(CompFactory.InDet.ConversionPostSelector(name, **kwargs))
    return acc

def SingleTrackConversionToolCfg(flags, name="SingleTrackConversionTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MaxBLayerHits",
                      flags.Tracking.SecVertex.SingleTrk.MaxBLayerHits)
    kwargs.setdefault("MinInitialHitRadius",
                      flags.Tracking.SecVertex.SingleTrk.MinInitialHitRadius)
    kwargs.setdefault("MinInitialHitRadius_noBlay",
                      flags.Tracking.SecVertex.SingleTrk.MinInitialHitRadius_noBlay)
    kwargs.setdefault("MinRatioOfHLhits",
                      flags.Tracking.SecVertex.SingleTrk.MinRatioOfHLhits)

    acc.setPrivateTools(CompFactory.InDet.SingleTrackConversionTool(name, **kwargs))
    return acc

def TrackPairsSelectorCfg(flags, name="TrackPairsSelector", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MaxDistBetweenTracks",
                      flags.Tracking.SecVertex.TrkPairSel.MaxDistBetweenTracks)
    kwargs.setdefault("MaxEta",
                      flags.Tracking.SecVertex.TrkPairSel.MaxEta)
    kwargs.setdefault("MaxFirstHitRadius",
                      flags.Tracking.SecVertex.TrkPairSel.MaxFirstHitRadius)
    kwargs.setdefault("MaxInitDistance",
                      flags.Tracking.SecVertex.TrkPairSel.MaxInitDistance)
    kwargs.setdefault("MinTrackAngle",
                      flags.Tracking.SecVertex.TrkPairSel.MinTrackAngle)

    acc.setPrivateTools(CompFactory.InDet.TrackPairsSelector(name, **kwargs))
    return acc

def VertexPointEstimatorCfg(flags, name="VertexPointEstimator", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MinDeltaR", flags.Tracking.SecVertex.VtxPt.MinDeltaR)
    kwargs.setdefault("MaxDeltaR", flags.Tracking.SecVertex.VtxPt.MaxDeltaR)
    kwargs.setdefault("MaxPhi",    flags.Tracking.SecVertex.VtxPt.MaxPhi)

    acc.setPrivateTools(CompFactory.InDet.VertexPointEstimator(name, **kwargs))
    return acc

def BPHY_VertexPointEstimatorCfg(flags, name="BPHY_VertexPointEstimator", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MinDeltaR", [-10000.,-10000.,-10000.])
    kwargs.setdefault("MaxDeltaR", [10000.,10000.,10000.])
    kwargs.setdefault("MaxPhi",    [10000., 10000., 10000.])

    acc.setPrivateTools(CompFactory.InDet.VertexPointEstimator(name, **kwargs))
    return acc

def V0VertexPointEstimatorCfg(flags, name="InDetV0VertexPointEstimator", **kwargs):
    kwargs.setdefault("MaxTrkXYDiffAtVtx", [ 20.,   20.,   20.])
    kwargs.setdefault("MaxTrkZDiffAtVtx",  [ 100.,  100.,  100.])
    kwargs.setdefault("MaxTrkXYValue",     [ 400.,  400.,  400.])
    kwargs.setdefault("MinArcLength",      [-800., -800., -800.])
    kwargs.setdefault("MaxArcLength",      [ 800.,  800.,  800.])
    return BPHY_VertexPointEstimatorCfg(flags, name, **kwargs)

def InDetConversionFinderToolsCfg(flags, name="ConversionFinderTool", **kwargs):
    """Configures InDet::InDetConversionFinderTools """

    acc = ComponentAccumulator()

    if "PostSelector" not in kwargs:
        kwargs.setdefault("PostSelector", acc.popToolsAndMerge(
            ConversionPostSelectorCfg(flags)))

    if "SingleTrackConversionTool" not in kwargs:
        kwargs.setdefault("SingleTrackConversionTool", acc.popToolsAndMerge(
            SingleTrackConversionToolCfg(flags)))

    if "TrackPairsSelector" not in kwargs:
        kwargs.setdefault("TrackPairsSelector", acc.popToolsAndMerge(
            TrackPairsSelectorCfg(flags)))

    if "VertexPointEstimator" not in kwargs:
        kwargs.setdefault("VertexPointEstimator", acc.popToolsAndMerge(
            VertexPointEstimatorCfg(flags)))

    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import InDetConversionTrackSelectorToolCfg
        kwargs.setdefault("TrackSelectorTool", acc.popToolsAndMerge(
            InDetConversionTrackSelectorToolCfg(flags)))

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVKalVrtFitterConfig import SecVx_TrkVKalVrtFitterCfg
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            SecVx_TrkVKalVrtFitterCfg(flags)))

    kwargs.setdefault("IsConversion", True)

    kwargs.setdefault("MaxDistVtxHit",
                      flags.Tracking.SecVertex.Finder.MaxDistVtxHit)
    kwargs.setdefault("MinDistVtxHit",
                      flags.Tracking.SecVertex.Finder.MinDistVtxHit)
    kwargs.setdefault("MinFlightAngle",
                      flags.Tracking.SecVertex.Finder.MinFlightAngle)
    kwargs.setdefault("MinInitVtxR",
                      flags.Tracking.SecVertex.Finder.MinInitVtxR)
    kwargs.setdefault("RemoveTrtTracks",
                      flags.Tracking.SecVertex.Finder.RemoveTrtTracks)

    acc.setPrivateTools(CompFactory.InDet.InDetConversionFinderTools(name, **kwargs))
    return acc
