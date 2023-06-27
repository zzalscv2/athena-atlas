# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Tools


def ActsTrkFindingToolCfg(
    flags, name: str = "ActsTrackFindingTool", **kwargs
) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("skipDuplicateSeeds", flags.Acts.skipDuplicateSeeds)
    kwargs.setdefault("etaBins", [])
    kwargs.setdefault("chi2CutOff", [15.0])
    kwargs.setdefault("numMeasurementsCutOff", [10])

    from ActsConfig.ActsTrkGeometryConfig import ActsExtrapolationToolCfg, ActsTrackingGeometryToolCfg
    kwargs.setdefault(
        "TrackingGeometryTool",
        acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
    )  # PrivateToolHandle
    kwargs.setdefault(
        "ExtrapolationTool",
        acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
    )  # PrivateToolHandle
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
    kwargs.setdefault(
        "SummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags))
    )  # PrivateToolHandle
    from ActsConfig.ActsTrkEventCnvConfig import ActsToTrkConverterToolCfg
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
            acc.popToolsAndMerge(RotCreatorCfg(
                flags, name="ActsRotCreatorTool")),
        )

    if flags.Acts.doPrintTrackStates:
        kwargs.setdefault(
            "TrackStatePrinter",
            acc.popToolsAndMerge(ActsTrackStatePrinterCfg(flags)),
        )

    acc.setPrivateTools(CompFactory.ActsTrk.TrackFindingTool(name, **kwargs))
    return acc


def ActsTrackStatePrinterCfg(
    flags, name: str = "TrackStatePrinter", **kwargs
) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault(
        "InputSpacePoints",
        [
            "ITkPixelSpacePoints",
            "ITkStripSpacePoints",
            "ITkStripOverlapSpacePoints",
        ],
    )
    kwargs.setdefault("spacePointType", [0, 1, 1])

    acc.setPrivateTools(CompFactory.ActsTrk.TrackStatePrinter(name, **kwargs))
    return acc

# ACTS only algorithm


def ActsTrkFindingCfg(flags, name: str = "ActsTrkFindingAlg", **kwargs):
    acc = ComponentAccumulator()

    # tools
    if "TrackFindingTool" not in kwargs:
        kwargs["TrackFindingTool"] = acc.popToolsAndMerge(
            ActsTrkFindingToolCfg(flags))

    kwargs.setdefault("PixelClusterContainerKey", "ITkPixelClusters")
    kwargs.setdefault("StripClusterContainerKey", "ITkStripClusters")
    kwargs.setdefault("PixelDetectorElements",
                      "ITkPixelDetectorElementCollection")
    kwargs.setdefault("StripDetectorElements",
                      "ITkStripDetectorElementCollection")
    kwargs.setdefault("PixelEstimatedTrackParameters",
                      "ITkPixelEstimatedTrackParams")
    kwargs.setdefault("StripEstimatedTrackParameters",
                      "ITkStripEstimatedTrackParams")
    kwargs.setdefault('PixelSeeds', 'ITkPixelSeeds')
    kwargs.setdefault('StripSeeds', 'ITkStripSeeds')
    kwargs.setdefault("TracksLocation", "SiSPSeededActsTracks")

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkFindingMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrkFindingMonitoringCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc
