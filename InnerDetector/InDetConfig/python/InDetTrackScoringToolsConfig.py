# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackScoringTools package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#########################
# InDet configs
#########################

# InDetAmbiScoringTool


def InDetAmbiScoringToolBaseCfg(flags, name='InDetAmbiScoringTool', **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("SummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    if 'DriftCircleCutTool' not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        kwargs.setdefault("DriftCircleCutTool", acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags)))

    have_calo_rois = (flags.Tracking.doBremRecovery and
                      flags.Tracking.doCaloSeededBrem and
                      kwargs.get("doEmCaloSeed", True))

    if have_calo_rois:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            CaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))
        kwargs.setdefault("EMROIPhiRZContainer",
                          "InDetCaloClusterROIPhiRZ5GeV")

    kwargs.setdefault("doEmCaloSeed", have_calo_rois)
    kwargs.setdefault("useAmbigFcn", True)
    kwargs.setdefault("useTRT_AmbigFcn", False)
    kwargs.setdefault("maxZImp", flags.InDet.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxEta", flags.InDet.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.InDet.Tracking.ActiveConfig.usePixel)
    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActiveConfig.useSCT)

    acc.setPrivateTools(CompFactory.InDet.InDetAmbiScoringTool(name, **kwargs))
    return acc


