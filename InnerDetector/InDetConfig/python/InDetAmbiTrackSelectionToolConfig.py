# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetAmbiTrackSelectionTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod


def InDetAmbiTrackSelectionToolCfg(flags, name="InDetAmbiTrackSelectionTool", **kwargs):
    acc = ComponentAccumulator()

    if ('UseParameterization' in kwargs and
        kwargs.get('UseParameterization', False) and
            "DriftCircleCutTool" not in kwargs):
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        kwargs.setdefault("DriftCircleCutTool", acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags)))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minHits",
                      flags.InDet.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minNotShared",
                      flags.InDet.Tracking.ActiveConfig.minSiNotShared)
    kwargs.setdefault("maxShared",
                      flags.InDet.Tracking.ActiveConfig.maxShared)
    kwargs.setdefault("minTRTHits", 0)  # used for Si only tracking !!!
    kwargs.setdefault("UseParameterization", False)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("doPixelSplitting",
                      flags.InDet.Tracking.doPixelClusterSplitting)

    if flags.InDet.Tracking.ActiveConfig.useTIDE_Ambi:
        kwargs.setdefault("sharedProbCut",
                          flags.InDet.Tracking.pixelClusterSplitProb1)
        kwargs.setdefault("sharedProbCut2",
                          flags.InDet.Tracking.pixelClusterSplitProb2)
        kwargs.setdefault("minSiHitsToAllowSplitting",
                          8 if flags.GeoModel.Run is LHCPeriod.Run1 else 9)
        kwargs.setdefault("minUniqueSCTHits", 4)
        kwargs.setdefault("minTrackChi2ForSharedHits", 3)
        # Only allow split clusters on track withe pt greater than this MeV
        kwargs.setdefault("minPtSplit", 1000)
        # Maximum number of shared modules for tracks in ROI
        kwargs.setdefault("maxSharedModulesInROI", 3)
        # Minimum number of unique modules for tracks in ROI
        kwargs.setdefault("minNotSharedInROI", 2)
        # Minimum number of Si hits to allow splittings for tracks in ROI
        kwargs.setdefault("minSiHitsToAllowSplittingInROI", 8)
        # Split cluster ROI size
        kwargs.setdefault("phiWidth", 0.05)
        kwargs.setdefault("etaWidth", 0.05)

        # Only split in cluster in region of interest
        kwargs.setdefault("doEmCaloSeed",
                          flags.InDet.Tracking.doCaloSeededAmbi)
        kwargs.setdefault("EMROIPhiRZContainer",
                          "InDetCaloClusterROIPhiRZ10GeV")
        if flags.InDet.Tracking.doCaloSeededAmbi:
            from InDetConfig.InDetCaloClusterROISelectorConfig import (
                CaloClusterROIPhiRZContainerMakerCfg)
            acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))

        # Do special cuts in region of interest
        kwargs.setdefault("doHadCaloSeed",
                          flags.InDet.Tracking.doCaloSeededAmbi)
        kwargs.setdefault("HadROIPhiRZContainer",
                          "InDetHadCaloClusterROIPhiRZBjet")
        if flags.InDet.Tracking.doCaloSeededAmbi:
            from InDetConfig.InDetCaloClusterROISelectorConfig import (
                HadCaloClusterROIPhiRZContainerMakerCfg)
            acc.merge(HadCaloClusterROIPhiRZContainerMakerCfg(flags))

        # Do special cuts in region of interest
        kwargs.setdefault("minPtConv", 10000)
        kwargs.setdefault("minPtBjetROI", 10000)
        # Split cluster ROI size
        kwargs.setdefault("phiWidthEM", 0.05)
        kwargs.setdefault("etaWidthEM", 0.05)
        # Skip ambi solver in hadronic ROI
        kwargs.setdefault("doSkipAmbiInROI", flags.InDet.Tracking.doSkipAmbiROI)

        if (flags.Tracking.doTIDE_AmbiTrackMonitoring and
                flags.InDet.Tracking.ActiveConfig.extension == ""):
            from TrkConfig.TrkValToolsConfig import TrkObserverToolCfg
            TrkObserverTool = acc.popToolsAndMerge(TrkObserverToolCfg(flags))
            acc.addPublicTool(TrkObserverTool)
            kwargs.setdefault("ObserverTool", TrkObserverTool)

    else:
        kwargs.setdefault("sharedProbCut", 0.10)

    if flags.InDet.Tracking.ActiveConfig.useTIDE_Ambi:
        AmbiTrackSelectionTool = CompFactory.InDet.InDetDenseEnvAmbiTrackSelectionTool
    else:
        AmbiTrackSelectionTool = CompFactory.InDet.InDetAmbiTrackSelectionTool

    InDetAmbiTrackSelectionTool = AmbiTrackSelectionTool(
        name=name+flags.InDet.Tracking.ActiveConfig.extension, **kwargs)
    acc.setPrivateTools(InDetAmbiTrackSelectionTool)
    return acc


def InDetTRTAmbiTrackSelectionToolCfg(flags, name='InDetTRT_SeededAmbiTrackSelectionTool', **kwargs):
    acc = ComponentAccumulator()

    if "DriftCircleCutTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        kwargs.setdefault("DriftCircleCutTool", acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags)))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minScoreShareTracks", -1.)  # off !
    kwargs.setdefault("minHits",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryClusters)
    kwargs.setdefault("minNotShared",
                      flags.InDet.Tracking.ActiveConfig.minSecondarySiNotShared)
    kwargs.setdefault("maxShared",
                      flags.InDet.Tracking.ActiveConfig.maxSecondaryShared)
    kwargs.setdefault("minTRTHits",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryTRTonTrk)
    kwargs.setdefault("UseParameterization",
                      flags.InDet.Tracking.ActiveConfig.useParameterizedTRTCuts)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("doPixelSplitting",
                      flags.InDet.Tracking.doPixelClusterSplitting)

    acc.setPrivateTools(
        CompFactory.InDet.InDetAmbiTrackSelectionTool(name, **kwargs))
    return acc


def InDetTrigTrackSelectionToolCfg(flags, name='InDetTrigAmbiTrackSelectionTool', **kwargs):
    acc = ComponentAccumulator()

    # TODO add configurations for beamgas and cosmic see: trackSelectionTool_getter
    kwargs.setdefault("DriftCircleCutTool", None)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            TrigPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            TrigPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minHits", flags.InDet.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minNotShared",
                      flags.InDet.Tracking.ActiveConfig.minSiNotShared)
    kwargs.setdefault("maxShared", flags.InDet.Tracking.ActiveConfig.maxShared)
    kwargs.setdefault("minTRTHits", 0)  # used for Si only tracking !!!
    kwargs.setdefault("Cosmics", False)  # there is a different instance
    kwargs.setdefault("UseParameterization", False)

    acc.addPublicTool(CompFactory.InDet.InDetAmbiTrackSelectionTool(
        name, **kwargs), primary=True)
    return acc


def ITkAmbiTrackSelectionToolCfg(flags, name="ITkAmbiTrackSelectionTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DriftCircleCutTool", None)

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            ITkPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            ITkPRDtoTrackMapToolGangedPixelsCfg(flags)))

    kwargs.setdefault("minTRTHits", 0)  # used for Si only tracking !!!
    kwargs.setdefault("UseParameterization", False)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("doPixelSplitting",
                      flags.ITk.Tracking.doPixelClusterSplitting)
    kwargs.setdefault("doITk", True)

    kwargs.setdefault("sharedProbCut",
                      flags.ITk.Tracking.pixelClusterSplitProb1)
    kwargs.setdefault("sharedProbCut2",
                      flags.ITk.Tracking.pixelClusterSplitProb2)
    kwargs.setdefault("minSiHitsToAllowSplitting", 9)
    kwargs.setdefault("minUniqueSCTHits", 4)
    kwargs.setdefault("minTrackChi2ForSharedHits", 3)
    # Only allow split clusters on track withe pt greater than this MeV
    kwargs.setdefault("minPtSplit", 1000)
    # Maximum number of shared modules for tracks in ROI
    kwargs.setdefault("maxSharedModulesInROI", 3)
    # Minimum number of unique modules for tracks in ROI
    kwargs.setdefault("minNotSharedInROI", 2)
    # Minimum number of Si hits to allow splittings for tracks in ROI
    kwargs.setdefault("minSiHitsToAllowSplittingInROI", 8)
    # Split cluster ROI size
    kwargs.setdefault("phiWidth", 0.05)
    kwargs.setdefault("etaWidth", 0.05)

    # Only split in cluster in region of interest
    kwargs.setdefault("doEmCaloSeed", flags.ITk.Tracking.doCaloSeededAmbi)
    kwargs.setdefault("EMROIPhiRZContainer", "ITkCaloClusterROIPhiRZ10GeV")
    if flags.ITk.Tracking.doCaloSeededAmbi:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))

    # Do special cuts in region of interest
    kwargs.setdefault("doHadCaloSeed", flags.ITk.Tracking.doCaloSeededAmbi)
    kwargs.setdefault("HadROIPhiRZContainer", "ITkHadCaloClusterROIPhiRZBjet")
    if flags.ITk.Tracking.doCaloSeededAmbi:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkHadCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkHadCaloClusterROIPhiRZContainerMakerCfg(flags))

    # Only allow split clusters on track withe pt greater than this MeV
    kwargs.setdefault("minPtConv", 10000)
    kwargs.setdefault("minPtBjetROI", 10000)
    # Split cluster ROI size
    kwargs.setdefault("phiWidthEM", 0.05)
    kwargs.setdefault("etaWidthEM", 0.05)

    if 'InDetEtaDependentCutsSvc' not in kwargs:
        from InDetConfig.InDetEtaDependentCutsConfig import (
            ITkEtaDependentCutsSvcCfg)
        acc.merge(ITkEtaDependentCutsSvcCfg(flags))
        kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService(
            "ITkEtaDependentCutsSvc"+flags.ITk.Tracking.ActiveConfig.extension))

    acc.setPrivateTools(CompFactory.InDet.InDetDenseEnvAmbiTrackSelectionTool(
        name=name+flags.ITk.Tracking.ActiveConfig.extension, **kwargs))
    return acc
