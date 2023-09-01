# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetConversionFinderTools package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ConversionPostSelectorCfg(flags, name="ConversionPostSelector", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MaxPhiVtxTrk",
                      flags.Egamma.PhotonConv.SecVtxPost.MaxPhiVtxTrk)
    kwargs.setdefault("MinRadius",
                      flags.Egamma.PhotonConv.SecVtxPost.MinRadius)

    kwargs.setdefault("MaxChi2Vtx", [50., 50., 50.])
    kwargs.setdefault("MaxInvariantMass", [10000., 10000., 10000.])
    kwargs.setdefault("MinFitMomentum", [0., 0., 0.])
    kwargs.setdefault("MinPt", 0.)
    kwargs.setdefault("MaxdR", -250.) # off

    acc.setPrivateTools(CompFactory.InDet.ConversionPostSelector(name, **kwargs))
    return acc

def SingleTrackConversionToolCfg(
        flags, name="SingleTrackConversionTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MaxBLayerHits",
                      flags.Egamma.PhotonConv.SingleTrk.MaxBLayerHits)
    kwargs.setdefault("MinInitialHitRadius",
                      flags.Egamma.PhotonConv.SingleTrk.MinInitialHitRadius)
    kwargs.setdefault("MinInitialHitRadius_noBlay",
                      flags.Egamma.PhotonConv.SingleTrk.MinInitialHitRadius_noBlay)
    kwargs.setdefault("MinRatioOfHLhits",
                      flags.Egamma.PhotonConv.SingleTrk.MinRatioOfHLhits)

    acc.setPrivateTools(
        CompFactory.InDet.SingleTrackConversionTool(name, **kwargs))
    return acc

def TrackPairsSelectorCfg(flags, name="TrackPairsSelector", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MaxDistBetweenTracks",
                      flags.Egamma.PhotonConv.TrkPairSel.MaxDistBetweenTracks)
    kwargs.setdefault("MaxEta",
                      flags.Egamma.PhotonConv.TrkPairSel.MaxEta)
    kwargs.setdefault("MinTrackAngle",
                      flags.Egamma.PhotonConv.TrkPairSel.MinTrackAngle)

    # hacky way to determine if TRT only of SI
    kwargs.setdefault("MaxFirstHitRadius", 500)
    kwargs.setdefault("MaxInitDistance", [10000.0, 10000.0, 10000.0])

    acc.setPrivateTools(CompFactory.InDet.TrackPairsSelector(name, **kwargs))
    return acc

def VertexPointEstimatorCfg(flags, name="VertexPointEstimator", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MinDeltaR", flags.Egamma.PhotonConv.VtxPt.MinDeltaR)
    kwargs.setdefault("MaxDeltaR", flags.Egamma.PhotonConv.VtxPt.MaxDeltaR)
    kwargs.setdefault("MaxPhi",    flags.Egamma.PhotonConv.VtxPt.MaxPhi)

    acc.setPrivateTools(CompFactory.InDet.VertexPointEstimator(name, **kwargs))
    return acc

def BPHY_VertexPointEstimatorCfg(
        flags, name="BPHY_VertexPointEstimator", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("MinDeltaR", [-10000., -10000., -10000.])
    kwargs.setdefault("MaxDeltaR", [ 10000.,  10000.,  10000.])
    kwargs.setdefault("MaxPhi",    [ 10000.,  10000.,  10000.])

    acc.setPrivateTools(CompFactory.InDet.VertexPointEstimator(name, **kwargs))
    return acc

def V0VertexPointEstimatorCfg(
        flags, name="InDetV0VertexPointEstimator", **kwargs):
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
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetConversionTrackSelectorToolCfg)
        kwargs.setdefault("TrackSelectorTool", acc.popToolsAndMerge(
            InDetConversionTrackSelectorToolCfg(flags)))

    if "VertexFitterTool" not in kwargs:
        from TrkConfig.TrkVKalVrtFitterConfig import (
            Conversion_TrkVKalVrtFitterCfg)
        kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(
            Conversion_TrkVKalVrtFitterCfg(flags)))

    kwargs.setdefault("IsConversion", True)

    kwargs.setdefault("MaxDistVtxHit",
                      flags.Egamma.PhotonConv.Finder.MaxDistVtxHit)
    kwargs.setdefault("MinDistVtxHit",
                      flags.Egamma.PhotonConv.Finder.MinDistVtxHit)
    kwargs.setdefault("MinFlightAngle",
                      flags.Egamma.PhotonConv.Finder.MinFlightAngle)
    kwargs.setdefault("MinInitVtxR",
                      flags.Egamma.PhotonConv.Finder.MinInitVtxR)
    kwargs.setdefault("RemoveTrtTracks",
                      flags.Egamma.PhotonConv.Finder.RemoveTrtTracks)

    acc.setPrivateTools(
        CompFactory.InDet.InDetConversionFinderTools(name, **kwargs))
    return acc
