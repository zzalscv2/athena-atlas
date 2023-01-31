#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsGeometry.ActsGeometryConfig import (
    ActsExtrapolationToolCfg,
    ActsTrackingGeometryToolCfg,
    ActsATLASConverterToolCfg,
)
from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg


def ActsTrkFindingToolCfg(flags, **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("etaBins", [])
    kwargs.setdefault("chi2CutOff", [15.0])
    kwargs.setdefault("numMeasurementsCutOff", [10])

    kwargs.setdefault(
        "TrackingGeometryTool",
        acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)),
    )
    kwargs.setdefault(
        "ExtrapolationTool",
        acc.getPrimaryAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
    )

    kwargs.setdefault(
        "SummaryTool", acc.getPrimaryAndMerge(InDetTrackSummaryToolCfg(flags))
    )

    kwargs.setdefault(
        "ATLASConverterTool",
        acc.popToolsAndMerge(ActsATLASConverterToolCfg(flags)),
    )

    if flags.Detector.GeometryITk:
        from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg

        boundaryCheckToolCfg = ITkBoundaryCheckToolCfg(flags)
    else:
        from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg

        boundaryCheckToolCfg = InDetBoundaryCheckToolCfg(flags)

    kwargs.setdefault(
        "BoundaryCheckTool",
        acc.popToolsAndMerge(boundaryCheckToolCfg),
    )

    if flags.Acts.doRotCorrection:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg

        kwargs.setdefault(
            "RotCreatorTool",
            acc.popToolsAndMerge(ITkRotCreatorCfg(flags, name="ActsRotCreatorTool")),
        )

    acc.setPrivateTools(
        CompFactory.ActsTrk.TrackFindingTool(name="ActsTrackFindingTool", **kwargs)
    )
    return acc
