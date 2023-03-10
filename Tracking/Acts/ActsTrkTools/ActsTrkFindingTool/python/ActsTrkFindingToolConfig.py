#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsGeometry.ActsGeometryConfig import (
    ActsExtrapolationToolCfg,
    ActsTrackingGeometryToolCfg,
)
from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
from ActsTrkEventCnv.ActsTrkEventCnvConfig import ActsToTrkConverterToolCfg


def ActsTrkFindingToolCfg(flags, **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("etaBins", [])
    kwargs.setdefault("chi2CutOff", [15.0])
    kwargs.setdefault("numMeasurementsCutOff", [10])
    kwargs.setdefault("doPrintTrackStates", flags.Acts.doPrintTrackStates)

    kwargs.setdefault(
        "TrackingGeometryTool",
        acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
    ) # PrivateToolHandle
    kwargs.setdefault(
        "ExtrapolationTool",
        acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
    ) # PrivateToolHandle

    kwargs.setdefault(
        "SummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags))
    ) # PrivateToolHandle

    kwargs.setdefault(
        "ATLASConverterTool",
        acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
    )

    if flags.Detector.GeometryITk:
        from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg

        BoundaryCheckToolCfg = ITkBoundaryCheckToolCfg
    else:
        from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg

        BoundaryCheckToolCfg = InDetBoundaryCheckToolCfg

    kwargs.setdefault(
        "BoundaryCheckTool",
        acc.popToolsAndMerge(BoundaryCheckToolCfg(flags)),
    )

    if flags.Acts.doRotCorrection:
        if flags.Detector.GeometryITk:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg

            RotCreatorCfg = ITkRotCreatorCfg
        else:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorCfg

            RotCreatorCfg = InDetRotCreatorCfg

        kwargs.setdefault(
            "RotCreatorTool",
            acc.popToolsAndMerge(RotCreatorCfg(flags, name="ActsRotCreatorTool")),
        )

    acc.setPrivateTools(
        CompFactory.ActsTrk.TrackFindingTool(name="ActsTrackFindingTool", **kwargs)
    )
    return acc
