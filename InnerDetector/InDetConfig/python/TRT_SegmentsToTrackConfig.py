# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TRT_SegmentsToTrack package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_SegmentsToTrackCfg(flags, name ='InDetTRT_SegmentsToTrack_Barrel', **kwargs):
    acc = ComponentAccumulator()

    if "ExtrapolationTool" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterCfg
        kwargs.setdefault("TrackFitter", acc.popToolsAndMerge(
            InDetTrackFitterCfg(flags)))

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolSharedHitsCfg
        kwargs.setdefault("SummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolSharedHitsCfg(flags)))

    if "AssociationTool" not in kwargs \
       and "InputAssociationMapName" in kwargs \
       and kwargs["InputAssociationMapName"] != '':
        from InDetConfig.InDetAssociationToolsConfig import InDetPRDtoTrackMapToolGangedPixelsCfg
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge( InDetPRDtoTrackMapToolGangedPixelsCfg(flags) ))

    kwargs.setdefault("MinNHit", flags.InDet.Tracking.ActivePass.minTRTonly)
    kwargs.setdefault("OutlierRemoval", True)
    kwargs.setdefault("MaterialEffects", False)

    acc.addEventAlgo(CompFactory.InDet.TRT_SegmentsToTrack(name, **kwargs))
    return acc
