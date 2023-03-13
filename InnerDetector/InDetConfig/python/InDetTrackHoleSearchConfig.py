# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackHoleSearch package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

def InDetTrackHoleSearchToolCfg(flags, name = 'InDetHoleSearchTool', **kwargs):
  if flags.Detector.GeometryITk:
    name = name.replace("InDet", "ITk")
    return ITkTrackHoleSearchToolCfg(flags, name, **kwargs)

  result = ComponentAccumulator()
  if 'Extrapolator' not in kwargs:
    #TODO: Check if AtlasExtrapolatorCfg can be used instead
    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    extrapolator = result.popToolsAndMerge(InDetExtrapolatorCfg(flags))
    kwargs.setdefault("Extrapolator", extrapolator)

  if 'BoundaryCheckTool' not in kwargs:
    from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
    BoundaryCheckTool = result.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags))
    kwargs.setdefault('BoundaryCheckTool', BoundaryCheckTool)

  kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
  kwargs.setdefault("CountDeadModulesAfterLastHit", True)

  indet_hole_search_tool = CompFactory.InDet.InDetTrackHoleSearchTool(name, **kwargs)
  result.setPrivateTools(indet_hole_search_tool)
  return result

def TrigHoleSearchToolCfg(flags, name="InDetTrigHoleSearchTool", **kwargs):
  result = ComponentAccumulator()

  if 'Extrapolator' not in kwargs:
    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    extrapolator = result.popToolsAndMerge(InDetExtrapolatorCfg(flags, name="InDetTrigExtrapolator"))  
    kwargs.setdefault("Extrapolator", extrapolator)

  from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
  sctCondTool = result.popToolsAndMerge(SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False, withByteStreamErrorsTool=False))

  from InDetConfig.InDetTestPixelLayerConfig import InDetTrigTestPixelLayerToolCfg
  pixelLayerTool = result.popToolsAndMerge(InDetTrigTestPixelLayerToolCfg(flags))
  
  #create InDetTrigBoundaryCheckToolCfg with these settings
  if 'BoundaryCheckTool' not in kwargs:
    from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
    BoundaryCheckTool = result.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags,
                                                                          "InDetTrigBoundaryCheckTool",
                                                                          SCTDetElStatus="",
                                                                          SctSummaryTool=sctCondTool,
                                                                          PixelLayerTool=pixelLayerTool,
                                                                          ))
    kwargs.setdefault('BoundaryCheckTool', BoundaryCheckTool)

  kwargs.setdefault("CountDeadModulesAfterLastHit", True)

  indet_hole_search_tool = CompFactory.InDet.InDetTrackHoleSearchTool(name, **kwargs)
  result.setPrivateTools(indet_hole_search_tool)
  return result

def ITkTrackHoleSearchToolCfg(flags, name='ITkHoleSearchTool', **kwargs):
  result = ComponentAccumulator()
  if 'Extrapolator' not in kwargs:
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    result.addPublicTool(extrapolator)
    kwargs.setdefault("Extrapolator", extrapolator)

  if 'BoundaryCheckTool' not in kwargs:
    from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg
    kwargs.setdefault('BoundaryCheckTool', result.popToolsAndMerge(ITkBoundaryCheckToolCfg(flags)))

  kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
  kwargs.setdefault("CountDeadModulesAfterLastHit", True)

  result.setPrivateTools(CompFactory.InDet.InDetTrackHoleSearchTool(name, **kwargs))
  return result

def AtlasTrackHoleSearchToolCfg(flags, name = 'AtlasHoleSearchTool', **kwargs):
  result = ComponentAccumulator()

  if 'Extrapolator' not in kwargs:
      from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
      extrapolatorTool = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
      result.addPublicTool(extrapolatorTool)
      kwargs.setdefault("Extrapolator", extrapolatorTool)

  result.setPrivateTools(result.popToolsAndMerge(InDetTrackHoleSearchToolCfg(flags, name, **kwargs)))
  return result

def CombinedMuonIDHoleSearchCfg(flags, name = 'CombinedMuonIDHoleSearch', **kwargs):
  if flags.Detector.GeometryITk:
    return ITkTrackHoleSearchToolCfg(flags, name, **kwargs)

  result = ComponentAccumulator()

  if 'Extrapolator' not in kwargs:
      from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
      extrapolatorTool = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
      result.addPublicTool(extrapolatorTool)
      kwargs.setdefault("Extrapolator", extrapolatorTool)
  
  if 'BoundaryCheckTool' not in kwargs:
    from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
    from InDetConfig.InDetTestPixelLayerConfig import InDetTestPixelLayerToolCfg, InDetTrigTestPixelLayerToolCfg
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    atlasextrapolator = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    result.addPublicTool(atlasextrapolator)  # TODO: migrate to private?
    if flags.Muon.MuonTrigger:
      from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
      sctCondTool = result.popToolsAndMerge(SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False, withByteStreamErrorsTool=False))

      BoundaryCheckTool = result.popToolsAndMerge(
        InDetBoundaryCheckToolCfg(flags, name='CombinedMuonIDBoundaryCheckTool', 
                                  SCTDetElStatus="",
                                  SctSummaryTool=sctCondTool,
                                  PixelLayerTool=result.popToolsAndMerge(
                                    InDetTrigTestPixelLayerToolCfg(flags, name='CombinedMuonPixelLayerToolDefault', Extrapolator=atlasextrapolator))))
    else:
      BoundaryCheckTool = result.popToolsAndMerge(
        InDetBoundaryCheckToolCfg(flags, name='CombinedMuonIDBoundaryCheckTool', 
                                  PixelLayerTool=result.popToolsAndMerge(
                                    InDetTestPixelLayerToolCfg(flags, name='CombinedMuonPixelLayerToolDefault', Extrapolator=atlasextrapolator))))

    kwargs.setdefault('BoundaryCheckTool', BoundaryCheckTool)
  result.setPrivateTools(result.popToolsAndMerge(InDetTrackHoleSearchToolCfg(flags, name, **kwargs)))
  return result

