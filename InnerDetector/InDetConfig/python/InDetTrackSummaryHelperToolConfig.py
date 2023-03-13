# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSummaryHelperTool package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetTrackSummaryHelperToolCfg(flags, name='InDetSummaryHelper', **kwargs):
  if flags.Detector.GeometryITk:
    name = name.replace("InDet", "ITk")
    return ITkTrackSummaryHelperToolCfg(flags, name, **kwargs)

  result = ComponentAccumulator()

  if "HoleSearch" not in kwargs:
    from InDetConfig.InDetTrackHoleSearchConfig import InDetTrackHoleSearchToolCfg
    InDetTrackHoleSearchTool = result.popToolsAndMerge(InDetTrackHoleSearchToolCfg(flags))
    result.addPublicTool(InDetTrackHoleSearchTool)
    kwargs.setdefault("HoleSearch", InDetTrackHoleSearchTool)

  if not flags.Detector.EnableTRT:
    kwargs.setdefault("TRTStrawSummarySvc", "")

  kwargs.setdefault("usePixel", flags.Detector.EnablePixel)
  kwargs.setdefault("useSCT", flags.Detector.EnableSCT)
  kwargs.setdefault("useTRT", flags.Detector.EnableTRT)

  result.setPrivateTools(CompFactory.InDet.InDetTrackSummaryHelperTool(name, **kwargs))
  return result

def InDetSummaryHelperNoHoleSearchCfg(flags, name='InDetSummaryHelperNoHoleSearch', **kwargs):
  kwargs.setdefault("HoleSearch", None)
  return InDetTrackSummaryHelperToolCfg(flags, name, **kwargs)

def TrigTrackSummaryHelperToolCfg(flags, name="InDetTrigSummaryHelper", **kwargs):

  result = ComponentAccumulator()

  kwargs.setdefault("useTRT", flags.Detector.EnableTRT)

  #can always set HoleSearchTool - the actual search is controlled by TrackSummaryTool cfg
  if "HoleSearch" not in kwargs:
    from InDetConfig.InDetTrackHoleSearchConfig import TrigHoleSearchToolCfg
    holeSearchTool = result.popToolsAndMerge( TrigHoleSearchToolCfg(flags))
    kwargs.setdefault("HoleSearch", holeSearchTool)

  # Kept for consistency with previous config but unclear if different from default TRT_StrawStatusSummaryTool loaded in C++
  if "TRTStrawSummarySvc" not in kwargs:
    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
    TRT_StrawStatusSummaryTool = result.popToolsAndMerge( TRT_StrawStatusSummaryToolCfg(flags) )
    kwargs.setdefault("TRTStrawSummarySvc", TRT_StrawStatusSummaryTool)
      
  kwargs.setdefault("usePixel", flags.Detector.EnablePixel)
  kwargs.setdefault("useSCT", flags.Detector.EnableSCT)

  result.setPrivateTools(CompFactory.InDet.InDetTrackSummaryHelperTool(name=name, **kwargs))
  return result

def TrigTrackSummaryHelperToolSharedHitsCfg(flags, name="InDetTrigSummaryHelperSharedHits", **kwargs):
  return TrigTrackSummaryHelperToolCfg(flags, 
                                       name,
                                       DoSharedHits = True,
                                       **kwargs)

def TrigTrackSummaryHelperToolSiOnlyCfg(flags, name="InDetTrigSummaryHelperSiOnly", **kwargs):
  return TrigTrackSummaryHelperToolCfg(flags, 
                                       name,
                                       useTRT = False,
                                       TRTStrawSummarySvc = None,
                                       **kwargs)
                                       

def ITkTrackSummaryHelperToolCfg(flags, name='ITkSummaryHelper', **kwargs):
  result = ComponentAccumulator()

  if "HoleSearch" not in kwargs:
    from InDetConfig.InDetTrackHoleSearchConfig import ITkTrackHoleSearchToolCfg
    ITkTrackHoleSearchTool = result.popToolsAndMerge(ITkTrackHoleSearchToolCfg(flags))
    result.addPublicTool(ITkTrackHoleSearchTool)
    kwargs.setdefault("HoleSearch", ITkTrackHoleSearchTool)

  kwargs.setdefault("TRTStrawSummarySvc", "")
  kwargs.setdefault("usePixel", flags.Detector.EnableITkPixel)
  kwargs.setdefault("useSCT", flags.Detector.EnableITkStrip)
  kwargs.setdefault("useTRT", False)

  result.setPrivateTools(CompFactory.InDet.InDetTrackSummaryHelperTool(name, **kwargs))
  return result

def ITkSummaryHelperNoHoleSearchCfg(flags, name='ITkSummaryHelperNoHoleSearch', **kwargs):
  kwargs.setdefault("HoleSearch", None)
  return ITkTrackSummaryHelperToolCfg(flags, name, **kwargs)

def AtlasTrackSummaryHelperToolCfg(flags, name='AtlasTrackSummaryHelperTool', **kwargs):
  result = ComponentAccumulator()

  if "HoleSearch" not in kwargs:
    from InDetConfig.InDetTrackHoleSearchConfig import AtlasTrackHoleSearchToolCfg
    atlasHoleSearchTool = result.popToolsAndMerge(AtlasTrackHoleSearchToolCfg(flags))
    result.addPublicTool(atlasHoleSearchTool)
    kwargs.setdefault("HoleSearch", atlasHoleSearchTool)

  result.setPrivateTools(result.popToolsAndMerge(InDetTrackSummaryHelperToolCfg(flags, name, **kwargs)))
  return result

  
