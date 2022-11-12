# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkRefitAlg package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Configuration not supported, to be recommissioned if needed
def ReFitTrackAlgCfg(flags, name="InDetRefitTrack", **kwargs):

    result = ComponentAccumulator()

    if "FitterTool" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterCfg
        kwargs.setdefault("FitterTool", result.popToolsAndMerge(
            InDetTrackFitterCfg(flags)))

    if "FitterToolTRT" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterTRTCfg
        kwargs.setdefault("FitterToolTRT", result.popToolsAndMerge(
            InDetTrackFitterTRTCfg(flags)))

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolSharedHitsCfg
        kwargs.setdefault("SummaryTool", result.popToolsAndMerge(
            InDetTrackSummaryToolSharedHitsCfg(flags)))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import InDetPRDtoTrackMapToolGangedPixelsCfg
        kwargs.setdefault("AssociationTool", result.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("TrackName", "CombinedInDetTracks")
    kwargs.setdefault("NewTrackName", "RefittedTracks")
    kwargs.setdefault("useParticleHypothesisFromTrack", True)
    kwargs.setdefault("matEffects", flags.InDet.Tracking.materialInteractionsType if flags.InDet.Tracking.materialInteractions else 0)

    result.addEventAlgo(CompFactory.Trk.ReFitTrack(name, **kwargs))
    return result
