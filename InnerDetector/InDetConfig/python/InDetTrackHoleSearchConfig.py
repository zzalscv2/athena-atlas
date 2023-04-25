# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackHoleSearch package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType


def InDetTrackHoleSearchToolCfg(flags, name='InDetHoleSearchTool', **kwargs):
    if flags.Detector.GeometryITk:
        name = name.replace("InDet", "ITk")
        return ITkTrackHoleSearchToolCfg(flags, name, **kwargs)

    result = ComponentAccumulator()
    if 'Extrapolator' not in kwargs:
        # TODO: Check if AtlasExtrapolatorCfg can be used instead
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            InDetExtrapolatorCfg(flags)))

    if 'BoundaryCheckTool' not in kwargs:
        from InDetConfig.InDetBoundaryCheckToolConfig import (
            InDetBoundaryCheckToolCfg)
        kwargs.setdefault('BoundaryCheckTool', result.popToolsAndMerge(
            InDetBoundaryCheckToolCfg(flags)))

    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("CountDeadModulesAfterLastHit", True)

    result.setPrivateTools(
        CompFactory.InDet.InDetTrackHoleSearchTool(name, **kwargs))
    return result


def TrigHoleSearchToolCfg(flags, name="InDetTrigHoleSearchTool", **kwargs):
    result = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            InDetExtrapolatorCfg(flags, name="InDetTrigExtrapolator")))

    # create InDetTrigBoundaryCheckToolCfg with these settings
    if 'BoundaryCheckTool' not in kwargs:
        from InDetConfig.InDetBoundaryCheckToolConfig import (
            InDetTrigBoundaryCheckToolCfg)
        kwargs.setdefault('BoundaryCheckTool', result.popToolsAndMerge(
            InDetTrigBoundaryCheckToolCfg(flags)))

    kwargs.setdefault("CountDeadModulesAfterLastHit", True)

    result.setPrivateTools(
        CompFactory.InDet.InDetTrackHoleSearchTool(name, **kwargs))
    return result


def ITkTrackHoleSearchToolCfg(flags, name='ITkHoleSearchTool', **kwargs):
    result = ComponentAccumulator()
    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    if 'BoundaryCheckTool' not in kwargs:
        from InDetConfig.InDetBoundaryCheckToolConfig import (
            ITkBoundaryCheckToolCfg)
        kwargs.setdefault('BoundaryCheckTool', result.popToolsAndMerge(
            ITkBoundaryCheckToolCfg(flags)))

    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("CountDeadModulesAfterLastHit", True)

    result.setPrivateTools(
        CompFactory.InDet.InDetTrackHoleSearchTool(name, **kwargs))
    return result


def AtlasTrackHoleSearchToolCfg(flags, name='AtlasHoleSearchTool', **kwargs):
    result = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    result.setPrivateTools(result.popToolsAndMerge(
        InDetTrackHoleSearchToolCfg(flags, name, **kwargs)))
    return result


def CombinedMuonIDHoleSearchCfg(
        flags, name='CombinedMuonIDHoleSearch', **kwargs):
    if flags.Detector.GeometryITk:
        return ITkTrackHoleSearchToolCfg(flags, name, **kwargs)

    result = ComponentAccumulator()

    if 'Extrapolator' not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    if 'BoundaryCheckTool' not in kwargs:
        from InDetConfig.InDetBoundaryCheckToolConfig import (
            InDetBoundaryCheckToolCfg)
        from InDetConfig.InDetTestPixelLayerConfig import (
            InDetTestPixelLayerToolCfg, InDetTrigTestPixelLayerToolCfg)
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        atlasextrapolator = result.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags))
        if flags.Muon.MuonTrigger:
            from SCT_ConditionsTools.SCT_ConditionsToolsConfig import (
                SCT_ConditionsSummaryToolCfg)
            sctCondTool = result.popToolsAndMerge(SCT_ConditionsSummaryToolCfg(
                flags, withFlaggedCondTool=False,
                withByteStreamErrorsTool=False))

            BoundaryCheckTool = result.popToolsAndMerge(
                InDetBoundaryCheckToolCfg(
                    flags, name='CombinedMuonIDBoundaryCheckTool',
                    SCTDetElStatus="",
                    SctSummaryTool=sctCondTool,
                    PixelLayerTool=result.popToolsAndMerge(
                        InDetTrigTestPixelLayerToolCfg(
                            flags, name='CombinedMuonPixelLayerToolDefault',
                            Extrapolator=atlasextrapolator)
                    )
                ))
        else:
            BoundaryCheckTool = result.popToolsAndMerge(
                InDetBoundaryCheckToolCfg(
                    flags, name='CombinedMuonIDBoundaryCheckTool',
                    PixelLayerTool=result.popToolsAndMerge(
                        InDetTestPixelLayerToolCfg(
                            flags, name='CombinedMuonPixelLayerToolDefault',
                            Extrapolator=atlasextrapolator))))

        kwargs.setdefault('BoundaryCheckTool', BoundaryCheckTool)

    result.setPrivateTools(result.popToolsAndMerge(
        InDetTrackHoleSearchToolCfg(flags, name, **kwargs)))
    return result
