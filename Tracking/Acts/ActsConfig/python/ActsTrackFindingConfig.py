# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Tools


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

    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault(
        "ATLASConverterTool",
        acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
    )

    acc.setPrivateTools(CompFactory.ActsTrk.TrackStatePrinter(name, **kwargs))
    return acc

# ACTS only algorithm


def ActsTrackFindingCfg(flags,
                        name: str = "ActsTrackFindingAlg",
                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

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

    kwargs.setdefault('ACTSTracksLocation', 'ActsTracks')

    if flags.Acts.doAmbiguityResolution:
        kwargs.setdefault(
            "ACTSTracksLocation",
            "ActsTracks"
        )

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsTrackFindingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsTrackFindingMonitoringToolCfg(flags)))

    kwargs.setdefault("maxPropagationStep", 10000)
    kwargs.setdefault("skipDuplicateSeeds", flags.Acts.skipDuplicateSeeds)
    kwargs.setdefault("etaBins", [])
    kwargs.setdefault("chi2CutOff", [flags.Acts.trackFindingChi2CutOff])
    kwargs.setdefault("numMeasurementsCutOff", [3])

    if flags.Acts.doTrackFindingTrackSelector:
        # TrackSelector no tighter than createITkTrackingPassFlags() (https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/TrkConfig/python/TrackingPassFlags.py#L383)
        kwargs.setdefault("absEtaMax", 4.0)
        kwargs.setdefault("ptMin", 0.4)
        kwargs.setdefault("minMeasurements", 7)

    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg, ActsTrackingGeometryToolCfg
    kwargs.setdefault(
        "TrackingGeometryTool",
        acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
    )  # PrivateToolHandle
    kwargs.setdefault(
        "ExtrapolationTool",
        acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
    )  # PrivateToolHandle

    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault(
        "ATLASConverterTool",
        acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)),
    )

    if flags.Acts.doPrintTrackStates:
        kwargs.setdefault(
            "TrackStatePrinter",
            acc.popToolsAndMerge(ActsTrackStatePrinterCfg(flags)),
        )
    kwargs.setdefault("SeedLabels",["PPP","SSS"])
    # there is always an over and underflow bin so the first bin will be 0. - 0.5 the last bin 3.5 - inf.
    # if all eta bins are >=0. the counter will be categorized qby abs(eta) otherwise eta
    kwargs.setdefault("StatisticEtaBins",[ eta/10. for eta in range(5,40,5)]) # eta 0.0 - 4.0 in steps of 0.5
    kwargs.setdefault("DumpEtaBinsForAll",False)

    acc.addEventAlgo(CompFactory.ActsTrk.TrackFindingAlg(name, **kwargs))
    return acc


def ActsAmbiguityResolutionCfg(flags, name: str = "ActsAmbiguityResolution", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault('ResolvedTracksLocation', 'ResolvedActsTracks')
    kwargs.setdefault('MaximumSharedHits', 3)
    kwargs.setdefault('MaximumIterations', 10000)
    kwargs.setdefault('NMeasurementsMin', 7)

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsAmbiguityResolutionMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(
            ActsAmbiguityResolutionMonitoringToolCfg(flags)))

    acc.addEventAlgo(
        CompFactory.ActsTrk.AmbiguityResolutionAlg(name, **kwargs))
    return acc
