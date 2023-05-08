# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsTrkSeedingToolConfig import ActsTrkITkPixelSeedingToolCfg, ActsTrkITkStripSeedingToolCfg
from ActsConfig.ActsTrkSeedingToolConfig import ActsTrkITkPixelOrthogonalSeedingToolCfg, ActsTrkITkStripOrthogonalSeedingToolCfg
from ActsConfig.ActsTrkTrackParamsEstimationToolConfig import TrackParamsEstimationToolCfg
from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
from ActsConfig.ActsConfigFlags import SeedingStrategy
from ActsConfig.ActsTrkEventCnvConfig import ActsToTrkConverterToolCfg

# ACTS algorithm using Athena objects upstream
def ActsTrkITkPixelSeedingCfg(flags,
                              name: str = 'ActsTrkPixelSeedingAlg',
                              **kwargs):
    acc = ComponentAccumulator()

    # Need To add additional tool(s)
    # Tracking Geometry Tool
    geoTool = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
    acc.addPublicTool(geoTool)

    # ATLAS Converter Tool
    converterTool = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))

    # Track Param Estimation Tool
    trackEstimationTool = acc.popToolsAndMerge(TrackParamsEstimationToolCfg(flags))

    seedTool = None
    if "SeedTool" not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            seedTool = acc.popToolsAndMerge(ActsTrkITkPixelOrthogonalSeedingToolCfg(flags))
        else:
            seedTool = acc.popToolsAndMerge(ActsTrkITkPixelSeedingToolCfg(flags))

    kwargs.setdefault('InputSpacePoints', ['ITkPixelSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ITkPixelSeeds')
    kwargs.setdefault('SeedTool', seedTool)
    kwargs.setdefault('TrackingGeometryTool', acc.getPublicTool(geoTool.name)) # PublicToolHandle
    kwargs.setdefault('ATLASConverterTool', converterTool)
    kwargs.setdefault('TrackParamsEstimationTool', trackEstimationTool)
    kwargs.setdefault('OutputEstimatedTrackParameters', 'ITkPixelEstimatedTrackParams')
    kwargs.setdefault('UsePixel', True)
    kwargs.setdefault('DetectorElements', 'ITkPixelDetectorElementCollection')

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkITkPixelSeedingMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkITkPixelSeedingMonitoringCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
    return acc


def ActsTrkITkStripSeedingCfg(flags,
                              name: str = 'ActsTrkStripSeedingAlg',
                              **kwargs):
    acc = ComponentAccumulator()

    # Need To add additional tool(s)
    # Tracking Geometry Tool
    geoTool = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
    acc.addPublicTool(geoTool)

    # ATLAS Converter Tool
    converterTool = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))

    # Track Param Estimation Tool
    trackEstimationTool = acc.popToolsAndMerge(TrackParamsEstimationToolCfg(flags))

    seedTool = None
    if "SeedTool" not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            seedTool = acc.popToolsAndMerge(ActsTrkITkStripOrthogonalSeedingToolCfg(flags))
        else:
            seedTool = acc.popToolsAndMerge(ActsTrkITkStripSeedingToolCfg(flags))

    kwargs.setdefault('InputSpacePoints', ['ITkStripSpacePoints', 'ITkStripOverlapSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ITkStripSeeds')
    kwargs.setdefault('SeedTool', seedTool)
    kwargs.setdefault('TrackingGeometryTool', acc.getPublicTool(geoTool.name)) # PublicToolHandle
    kwargs.setdefault('ATLASConverterTool', converterTool)
    kwargs.setdefault('TrackParamsEstimationTool', trackEstimationTool)
    kwargs.setdefault('OutputEstimatedTrackParameters', 'ITkStripEstimatedTrackParams')
    kwargs.setdefault('UsePixel', False)
    kwargs.setdefault('DetectorElements', 'ITkStripDetectorElementCollection')

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkITkStripSeedingMonitoringCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsTrkITkStripSeedingMonitoringCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
    return acc


def ActsTrkSeedingCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsTrkITkPixelSeedingCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsTrkITkStripSeedingCfg(flags))

    if flags.Acts.doAnalysis:
        from ActsConfig.ActsTrkAnalysisConfig import ActsTrkSeedAnalysisCfg, ActsTrkEstimatedTrackParamsAnalysisCfg
        acc.merge(ActsTrkSeedAnalysisCfg(flags))
        acc.merge(ActsTrkEstimatedTrackParamsAnalysisCfg(flags))

    return acc

