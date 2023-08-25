# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
from ActsConfig.ActsConfigFlags import SeedingStrategy
from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
from ActsInterop import UnitConstants

# ACTS tools
def ActsITkPixelSeedingToolCfg(flags,
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## For ITkPixel
    kwargs.setdefault("numSeedIncrement" , float("inf"))
    kwargs.setdefault("deltaZMax" , float("inf"))
    kwargs.setdefault("maxPtScattering", float("inf"))
    acc.setPrivateTools(CompFactory.ActsTrk.SeedingTool(name = "ActsSeedingTool_ITkPixel", **kwargs))
    return acc

def ActsITkFastPixelSeedingToolCfg(flags,
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## For ITkPixel
    kwargs.setdefault("numSeedIncrement" , float("inf"))
    kwargs.setdefault("deltaZMax" , float("inf"))
    kwargs.setdefault("maxPtScattering", float("inf"))

    ## Additional cuts for fast seed configuration

    acc.setPrivateTools(CompFactory.ActsTrk.SeedingTool(name = "ActsFastSeedingTool_ITkPixel", **kwargs))
    return acc

def ActsITkStripSeedingToolCfg(flags,
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## For ITkStrip, change properties that have to be modified w.r.t. the default values
    kwargs.setdefault("doSeedQualitySelection", False)
    # For SpacePointGridConfig
    kwargs.setdefault("gridRMax" , 1000. * UnitConstants.mm)
    kwargs.setdefault("deltaRMax" , 600. * UnitConstants.mm)
    kwargs.setdefault("impactMax" , 20. * UnitConstants.mm)
    # For SeedfinderConfig
    kwargs.setdefault("rMax" , 1200. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinTopSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxTopSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinBottomSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxBottomSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaZMax" , 900. * UnitConstants.mm)
    kwargs.setdefault("interactionPointCut" , False)
    kwargs.setdefault("arithmeticAverageCotTheta" , True)
    kwargs.setdefault("zBinsCustomLooping" , [6, 7, 5, 8, 4, 9, 3, 10, 2, 11, 1])
    kwargs.setdefault("deltaRMiddleMinSPRange" , 30 * UnitConstants.mm)
    kwargs.setdefault("deltaRMiddleMaxSPRange" , 150 * UnitConstants.mm)
    kwargs.setdefault("useDetailedDoubleMeasurementInfo" , True)
    kwargs.setdefault("maxPtScattering", float("inf"))
    # For SeedFilterConfig
    kwargs.setdefault("useDeltaRorTopRadius" , False)
    kwargs.setdefault("seedConfirmationInFilter" , False)
    kwargs.setdefault("impactWeightFactor" , 1.)
    kwargs.setdefault("compatSeedLimit" , 4)
    kwargs.setdefault("numSeedIncrement" , 1.)
    kwargs.setdefault("seedWeightIncrement" , 10100.)
    kwargs.setdefault("maxSeedsPerSpMConf" , 100)
    kwargs.setdefault("maxQualitySeedsPerSpMConf" , 100)
    # For seeding algorithm
    kwargs.setdefault("zBinNeighborsBottom" , [(0,1),(0,1),(0,1),(0,2),(0,1),(0,0),(-1,0),(-2,0),(-1,0),(-1,0),(-1,0)])

    acc.setPrivateTools(CompFactory.ActsTrk.SeedingTool(name = "ActsSeedingTool_ITkStrip", **kwargs))
    return acc

def ActsITkPixelOrthogonalSeedingToolCfg(flags,
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## For ITkPixel, use default values for ActsTrk::OrthogonalSeedingTool
    acc.setPrivateTools(CompFactory.ActsTrk.OrthogonalSeedingTool(name = "OrthogonalSeedingTool_ITkPixel", **kwargs))
    return acc

def ActsITkStripOrthogonalSeedingToolCfg(flags,
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    ## For ITkStrip, change properties that have to be modified w.r.t. the default values
    kwargs.setdefault("impactMax" , 20. * UnitConstants.mm)
    kwargs.setdefault('rMax', 1200. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinTopSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxTopSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaRMinBottomSP" , 20. * UnitConstants.mm)
    kwargs.setdefault("deltaRMaxBottomSP" , 300. * UnitConstants.mm)
    kwargs.setdefault("deltaZMax" , 900. * UnitConstants.mm)
    kwargs.setdefault("interactionPointCut" , False)
    kwargs.setdefault("impactWeightFactor" , 1.)
    kwargs.setdefault("compatSeedLimit" , 4)
    kwargs.setdefault("seedWeightIncrement" , 10100.)
    kwargs.setdefault("numSeedIncrement" , 1.)
    kwargs.setdefault("seedConfirmationInFilter" , False)
    kwargs.setdefault("maxSeedsPerSpMConf" , 100)
    kwargs.setdefault("maxQualitySeedsPerSpMConf" , 100)
    kwargs.setdefault("useDeltaRorTopRadius" , False)
    kwargs.setdefault("rMinMiddle", 33. * UnitConstants.mm)
    kwargs.setdefault("rMaxMiddle", 1200. * UnitConstants.mm)

    acc.setPrivateTools(CompFactory.ActsTrk.OrthogonalSeedingTool(name = "OrthogonalSeedingTool_ITkStrip", **kwargs))
    return acc

def ActsSiSpacePointsSeedMakerCfg(flags,
                                  name: str = 'ActsSiSpacePointsSeedMaker',
                                  **kwargs) -> ComponentAccumulator:
    assert isinstance(name, str)

    acc = ComponentAccumulator()

    kwargs['name'] = name

    # Main properties
    kwargs.setdefault('usePixel', 
                      flags.Tracking.ActiveConfig.useITkPixel and
                      flags.Tracking.ActiveConfig.useITkPixelSeeding)
    kwargs.setdefault('useStrip',
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault('useOverlapSpCollection',
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault('ActsSpacePointsPixelName'    , "ITkPixelSpacePoints")
    kwargs.setdefault('ActsSpacePointsStripName'    , "ITkStripSpacePoints")
    kwargs.setdefault('ActsSpacePointsOverlapName'  , "ITkStripOverlapSpacePoints")

    from ActsConfig.TrackingComponentConfigurer import TrackingComponentConfigurer
    configuration_settings = TrackingComponentConfigurer(flags)

    # The code will need to use Trk::SpacePoint object for downstream Athena tracking
    # If we run this tool we have two options to retrieve this:
    #     (1) Have the Athena->Acts Space Point Converter scheduled beforehand
    #     (2) Have the Athena->Acts Cluster Converter scheduled beforehand
    # In case (1) the link xAOD -> Trk Space Point will be used to retrieve the Trk::SpacePoints
    # In case (2) the link xAOD -> InDet Cluster will be used to create the Trk::SpacePoints
    # If none of the above conditions are met, it means there is a misconfiguration of the algorithms
    useClusters = configuration_settings.doAthenaToActsCluster and not configuration_settings.doAthenaToActsSpacePoint
    kwargs.setdefault('useClustersForSeedConversion', useClusters)

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault('PRDtoTrackMap', (
            'ITkPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    # Acts Seed Tools
    # Do not overwrite if already present in `kwargs`
    seedTool_pixel = None
    if 'SeedToolPixel' not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            seedTool_pixel = acc.popToolsAndMerge(ActsITkPixelOrthogonalSeedingToolCfg(flags))
        else:
            if flags.Tracking.doITkFastTracking:
                kwargs.setdefault("useFastTracking", True)
                seedTool_pixel = acc.popToolsAndMerge(ActsITkFastPixelSeedingToolCfg(flags))
            else:
                seedTool_pixel = acc.popToolsAndMerge(ActsITkPixelSeedingToolCfg(flags))

    seedTool_strip = None
    if 'SeedToolStrip' not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            seedTool_strip = acc.popToolsAndMerge(ActsITkStripOrthogonalSeedingToolCfg(flags))
        else:
            seedTool_strip = acc.popToolsAndMerge(ActsITkStripSeedingToolCfg(flags))

    kwargs.setdefault('SeedToolPixel', seedTool_pixel)
    kwargs.setdefault('SeedToolStrip', seedTool_strip)

    # Validation
    if flags.Tracking.writeSeedValNtuple:
        kwargs.setdefault('WriteNtuple', True)
        HistService = CompFactory.THistSvc(Output = ["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"])
        acc.addService(HistService)

    acc.setPrivateTools(CompFactory.ActsTrk.SiSpacePointsSeedMaker(**kwargs))
    return acc


# ACTS algorithm using Athena objects upstream
def ActsITkPixelSeedingCfg(flags,
                           name: str = 'ActsPixelSeedingAlg',
                           **kwargs):
    acc = ComponentAccumulator()

    # Need To add additional tool(s)
    # Tracking Geometry Tool
    geoTool = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
    acc.addPublicTool(geoTool)

    # ATLAS Converter Tool
    converterTool = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))

    # Track Param Estimation Tool
    trackEstimationTool = acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags))

    seedTool = None
    if "SeedTool" not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            seedTool = acc.popToolsAndMerge(ActsITkPixelOrthogonalSeedingToolCfg(flags))
        else:
            if flags.Tracking.doITkFastTracking:
                kwargs.setdefault("useFastTracking", True)
                seedTool = acc.popToolsAndMerge(ActsITkFastPixelSeedingToolCfg(flags))
            else:
                seedTool = acc.popToolsAndMerge(ActsITkPixelSeedingToolCfg(flags))

    kwargs.setdefault('InputSpacePoints', ['ITkPixelSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ITkPixelSeeds')
    kwargs.setdefault('SeedTool', seedTool)
    kwargs.setdefault('TrackingGeometryTool', acc.getPublicTool(geoTool.name)) # PublicToolHandle
    kwargs.setdefault('ATLASConverterTool', converterTool)
    kwargs.setdefault('TrackParamsEstimationTool', trackEstimationTool)
    kwargs.setdefault('OutputEstimatedTrackParameters', 'ITkPixelEstimatedTrackParams')
    kwargs.setdefault('DetectorElements', 'ITkPixelDetectorElementCollection')

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsITkPixelSeedingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkPixelSeedingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
    return acc


def ActsITkStripSeedingCfg(flags,
                           name: str = 'ActsStripSeedingAlg',
                           **kwargs):
    acc = ComponentAccumulator()

    # Need To add additional tool(s)
    # Tracking Geometry Tool
    geoTool = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
    acc.addPublicTool(geoTool)

    # ATLAS Converter Tool
    converterTool = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))

    # Track Param Estimation Tool
    trackEstimationTool = acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags))

    seedTool = None
    if "SeedTool" not in kwargs:
        if flags.Acts.SeedingStrategy is SeedingStrategy.Orthogonal:
            seedTool = acc.popToolsAndMerge(ActsITkStripOrthogonalSeedingToolCfg(flags))
        else:
            seedTool = acc.popToolsAndMerge(ActsITkStripSeedingToolCfg(flags))

    kwargs.setdefault('InputSpacePoints', ['ITkStripSpacePoints', 'ITkStripOverlapSpacePoints'])
    kwargs.setdefault('OutputSeeds', 'ITkStripSeeds')
    kwargs.setdefault('SeedTool', seedTool)
    kwargs.setdefault('TrackingGeometryTool', acc.getPublicTool(geoTool.name)) # PublicToolHandle
    kwargs.setdefault('ATLASConverterTool', converterTool)
    kwargs.setdefault('TrackParamsEstimationTool', trackEstimationTool)
    kwargs.setdefault('OutputEstimatedTrackParameters', 'ITkStripEstimatedTrackParams')
    kwargs.setdefault('DetectorElements', 'ITkStripDetectorElementCollection')

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsITkStripSeedingMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkStripSeedingMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.SeedingAlg(name, **kwargs))
    return acc


def ActsSeedingCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsITkPixelSeedingCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsITkStripSeedingCfg(flags))

    if flags.Acts.doAnalysis:
        from ActsConfig.ActsAnalysisConfig import ActsSeedAnalysisCfg, ActsEstimatedTrackParamsAnalysisCfg
        acc.merge(ActsSeedAnalysisCfg(flags))
        acc.merge(ActsEstimatedTrackParamsAnalysisCfg(flags))

    return acc

