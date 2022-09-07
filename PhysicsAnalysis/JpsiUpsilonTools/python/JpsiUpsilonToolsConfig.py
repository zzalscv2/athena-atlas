# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PrimaryVertexRefittingToolCfg(ConfigFlags, **kwargs):
    acc = ComponentAccumulator()
    from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
    kwargs.setdefault( "TrackToVertexIPEstimator", acc.popToolsAndMerge( TrackToVertexIPEstimatorCfg(ConfigFlags,**kwargs) ) )
    acc.setPrivateTools( CompFactory.Analysis.PrimaryVertexRefitter( **kwargs) )
    return acc

#A setup with nice friendly defaults
def JpsiFinderCfg(ConfigFlags,name="JpsiFinder", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("useV0Fitter", False)
    kwargs.setdefault("V0VertexFitterTool", None)
    if "TrkVertexFitterTool" not in kwargs:
        from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
        kwargs.setdefault("TrkVertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(ConfigFlags)))
    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import BPHY_InDetDetailedTrackSelectorToolCfg
        kwargs.setdefault("TrackSelectorTool", acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(ConfigFlags)))
    if "VertexPointEstimator" not in kwargs:
        from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg
        kwargs.setdefault("VertexPointEstimator", acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(ConfigFlags)))
    acc.addPublicTool(kwargs["TrkVertexFitterTool"])
    acc.addPublicTool(kwargs["TrackSelectorTool"])
    acc.addPublicTool(kwargs["VertexPointEstimator"])
    acc.setPrivateTools(CompFactory.Analysis.JpsiFinder(name, **kwargs))
    return acc

def JpsiAlgCfg(ConfigFlags, name="JpsiAlg", **kwargs):
   acc = ComponentAccumulator()
   if "JpsiFinderName" not in kwargs:
       kwargs.setdefault("JpsiFinderName", acc.popToolsAndMerge(JpsiFinderCfg(ConfigFlags)))
   acc.addEventAlgo(CompFactory.JpsiAlg(name, **kwargs))
   return acc