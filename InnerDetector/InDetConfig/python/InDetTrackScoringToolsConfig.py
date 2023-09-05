# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackScoringTools package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#########################
# InDet configs
#########################

def InDetAmbiScoringToolBaseCfg(flags, name='InDetAmbiScoringTool', **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if 'DriftCircleCutTool' not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import (
            InDetTRTDriftCircleCutToolCfg)
        kwargs.setdefault("DriftCircleCutTool", acc.popToolsAndMerge(
            InDetTRTDriftCircleCutToolCfg(flags)))

    have_calo_rois = (flags.Tracking.doCaloSeededBrem and
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
    kwargs.setdefault("maxZImp", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxEta", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.usePixel)
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useSCT)

    acc.setPrivateTools(CompFactory.InDet.InDetAmbiScoringTool(name, **kwargs))
    return acc


def InDetAmbiScoringToolCfg(flags, name='InDetAmbiScoringTool', **kwargs):
    kwargs.setdefault("minTRTonTrk", 0)
    kwargs.setdefault("minTRTPrecisionFraction", 0)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("maxRPhiImp",
                      flags.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxRPhiImpEM", flags.Tracking.ActiveConfig.maxEMImpact)
    kwargs.setdefault("minSiClusters", flags.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minPixel", flags.Tracking.ActiveConfig.minPixel)
    kwargs.setdefault("maxSiHoles", flags.Tracking.ActiveConfig.maxHoles)
    kwargs.setdefault("maxPixelHoles",
                      flags.Tracking.ActiveConfig.maxPixelHoles)
    kwargs.setdefault("maxSCTHoles", flags.Tracking.ActiveConfig.maxSctHoles)
    kwargs.setdefault("maxDoubleHoles",
                      flags.Tracking.ActiveConfig.maxDoubleHoles)

    return InDetAmbiScoringToolBaseCfg(
        flags, name + flags.Tracking.ActiveConfig.extension, **kwargs)


def InDetAmbiScoringToolSiCfg(flags, name='InDetAmbiScoringToolSi', **kwargs):
    kwargs.setdefault('DriftCircleCutTool', None)
    return InDetAmbiScoringToolCfg(flags, name, **kwargs)


def InDetExtenScoringToolCfg(flags, name='InDetExtenScoringTool', **kwargs):
    kwargs.setdefault("minTRTonTrk",
                      flags.Tracking.ActiveConfig.minTRTonTrk)
    kwargs.setdefault("minTRTPrecisionFraction",
                      flags.Tracking.ActiveConfig.minTRTPrecFrac)
    return InDetAmbiScoringToolCfg(flags, name, **kwargs)


def InDetTRT_SeededScoringToolCfg(
        flags, name='InDetTRT_SeededScoringTool', **kwargs):
    kwargs.setdefault("useAmbigFcn", False)
    kwargs.setdefault("useTRT_AmbigFcn", True)
    kwargs.setdefault("minTRTonTrk",
                      flags.Tracking.ActiveConfig.minSecondaryTRTonTrk)
    kwargs.setdefault("minTRTPrecisionFraction",
                      flags.Tracking.ActiveConfig.minSecondaryTRTPrecFrac)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minSecondaryPt)
    kwargs.setdefault("maxRPhiImp",
                      flags.Tracking.ActiveConfig.maxSecondaryImpact)
    kwargs.setdefault("minSiClusters",
                      flags.Tracking.ActiveConfig.minSecondaryClusters)
    kwargs.setdefault("maxSiHoles",
                      flags.Tracking.ActiveConfig.maxSecondaryHoles)
    kwargs.setdefault("maxPixelHoles",
                      flags.Tracking.ActiveConfig.maxSecondaryPixelHoles)
    kwargs.setdefault("maxSCTHoles",
                      flags.Tracking.ActiveConfig.maxSecondarySCTHoles)
    kwargs.setdefault("maxDoubleHoles",
                      flags.Tracking.ActiveConfig.maxSecondaryDoubleHoles)

    return InDetAmbiScoringToolBaseCfg(flags, name, **kwargs)


def InDetTrigAmbiScoringToolCfg(
        flags, 
        name='TrigAmbiguityScoringTool', **kwargs):

    acc = ComponentAccumulator()

    if flags.Tracking.ActiveConfig.input_name == "cosmics":
        ambiScoringFactory = CompFactory.InDet.InDetCosmicScoringTool
        kwargs.setdefault("nWeightedClustersMin",
                          flags.Tracking.ActiveConfig.nWeightedClustersMin)
        kwargs.setdefault("minTRTHits", 0)
        
    else:
        ambiScoringFactory = CompFactory.InDet.InDetAmbiScoringTool

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
      
        kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPT)
        kwargs.setdefault("useAmbigFcn", True)
        kwargs.setdefault("useTRT_AmbigFcn", False)
        kwargs.setdefault("maxZImp", flags.Tracking.ActiveConfig.maxZImpact)
        kwargs.setdefault("maxRPhiImp", flags.Tracking.ActiveConfig.maxRPhiImpact)
        kwargs.setdefault("maxEta", flags.Tracking.ActiveConfig.maxEta)
        kwargs.setdefault("maxSCTHoles", flags.Tracking.ActiveConfig.maxSCTHoles)
        kwargs.setdefault("maxSiHoles", flags.Tracking.ActiveConfig.maxSiHoles)
        kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.usePixel)
        kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useSCT)
        kwargs.setdefault("doEmCaloSeed", False)
        kwargs.setdefault("EMROIPhiRZContainer", "")
        kwargs.setdefault("minTRTonTrk", 0)
        kwargs.setdefault("minTRTPrecisionFraction", 0)

        
    acc.setPrivateTools(ambiScoringFactory(
        name=name+flags.Tracking.ActiveConfig.input_name, 
        **kwargs))
    
    return acc


