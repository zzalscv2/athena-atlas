# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import SpacePointStrategy

def ActsPixelSpacePointToolCfg(flags, name = "ActsPixelSpacePointTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.PixelSpacePointFormationTool(name, **kwargs))
    return acc

def ActsStripSpacePointToolCfg(flags, name = "ActsStripSpacePointTool", **kwargs):
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
    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault("ConverterTool", acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    # need to persistify source link helper container to ensure that source links do not contaion
    # stale pointers pointing to freed memory
    kwargs.setdefault("ATLASUncalibSourceLinkElementsName","ACTSStripSourceLinkElements")

    acc.setPrivateTools(CompFactory.ActsTrk.ActsCoreStripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsPixelSpacePointFormationCfg(flags,
                                    name = "ActsPixelSpacePointFormation",
                                    **kwargs):

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc = ITkPixelReadoutGeometryCfg(flags)

    kwargs.setdefault("SpacePointFormationTool", acc.popToolsAndMerge(ActsPixelSpacePointToolCfg(flags)))
    kwargs.setdefault("PixelClusters", "ITkPixelClusters")
    kwargs.setdefault("PixelDetectorElements", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("PixelSpacePoints", "ITkPixelSpacePoints")

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsPixelSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsPixelSpacePointFormationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsStripSpacePointFormationCfg(flags,
                                    name = "ActsStripSpacePointFormation",
                                    **kwargs):

    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc = ITkStripReadoutGeometryCfg(flags)

    actsStripSpacePointTool = None
    if flags.Acts.SpacePointStrategy is SpacePointStrategy.ActsCore:
        actsStripSpacePointTool = acc.popToolsAndMerge(ActsCoreStripSpacePointToolCfg(flags))
    else:
        actsStripSpacePointTool = acc.popToolsAndMerge(ActsStripSpacePointToolCfg(flags))

    kwargs.setdefault("SpacePointFormationTool", actsStripSpacePointTool)
    kwargs.setdefault("StripClusters", "ITkStripClusters")
    kwargs.setdefault("StripDetectorElements", "ITkStripDetectorElementCollection")
    kwargs.setdefault("StripElementPropertiesTable", "ITkStripElementPropertiesTable")
    kwargs.setdefault("StripSpacePoints", "ITkStripSpacePoints")
    kwargs.setdefault("StripOverlapSpacePoints", "ITkStripOverlapSpacePoints")
    kwargs.setdefault("ProcessOverlapForStrip", True)

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsStripSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsStripSpacePointFormationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.StripSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsSpacePointFormationCfg(flags):
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelSpacePointFormationCfg(flags))
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
        
        acc.merge(ActsStripSpacePointFormationCfg(flags))

    if flags.Acts.doAnalysis:
        from ActsConfig.ActsAnalysisConfig import ActsSpacePointAnalysisCfg
        acc.merge(ActsSpacePointAnalysisCfg(flags))
        
    return acc
