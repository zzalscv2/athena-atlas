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
    kwargs.setdefault("chi2CutOff", [flags.Acts.trackFindingChi2CutOff])
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

    # need to persistify source link helper container to ensure that source links do not contaion
    # stale pointers pointing to freed memory
    kwargs.setdefault("ATLASUncalibSourceLinkElementsName","ACTSUncalibratedMeasurementSourceLinkElements")

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

    if flags.Detector.EnableITkPixel:
        kwargs.setdefault("PixelClusterContainerKey", "ITkPixelClusters")
        kwargs.setdefault("PixelDetectorElements",
                          "ITkPixelDetectorElementCollection")
        kwargs.setdefault("PixelEstimatedTrackParameters",
                          "ITkPixelEstimatedTrackParams")
        kwargs.setdefault('PixelSeeds', 'ITkPixelSeeds')

    if flags.Detector.EnableITkStrip:
        kwargs.setdefault("StripClusterContainerKey", "ITkStripClusters")
        kwargs.setdefault("StripDetectorElements",
                          "ITkStripDetectorElementCollection")
        kwargs.setdefault("StripEstimatedTrackParameters",
                          "ITkStripEstimatedTrackParams")
        kwargs.setdefault('StripSeeds', 'ITkStripSeeds')

    kwargs.setdefault("TracksLocation", "SiSPSeededActsTracks")


    if flags.Acts.doAmbiguityResolution:
        kwargs.setdefault(
            "ACTSTracksLocation",
            "ActsTracks"
        )

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkFindingMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrkFindingMonitoringCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc

def ActsAmbiguityResolutionCfg(flags, name: str = "ActsAmbiguityResolution", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ActsTracksResolved')
    kwargs.setdefault('MaximumSharedHits', 3)
    kwargs.setdefault('MaximumIterations', 10000)
    kwargs.setdefault('NMeasurementsMin', 7)

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsAmbiguityResolutionMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsAmbiguityResolutionMonitoringCfg(flags)))
    
    acc.addEventAlgo(CompFactory.ActsTrk.AmbiguityResolutionAlg(name, **kwargs))
    return acc
