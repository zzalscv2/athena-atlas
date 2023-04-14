# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTruthAlgs package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def InDetPRDTruthTrajectorySorterCfg(
        flags, name='InDetTruthTrajectorySorter', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.InDet.PRD_TruthTrajectorySorterID(name, **kwargs))
    return result


def InDetPRD_ProviderCfg(flags, name='InDetPRD_Provider', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault('PixelClusterContainer', 'PixelClusters')
    kwargs.setdefault('SCT_ClusterContainer', 'SCT_Clusters')
    kwargs.setdefault('TRT_DriftCircleContainer', 'TRT_DriftCircles')
    result.setPrivateTools(CompFactory.InDet.InDetPRD_Provider(name, **kwargs))
    return result


def InDetPRD_TruthTrajectoryManipulatorIDCfg(
        flags, name='InDetTruthTrajectoryManipulator', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.InDet.PRD_TruthTrajectoryManipulatorID(name, **kwargs))
    return result


def InDetTruthTrackBuilderCfg(flags, name='InDetTruthTrackBuilder', **kwargs):
    result = ComponentAccumulator()

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetTrackFitterCfg
        kwargs.setdefault('TrackFitter', result.popToolsAndMerge(
            InDetTrackFitterCfg(flags)))

    if "ExtrapolationTool" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault('ExtrapolationTool', result.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if "RotCreatorTool" not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorCfg
        kwargs.setdefault('RotCreatorTool', result.popToolsAndMerge(
            InDetRotCreatorCfg(flags)))

    if "BroadRotCreatorTool" not in kwargs:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import (
            InDetBroadRotCreatorCfg)
        kwargs.setdefault('BroadRotCreatorTool', result.popToolsAndMerge(
            InDetBroadRotCreatorCfg(flags)))

    kwargs.setdefault('MinDegreesOfFreedom', 1)
    kwargs.setdefault('MatEffects', flags.Tracking.materialInteractionsType)
    kwargs.setdefault('MinSiHits',
                      flags.InDet.Tracking.ActiveConfig.minClusters)

    result.setPrivateTools(CompFactory.Trk.TruthTrackBuilder(name, **kwargs))
    return result


def InDetPRD_TruthTrajectoryBuilderCfg(
        flags, name='InDetPRD_TruthTrajectoryBuilder', **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault('PRD_MultiTruthCollections', [
        'PRD_MultiTruthPixel', 'PRD_MultiTruthSCT', 'PRD_MultiTruthTRT'])

    kwargs.setdefault('InDetPRD_Provider', result.popToolsAndMerge(
        InDetPRD_ProviderCfg(flags)))

    kwargs.setdefault('MinimumPt', flags.InDet.Tracking.ActiveConfig.minPT)

    manipulators = [result.popToolsAndMerge(
        InDetPRDTruthTrajectorySorterCfg(flags))]

    if not flags.Tracking.doIdealPseudoTracking:
        manipulators.append(result.popToolsAndMerge(
            InDetPRD_TruthTrajectoryManipulatorIDCfg(flags)))

    kwargs.setdefault('PRD_TruthTrajectoryManipulators', manipulators)

    result.setPrivateTools(
        CompFactory.Trk.PRD_TruthTrajectoryBuilder(name, **kwargs))
    return result


def InDetPRD_TruthTrajectorySelectorCfg(
        flags, name='InDetTruthTrajectorySelector', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.InDet.PRD_TruthTrajectorySelectorID(name, **kwargs))
    return result