def InDetCosmicsScoringToolCfg(flags, name='InDetCosmicsScoringTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("nWeightedClustersMin",
                      flags.Tracking.ActiveConfig.nWeightedClustersMin)
    kwargs.setdefault("minTRTHits", 0)

    acc.setPrivateTools(CompFactory.InDet.InDetCosmicScoringTool(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def InDetCosmicExtenScoringToolCfg(
        flags, name='InDetCosmicExtenScoringTool', **kwargs):
    kwargs.setdefault("nWeightedClustersMin", 0)
    kwargs.setdefault("minTRTHits", flags.Tracking.ActiveConfig.minTRTonTrk)
    return InDetCosmicsScoringToolCfg(flags, name, **kwargs)


def InDetCosmicScoringTool_TRTCfg(
        flags, name='InDetCosmicScoringTool_TRT', **kwargs):
    kwargs.setdefault("minTRTHits",
                      flags.Tracking.ActiveConfig.minSecondaryTRTonTrk)
    return InDetCosmicExtenScoringToolCfg(flags, name, **kwargs)


def InDetNNScoringToolBaseCfg(flags, name='InDetNNScoringTool', **kwargs):
    acc = ComponentAccumulator()

    if flags.Tracking.doCaloSeededBrem:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            CaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(CaloClusterROIPhiRZContainerMakerCfg(flags))
        kwargs.setdefault("EMROIPhiRZContainer",
                          "InDetCaloClusterROIPhiRZ5GeV")

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

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
    kwargs.setdefault("maxZImp", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxEta", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.usePixel)
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useSCT)
    kwargs.setdefault("doEmCaloSeed", flags.Tracking.doCaloSeededBrem)

    acc.setPrivateTools(CompFactory.InDet.InDetNNScoringTool(name, **kwargs))
    return acc


def InDetNNScoringToolCfg(flags, name='InDetNNScoringTool', **kwargs):
    kwargs.setdefault("minTRTonTrk", 0)
    kwargs.setdefault("minTRTPrecisionFraction", 0)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("maxRPhiImp",
                      flags.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxRPhiImpEM", flags.Tracking.ActiveConfig.maxEMImpact)
    kwargs.setdefault("minSiClusters", flags.Tracking.ActiveConfig.minClusters)
    kwargs.setdefault("minPixel", flags.Tracking.ActiveConfig.minPixel)
    kwargs.setdefault("maxSiHoles", flags.Tracking.ActiveConfig.maxHoles)
    kwargs.setdefault("maxPixelHoles",
                      flags.Tracking.ActiveConfig.maxPixelHoles)
    kwargs.setdefault("maxSCTHoles", flags.Tracking.ActiveConfig.maxSctHoles)
    kwargs.setdefault("maxDoubleHoles",
                      flags.Tracking.ActiveConfig.maxDoubleHoles)

    return InDetNNScoringToolBaseCfg(
        flags, name+flags.Tracking.ActiveConfig.extension, **kwargs)


def InDetNNScoringToolSiCfg(flags, name='InDetNNScoringToolSi', **kwargs):
    kwargs.setdefault('DriftCircleCutTool', None)
    return InDetNNScoringToolCfg(flags, name, **kwargs)


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
    kwargs.setdefault("PtMin", flags.Tracking.ActiveConfig.minTRTonlyPt)
    kwargs.setdefault("minTRTonTrk", flags.Tracking.ActiveConfig.minTRTonly)
    kwargs.setdefault("maxEta", 2.1)
    kwargs.setdefault("UseParameterization",
                      flags.Tracking.ActiveConfig.useTRTonlyParamCuts)
    kwargs.setdefault("OldTransitionLogic",
                      flags.Tracking.ActiveConfig.useTRTonlyOldLogic)
    kwargs.setdefault("minTRTPrecisionFraction",
                      flags.Tracking.ActiveConfig.minSecondaryTRTPrecFrac)
    kwargs.setdefault("TRTTrksEtaBins",
                      flags.Tracking.ActiveConfig.TrkSel.TRTTrksEtaBins)
    kwargs.setdefault("TRTTrksMinTRTHitsThresholds",
                      flags.Tracking.ActiveConfig.TrkSel.TRTTrksMinTRTHitsThresholds)
    kwargs.setdefault("TRTTrksMinTRTHitsMuDependencies",
                      flags.Tracking.ActiveConfig.TrkSel.TRTTrksMinTRTHitsMuDependencies)

    acc.setPrivateTools(
        CompFactory.InDet.InDetTrtTrackScoringTool(name, **kwargs))
    return acc


def InDetTRT_TrackSegmentScoringToolCfg(flags, name='InDetTRT_TrackSegmentScoringTool', **kwargs):
    kwargs.setdefault("PtMin", flags.Tracking.ActiveConfig.minPT)
    return InDetTRT_StandaloneScoringToolCfg(flags, name, **kwargs)

#########################
# ITk configs
#########################


def ITkAmbiScoringToolCfg(flags, name='ITkAmbiScoringTool', **kwargs):
    acc = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    if 'InDetEtaDependentCutsSvc' not in kwargs:
        from InDetConfig.InDetEtaDependentCutsConfig import (
            ITkEtaDependentCutsSvcCfg)
        acc.merge(ITkEtaDependentCutsSvcCfg(flags))
        kwargs.setdefault("InDetEtaDependentCutsSvc", acc.getService(
            "ITkEtaDependentCutsSvc"+flags.Tracking.ActiveConfig.extension))

    if flags.Tracking.doCaloSeededBrem:
        from InDetConfig.InDetCaloClusterROISelectorConfig import (
            ITkCaloClusterROIPhiRZContainerMakerCfg)
        acc.merge(ITkCaloClusterROIPhiRZContainerMakerCfg(flags))
        kwargs.setdefault("EMROIPhiRZContainer", "ITkCaloClusterROIPhiRZ5GeV")

    kwargs.setdefault("DriftCircleCutTool", None)
    kwargs.setdefault("useAmbigFcn", True)
    kwargs.setdefault("useTRT_AmbigFcn", False)
    kwargs.setdefault("maxEta", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.useITkPixel)
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useITkStrip)
    kwargs.setdefault("doEmCaloSeed", flags.Tracking.doCaloSeededBrem)
    kwargs.setdefault("useITkAmbigFcn", True)
    kwargs.setdefault("minTRTonTrk", 0)
    kwargs.setdefault("minTRTPrecisionFraction", 0)

    acc.setPrivateTools(CompFactory.InDet.InDetAmbiScoringTool(
        name + flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def ITkCosmicsScoringToolCfg(flags, name='ITkCosmicsScoringTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("nWeightedClustersMin",
                      flags.Tracking.ActiveConfig.nWeightedClustersMin)
    kwargs.setdefault("minTRTHits", 0)

    acc.setPrivateTools(CompFactory.InDet.InDetCosmicScoringTool(
        name+flags.ITk.Tracking.ActiveConfig.extension, **kwargs))
    return acc
