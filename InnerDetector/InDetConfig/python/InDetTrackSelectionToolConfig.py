# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSelectionTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def InDetTrackSelectionTool_TrackTools_Cfg(flags, name="InDetTrackSelectionTool", **kwargs):
    acc = ComponentAccumulator()

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(InDetExtrapolatorCfg(flags)))

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags)))

    kwargs.setdefault("UseTrkTrackTools", True)

    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

#############################################
#####  Configs based on CutLevel Loose  #####
#############################################

def InDetTrackSelectionTool_Loose_Cfg(flags, name="InDetTrackSelectionTool_Loose", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CutLevel", "Loose")
    kwargs.setdefault("maxAbsEta", 2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

def isoTrackSelectionToolCfg(flags, name="isoTrackSelectionTool", **kwargs):
    kwargs.setdefault('minPt', 1000)
    return InDetTrackSelectionTool_Loose_Cfg(flags, name, **kwargs)

####################################################
#####  Configs based on CutLevel LoosePrimary  #####
####################################################

def InDetTrackSelectionTool_LoosePrimary_Cfg(flags, name="InDetTrackSelectionTool_LoosePrimary", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CutLevel", "LoosePrimary")
    kwargs.setdefault("maxAbsEta", 2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

####################################################
#####  Configs based on CutLevel TightPrimary  #####
####################################################

def InDetTrackSelectionTool_TightPrimary_Cfg(flags, name="InDetTrackSelectionTool_TightPrimary", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CutLevel", "TightPrimary")
    kwargs.setdefault("maxAbsEta", 2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

def InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(flags, name="InDetTrackSelectionTool_TightPrimary", **kwargs):
    kwargs.setdefault("CutLevel", "TightPrimary")
    kwargs.setdefault("maxAbsEta", 2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    return InDetTrackSelectionTool_TrackTools_Cfg(flags, name, **kwargs)

def PFTrackSelectionToolCfg(flags, name="PFTrackSelectionTool", **kwargs):
    kwargs.setdefault('minPt', 500.0)
    return InDetTrackSelectionTool_TightPrimary_Cfg(flags, name, **kwargs)

def IDAlignMonTrackSelectionToolCfg(flags, name="IDAlignMonTrackSelectionTool", **kwargs):
    kwargs.setdefault("TrackSummaryTool", None)
    kwargs.setdefault("maxNPixelHoles"             , 1)
    kwargs.setdefault("minNBothInnermostLayersHits", 0)
    kwargs.setdefault("minNInnermostLayerHits"     , 1)
    kwargs.setdefault("minPt"                      , 5000)
    kwargs.setdefault("maxD0"                      , 100000)
    kwargs.setdefault("maxZ0SinTheta"              , 150)
    return InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(flags, name, **kwargs)

###############################################
#####  Configs not based on any CutLevel  #####
###############################################

def VtxInDetTrackSelectionCfg(flags, name="VertexInDetTrackSelectionTool", **kwargs):
    for key in (
        "maxAbsEta",
        "maxD0",
        "maxNPixelHoles",
        "maxSigmaD0",
        "maxSigmaZ0SinTheta",
        "maxZ0",
        "maxZ0SinTheta",
        "minNInnermostLayerHits",
        "minNPixelHits",
        "minNSctHits",
        "minNSiHits",
        "minNTrtHits",
        "minPt",
    ):
        kwargs.setdefault(key, getattr(flags.Tracking.PriVertex, key))

    kwargs.setdefault("UseTrkTrackTools", False)

    # Cut level = NoCut for a few modes
    if flags.Reco.EnableHI or \
       flags.Tracking.doMinBias or \
       flags.Tracking.doLowMu:
        acc = ComponentAccumulator()
        acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
        return acc

    # Default is TightPrimary
    else:
        return InDetTrackSelectionTool_TightPrimary_Cfg(flags, name, **kwargs)


def TrigVtxInDetTrackSelectionCfg(flags, name="InDetTrigDetailedTrackSelectionTool", **kwargs):
    from InDetTrigRecExample.TrigInDetConfiguredVtxCuts import ConfiguredTrigVtxCuts
    cuts = ConfiguredTrigVtxCuts()
    cuts.printInfo()

    signature = kwargs.pop("signature","")
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    config = getInDetTrigConfig(signature)

    acc = ComponentAccumulator()

    kwargs.setdefault("CutLevel", cuts.TrackCutLevel())
    kwargs.setdefault("minPt", cuts.minPT())
    kwargs.setdefault("maxD0", cuts.IPd0Max())
    kwargs.setdefault("maxZ0", cuts.z0Max())
    kwargs.setdefault("maxZ0SinTheta", cuts.IPz0Max())
    kwargs.setdefault("maxSigmaD0", cuts.sigIPd0Max())
    kwargs.setdefault("maxSigmaZ0SinTheta", cuts.sigIPz0Max())
    kwargs.setdefault("maxChiSqperNdf", cuts.fitChi2OnNdfMax())
    kwargs.setdefault("maxAbsEta", cuts.etaMax())
    kwargs.setdefault("minNInnermostLayerHits", cuts.nHitInnermostLayer())
    kwargs.setdefault("minNPixelHits", cuts.nHitPix())
    kwargs.setdefault("maxNPixelHoles", cuts.nHolesPix())
    kwargs.setdefault("minNSctHits", cuts.nHitSct())
    kwargs.setdefault("minNTrtHits", cuts.nHitTrt())
    kwargs.setdefault("minNSiHits", config.minNSiHits_vtx if config.minNSiHits_vtx is not None
                      else cuts.nHitSi())
    # N.B. Legacy config used to set extrapolator + trackSummary tools but since UseTrkTrackTools is not set to True, they're not used in the InDetTrackSelectionTool

    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name+signature, **kwargs))
    return acc

def Tau_InDetTrackSelectionToolForTJVACfg(flags, name="tauRec_InDetTrackSelectionToolForTJVA", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("minPt", 1000.)
    kwargs.setdefault("minNPixelHits", 2)
    kwargs.setdefault("minNSiHits", 7)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

def InDetGlobalLRTMonAlg_TrackSelectionToolCfg(flags, name="InDetGlobalLRTMonAlg_TrackSelectionTool", **kwargs):
    kwargs.setdefault("minPt", 1000.)
    kwargs.setdefault("maxNPixelHoles", 1)
    return InDetTrackSelectionTool_TrackTools_Cfg(flags, name, **kwargs)

def HI_InDetTrackSelectionToolForHITrackJetsCfg(flags, name="TrackSelHI", **kwargs):
    """Provides track selection tool for HI track jet reconstruction."""
    acc = ComponentAccumulator()
    kwargs.setdefault("minNSiHits", 7)
    kwargs.setdefault("maxAbsEta", 2.5)
    kwargs.setdefault("maxNSiHoles", 2)
    kwargs.setdefault("maxNPixelHoles", 1)
    kwargs.setdefault("minPt", 4000.)
    acc.setPrivateTools(CompFactory.InDet.InDetTrackSelectionTool(name, **kwargs))
    return acc

###############################################
#####  Configs for Sec Vtx  #####
###############################################


def InDetTrackSelectionTool_AMSVF_Cfg(flags, name='InDetTrackSelectionTool_AMSVF', **kwargs):

    kwargs.setdefault("CutLevel", "NoCut")
    kwargs.setdefault("minPt", 1000.)
    kwargs.setdefault("maxD0", 500.0)
    kwargs.setdefault("maxZ0", 1500.)
    kwargs.setdefault("maxSigmaD0", -1.0)
    kwargs.setdefault("maxSigmaZ0SinTheta", -1.0)
    kwargs.setdefault("maxChiSqperNdf", 5.0)
    kwargs.setdefault("maxAbsEta", 2.5)
    kwargs.setdefault("minNInnermostLayerHits", 0)
    kwargs.setdefault("minNPixelHits", 0)
    kwargs.setdefault("maxNPixelHoles", 1)
    kwargs.setdefault("minNSctHits", 2)
    kwargs.setdefault("minNTrtHits", 0)
    kwargs.setdefault("minNSiHits", 0)
    kwargs.setdefault("maxNSiSharedHits", 6)

    return InDetTrackSelectionTool_TrackTools_Cfg(flags, name, **kwargs)
