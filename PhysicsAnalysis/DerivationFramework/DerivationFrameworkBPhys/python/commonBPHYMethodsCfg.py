# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName, **kwargs):
    from TrkConfig.TrkVKalVrtFitterConfig import BPHY_TrkVKalVrtFitterCfg \
        as TrkVKalVrtFitterCfg
    return TrkVKalVrtFitterCfg(flags, name=BPHYDerivationName+"_VKalVrtFitter", **kwargs)

def BPHY_V0ToolCfg(flags, BPHYDerivationName):
    from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
    return V0ToolsCfg(flags, BPHYDerivationName+"_V0Tools")

def BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName):
    from InDetConfig.InDetConversionFinderToolsConfig import \
        BPHY_VertexPointEstimatorCfg as VertexPointEstimatorCfg
    return VertexPointEstimatorCfg(flags, BPHYDerivationName+"_VtxPointEstimator")

def BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName):
    from InDetConfig.InDetTrackSelectorToolConfig import \
        BPHY_InDetDetailedTrackSelectorToolCfg as \
        InDetDetailedTrackSelectorToolCfg
    return InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName+"_InDetDetailedTrackSelectorTool")

def getDefaultAllVariables():
    return ["EventInfo"]