def InDetAmbiScoringToolCfg(flags, name='InDetAmbiScoringTool', **kwargs):
    kwargs.setdefault("minTRTonTrk", 0)
    kwargs.setdefault("minTRTPrecisionFraction", 0)
    kwargs.setdefault("minPt", flags.InDet.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("maxRPhiImp",
                      flags.InDet.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxRPhiImpEM",
                      flags.InDet.Tracking.ActiveConfig.maxEMImpact)
    kwargs.setdefault("minSiClusters",
                      flags.InDet.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minPixel", flags.InDet.Tracking.ActiveConfig.minPixel)
    kwargs.setdefault("maxSiHoles", flags.InDet.Tracking.ActiveConfig.maxHoles)
    kwargs.setdefault("maxPixelHoles",
                      flags.InDet.Tracking.ActiveConfig.maxPixelHoles)
    kwargs.setdefault("maxSCTHoles",
                      flags.InDet.Tracking.ActiveConfig.maxSctHoles)
    kwargs.setdefault("maxDoubleHoles",
                      flags.InDet.Tracking.ActiveConfig.maxDoubleHoles)

    return InDetAmbiScoringToolBaseCfg(flags, name + flags.InDet.Tracking.ActiveConfig.extension, **kwargs)


def InDetAmbiScoringToolSiCfg(flags, name='InDetAmbiScoringToolSi', **kwargs):
    kwargs.setdefault('DriftCircleCutTool', None)
    return InDetAmbiScoringToolCfg(flags, name, **kwargs)


def InDetExtenScoringToolCfg(flags, name='InDetExtenScoringTool', **kwargs):
    kwargs.setdefault("minTRTonTrk",
                      flags.InDet.Tracking.ActiveConfig.minTRTonTrk)
    kwargs.setdefault("minTRTPrecisionFraction",
                      flags.InDet.Tracking.ActiveConfig.minTRTPrecFrac)
    return InDetAmbiScoringToolCfg(flags, name, **kwargs)


def InDetTRT_SeededScoringToolCfg(flags, name='InDetTRT_SeededScoringTool', **kwargs):
    kwargs.setdefault("useAmbigFcn", False)
    kwargs.setdefault("useTRT_AmbigFcn", True)
    kwargs.setdefault("minTRTonTrk",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryTRTonTrk)
    kwargs.setdefault("minTRTPrecisionFraction",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryTRTPrecFrac)
    kwargs.setdefault("minPt",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryPt)
    kwargs.setdefault("maxRPhiImp",
                      flags.InDet.Tracking.ActiveConfig.maxSecondaryImpact)
    kwargs.setdefault("minSiClusters",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryClusters)
    kwargs.setdefault("maxSiHoles",
                      flags.InDet.Tracking.ActiveConfig.maxSecondaryHoles)
    kwargs.setdefault("maxPixelHoles",
                      flags.InDet.Tracking.ActiveConfig.maxSecondaryPixelHoles)
    kwargs.setdefault("maxSCTHoles",
                      flags.InDet.Tracking.ActiveConfig.maxSecondarySCTHoles)
    kwargs.setdefault("maxDoubleHoles",
                      flags.InDet.Tracking.ActiveConfig.maxSecondaryDoubleHoles)

    return InDetAmbiScoringToolBaseCfg(flags, name, **kwargs)


def InDetTrigAmbiScoringToolCfg(flags, name='InDetTrigMT_AmbiguityScoringTool', **kwargs):
    acc = ComponentAccumulator()

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrigTrackSummaryToolCfg)
        kwargs.setdefault("SummaryTool", acc.popToolsAndMerge(
            InDetTrigTrackSummaryToolCfg(flags)))

    if "Extrapolator" not in kwargs:
        # TODO using offline, consider porting
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if "DriftCircleCutTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTrigTRTDriftCircleCutToolCfg)
        kwargs.setdefault("DriftCircleCutTool", acc.popToolsAndMerge(
            InDetTrigTRTDriftCircleCutToolCfg(flags)))

    kwargs.setdefault("useAmbigFcn", True)
    kwargs.setdefault("useTRT_AmbigFcn", False)
    kwargs.setdefault("maxZImp", flags.InDet.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxEta", flags.InDet.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.InDet.Tracking.ActiveConfig.usePixel)
    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActiveConfig.useSCT)
    # TODO understand and set appropriately, however current setting is probably a correct one
    kwargs.setdefault("doEmCaloSeed", False)

    acc.setPrivateTools(CompFactory.InDet.InDetAmbiScoringTool(
        name+flags.InDet.Tracking.ActiveConfig.name, **kwargs))
    return acc

# InDetCosmicScoringTool


def InDetCosmicsScoringToolCfg(flags, name='InDetCosmicsScoringTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("nWeightedClustersMin",
                      flags.InDet.Tracking.ActiveConfig.nWeightedClustersMin)
    kwargs.setdefault("minTRTHits", 0)

    if 'SummaryTool' not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        InDetTrackSummaryTool = acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags))
        acc.addPublicTool(InDetTrackSummaryTool)
        kwargs.setdefault("SummaryTool", InDetTrackSummaryTool)

    acc.setPrivateTools(CompFactory.InDet.InDetCosmicScoringTool(
        name+flags.InDet.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def InDetCosmicExtenScoringToolCfg(flags, name='InDetCosmicExtenScoringTool', **kwargs):
    kwargs.setdefault("nWeightedClustersMin", 0)
    kwargs.setdefault("minTRTHits",
                      flags.InDet.Tracking.ActiveConfig.minTRTonTrk)
    return InDetCosmicsScoringToolCfg(flags, name, **kwargs)


def InDetCosmicScoringTool_TRTCfg(flags, name='InDetCosmicScoringTool_TRT', **kwargs):
    acc = ComponentAccumulator()

    if 'SummaryTool' not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrackSummaryToolNoHoleSearchCfg)
        InDetTrackSummaryToolNoHoleSearch = acc.popToolsAndMerge(
            InDetTrackSummaryToolNoHoleSearchCfg(flags))
        acc.addPublicTool(InDetTrackSummaryToolNoHoleSearch)
        kwargs.setdefault("SummaryTool", InDetTrackSummaryToolNoHoleSearch)

    kwargs.setdefault("minTRTHits",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryTRTonTrk)

    acc.setPrivateTools(acc.popToolsAndMerge(
        InDetCosmicExtenScoringToolCfg(flags, name, **kwargs)))
    return acc

# InDetNNScoringTool


def InDetNNScoringToolBaseCfg(flags, name='InDetNNScoringTool', **kwargs):
    acc = ComponentAccumulator()

    have_calo_rois = (flags.Tracking.doBremRecovery and
                      flags.Tracking.doCaloSeededBrem)

    if have_calo_rois:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            CaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))
        kwargs.setdefault("EMROIPhiRZContainer",
                          "InDetCaloClusterROIPhiRZ5GeV")

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("SummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    if 'DriftCircleCutTool' not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        kwargs.setdefault("DriftCircleCutTool", acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags)))

    kwargs.setdefault("nnCutConfig",
                      "dev/TrackingCP/LRTAmbiNetwork/20200727_225401/nn-config.json")
    kwargs.setdefault("nnCutThreshold",
                      flags.Tracking.nnCutLargeD0Threshold)

    kwargs.setdefault("useAmbigFcn", True)
    kwargs.setdefault("useTRT_AmbigFcn", False)
    kwargs.setdefault("maxZImp", flags.InDet.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxEta", flags.InDet.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.InDet.Tracking.ActiveConfig.usePixel)
    kwargs.setdefault("useSCT", flags.InDet.Tracking.ActiveConfig.useSCT)
    kwargs.setdefault("doEmCaloSeed", have_calo_rois)

    acc.setPrivateTools(CompFactory.InDet.InDetNNScoringTool(name, **kwargs))
    return acc


def InDetNNScoringToolCfg(flags, name='InDetNNScoringTool', **kwargs):
    kwargs.setdefault("minTRTonTrk", 0)
    kwargs.setdefault("minTRTPrecisionFraction", 0)
    kwargs.setdefault("minPt", flags.InDet.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("maxRPhiImp",
                      flags.InDet.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxRPhiImpEM",
                      flags.InDet.Tracking.ActiveConfig.maxEMImpact)
    kwargs.setdefault("minSiClusters",
                      flags.InDet.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minPixel",
                      flags.InDet.Tracking.ActiveConfig.minPixel)
    kwargs.setdefault("maxSiHoles",
                      flags.InDet.Tracking.ActiveConfig.maxHoles)
    kwargs.setdefault("maxPixelHoles",
                      flags.InDet.Tracking.ActiveConfig.maxPixelHoles)
    kwargs.setdefault("maxSCTHoles",
                      flags.InDet.Tracking.ActiveConfig.maxSctHoles)
    kwargs.setdefault("maxDoubleHoles",
                      flags.InDet.Tracking.ActiveConfig.maxDoubleHoles)

    return InDetNNScoringToolBaseCfg(flags, name+flags.InDet.Tracking.ActiveConfig.extension, **kwargs)


def InDetNNScoringToolSiCfg(flags, name='InDetNNScoringToolSi', **kwargs):
    kwargs.setdefault('DriftCircleCutTool', None)
    return InDetNNScoringToolCfg(flags, name, **kwargs)

# InDetTrtTrackScoringTool


def InDetTRT_StandaloneScoringToolCfg(flags, name='InDetTRT_StandaloneScoringTool', **kwargs):
    acc = ComponentAccumulator()

    #
    # --- set up special Scoring Tool for standalone TRT tracks
    #
    if "DriftCircleCutTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        InDetTRTDriftCircleCut = acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags))
        acc.addPublicTool(InDetTRTDriftCircleCut)
        kwargs.setdefault("DriftCircleCutTool", InDetTRTDriftCircleCut)

    kwargs.setdefault("useAmbigFcn", True)
    kwargs.setdefault("useSigmaChi2", False)
    kwargs.setdefault("PtMin", flags.InDet.Tracking.ActiveConfig.minTRTonlyPt)
    kwargs.setdefault("minTRTonTrk",
                      flags.InDet.Tracking.ActiveConfig.minTRTonly)
    kwargs.setdefault("maxEta", 2.1)
    kwargs.setdefault("UseParameterization",
                      flags.InDet.Tracking.ActiveConfig.useTRTonlyParamCuts)
    kwargs.setdefault("OldTransitionLogic",
                      flags.InDet.Tracking.ActiveConfig.useTRTonlyOldLogic)
    kwargs.setdefault("minTRTPrecisionFraction",
                      flags.InDet.Tracking.ActiveConfig.minSecondaryTRTPrecFrac)
    kwargs.setdefault("TRTTrksEtaBins",
                      flags.InDet.Tracking.ActiveConfig.TrkSel.TRTTrksEtaBins)
    kwargs.setdefault("TRTTrksMinTRTHitsThresholds",
                      flags.InDet.Tracking.ActiveConfig.TrkSel.TRTTrksMinTRTHitsThresholds)
    kwargs.setdefault("TRTTrksMinTRTHitsMuDependencies",
                      flags.InDet.Tracking.ActiveConfig.TrkSel.TRTTrksMinTRTHitsMuDependencies)

    acc.setPrivateTools(
        CompFactory.InDet.InDetTrtTrackScoringTool(name, **kwargs))
    return acc


def InDetTRT_TrackSegmentScoringToolCfg(flags, name='InDetTRT_TrackSegmentScoringTool', **kwargs):
    kwargs.setdefault("PtMin", flags.InDet.Tracking.ActiveConfig.minPT)
    return InDetTRT_StandaloneScoringToolCfg(flags, name, **kwargs)

#########################
# ITk configs
#########################

# InDetAmbiScoringTool


def ITkAmbiScoringToolCfg(flags, name='ITkAmbiScoringTool', **kwargs):
    acc = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    if 'SummaryTool' not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
        kwargs.setdefault("SummaryTool", acc.popToolsAndMerge(
            ITkTrackSummaryToolCfg(flags)))

    if 'InDetEtaDependentCutsSvc' not in kwargs:
        from InDetConfig.InDetEtaDependentCutsConfig import (
            ITkEtaDependentCutsSvcCfg)
        acc.merge(ITkEtaDependentCutsSvcCfg(flags))
        kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService(
            "ITkEtaDependentCutsSvc"+flags.ITk.Tracking.ActiveConfig.extension))

    have_calo_rois = (flags.Tracking.doBremRecovery and
                      flags.Tracking.doCaloSeededBrem)
    if have_calo_rois:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))
        kwargs.setdefault("EMROIPhiRZContainer", "ITkCaloClusterROIPhiRZ5GeV")

    kwargs.setdefault("DriftCircleCutTool", None)
    kwargs.setdefault("useAmbigFcn", True)
    kwargs.setdefault("useTRT_AmbigFcn", False)
    kwargs.setdefault("maxEta", flags.ITk.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.ITk.Tracking.ActiveConfig.useITkPixel)
    kwargs.setdefault("useSCT", flags.ITk.Tracking.ActiveConfig.useITkStrip)
    kwargs.setdefault("doEmCaloSeed", have_calo_rois)
    kwargs.setdefault("useITkAmbigFcn", True)
    kwargs.setdefault("minTRTonTrk", 0)
    kwargs.setdefault("minTRTPrecisionFraction", 0)

    acc.setPrivateTools(CompFactory.InDet.InDetAmbiScoringTool(
        name + flags.ITk.Tracking.ActiveConfig.extension, **kwargs))
    return acc

# InDetCosmicScoringTool


def ITkCosmicsScoringToolCfg(flags, name='ITkCosmicsScoringTool', **kwargs):
    acc = ComponentAccumulator()

    if 'SummaryTool' not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
        ITkTrackSummaryTool = acc.popToolsAndMerge(
            ITkTrackSummaryToolCfg(flags))
        acc.addPublicTool(ITkTrackSummaryTool)
        kwargs.setdefault("SummaryTool", ITkTrackSummaryTool)

    kwargs.setdefault("nWeightedClustersMin",
                      flags.ITk.Tracking.ActiveConfig.nWeightedClustersMin)
    kwargs.setdefault("minTRTHits", 0)

    acc.setPrivateTools(CompFactory.InDet.InDetCosmicScoringTool(
        name+flags.ITk.Tracking.ActiveConfig.extension, **kwargs))
    return acc
