# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import SpacePointStrategy

def ActsTrkPixelSpacePointToolCfg(flags, name = "ActsTrkPixelSpacePointTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.PixelSpacePointFormationTool(name, **kwargs))
    return acc

def ActsTrkStripSpacePointToolCfg(flags, name = "ActsTrkStripSpacePointTool", **kwargs):
    acc = ComponentAccumulator()

    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )
    kwargs.setdefault("AllClusters", False)

    acc.setPrivateTools(CompFactory.ActsTrk.ActsTrkStripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsCoreStripSpacePointToolCfg(flags, name = "ActsCoreStripSpacePointTool", **kwargs):
    acc = ComponentAccumulator()

    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )
    kwargs.setdefault("AllClusters", False)
    from ActsConfig.ActsTrkEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault("ConverterTool", acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    acc.setPrivateTools(CompFactory.ActsTrk.ActsCoreStripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsTrkPixelSpacePointFormationCfg(flags,
                                       name = "ActsTrkPixelSpacePointFormation",
                                       **kwargs):

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc = ITkPixelReadoutGeometryCfg(flags)

    ActsTrkPixelSpacePointTool = acc.popToolsAndMerge(ActsTrkPixelSpacePointToolCfg(flags))
    kwargs.setdefault("SpacePointFormationTool", ActsTrkPixelSpacePointTool)

    kwargs.setdefault("PixelClusters", "ITkPixelClusters")
    kwargs.setdefault("PixelDetectorElements", "ITkPixelDetectorElementCollection")

    kwargs.setdefault("PixelSpacePoints", "ITkPixelSpacePoints")

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkPixelSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsTrkPixelSpacePointFormationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsTrkStripSpacePointFormationCfg(flags,
                                       name = "ActsTrkStripSpacePointFormation",
                                       **kwargs):

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc = ITkStripReadoutGeometryCfg(flags)

    if flags.Acts.SpacePointStrategy is SpacePointStrategy.ActsCore:
        ActsTrkStripSpacePointTool = acc.popToolsAndMerge(ActsCoreStripSpacePointToolCfg(flags))
    else:
        ActsTrkStripSpacePointTool = acc.popToolsAndMerge(ActsTrkStripSpacePointToolCfg(flags))

    kwargs.setdefault("SpacePointFormationTool", ActsTrkStripSpacePointTool)

    kwargs.setdefault("StripClusters", "ITkStripClusters")
    kwargs.setdefault("StripDetectorElements", "ITkStripDetectorElementCollection")
    kwargs.setdefault("StripElementPropertiesTable", "ITkStripElementPropertiesTable")

    kwargs.setdefault("StripSpacePoints", "ITkStripSpacePoints")
    kwargs.setdefault("StripOverlapSpacePoints", "ITkStripOverlapSpacePoints")
    kwargs.setdefault("ProcessOverlapForStrip", True)

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsTrkMonitoringConfig import ActsTrkStripSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsTrkStripSpacePointFormationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.StripSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsTrkSpacePointFormationCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsTrkPixelSpacePointFormationCfg(flags))
    if flags.Detector.EnableITkStrip:
        # Need to schedule this here in case the Athena space point formation is not schedule
        # This is because as of now requires at least ITkSiElementPropertiesTableCondAlgCfg
        # This may be because the current strip space point formation algorithm is not using Acts
        # May be not necessary once the Acts-based strip space point maker is ready
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        acc.merge(ITkStripReadoutGeometryCfg(flags))
        
        from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
        acc.merge(BeamSpotCondAlgCfg(flags))
        
        from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg
        acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))
        
        acc.merge(ActsTrkStripSpacePointFormationCfg(flags))

    if flags.Acts.doAnalysis:
        from ActsConfig.ActsTrkAnalysisConfig import ActsTrkSpacePointAnalysisCfg
        acc.merge(ActsTrkSpacePointAnalysisCfg(flags))
        
    return acc
