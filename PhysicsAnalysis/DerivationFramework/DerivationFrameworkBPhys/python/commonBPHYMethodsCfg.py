# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName, **kwargs):
    acc = ComponentAccumulator()
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", acc.popToolsAndMerge(InDetExtrapolatorCfg(flags)))
    if "FirstMeasuredPoint" not in kwargs:
        kwargs.setdefault("FirstMeasuredPoint", False)
    if "MakeExtendedVertex" not in kwargs:
        kwargs.setdefault("MakeExtendedVertex", True)
    tool = CompFactory.Trk.TrkVKalVrtFitter( name       = BPHYDerivationName+"_VKalVrtFitter",
                                                 **kwargs)
    acc.setPrivateTools(tool)
    return acc



def BPHY_V0ToolCfg(flags, BPHYDerivationName):
    from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsNoExtrapCfg
    return V0ToolsNoExtrapCfg(flags, BPHYDerivationName+"_V0Tools")

def BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName):
    acc = ComponentAccumulator()
    tool = CompFactory.InDet.VertexPointEstimator(name = BPHYDerivationName+"_VtxPointEstimator",
                                        MinDeltaR              = [-10000.,-10000.,-10000.],
                                        MaxDeltaR              = [10000.,10000.,10000.],
                                        MaxPhi                 = [10000., 10000., 10000.],
                                        MaxChi2OfVtxEstimation = 2000.) #NOTE MaxChi2OfVtxEstimation differs from tracking default
    acc.setPrivateTools(tool)
    return acc

def BPHY_InDetDetailedTrackSelectorToolCfg(flags,BPHYDerivationName, **kwargs):
    acc = ComponentAccumulator()
    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    extrap = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags))
    InDetTrackSelectorTool = CompFactory.InDet.InDetDetailedTrackSelectorTool(name = BPHYDerivationName+"_InDetDetailedTrackSelectorTool",
                                pTMin                = 400.0,
                                IPd0Max              = 10000.0,
                                IPz0Max              = 10000.0,
                                z0Max                = 10000.0,
                                sigIPd0Max           = 10000.0,
                                sigIPz0Max           = 10000.0,
                                d0significanceMax    = -1.,
                                z0significanceMax    = -1.,
                                etaMax               = 9999.,
                                useTrackSummaryInfo  = True,
                                nHitBLayer           = 0,
                                nHitPix              = 1,
                                nHitBLayerPlusPix    = 1,
                                nHitSct              = 2,
                                nHitSi               = 3,
                                nHitTrt              = 0,
                                nHitTrtHighEFractionMax = 10000.0,
                                useSharedHitInfo     = False,
                                useTrackQualityInfo  = True,
                                fitChi2OnNdfMax      = 10000.0,
                                TrtMaxEtaAcceptance  = 1.9,
                                UseEventInfoBS       = True,
                                TrackSummaryTool     = None,
                                Extrapolator         = extrap
                               )
    acc.setPrivateTools(InDetTrackSelectorTool)
    return acc

