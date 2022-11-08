# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep


def RDOAnalysisOutputCfg(flags, output_name="RDOAnalysis"):
    result = ComponentAccumulator()

    histsvc = CompFactory.THistSvc(name="THistSvc",
                                   Output=[ f"{output_name} DATAFILE='{output_name}.root' OPT='RECREATE'" ])
    result.addService(histsvc)
    
    return result


def EventInfoRDOAnalysisCfg(flags, name="EventInfoRDOAnalysis", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("HistPath", "/RDOAnalysis/histos/")
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("EventInfo", f"{flags.Overlay.BkgPrefix}EventInfo")

    result.addEventAlgo(CompFactory.EventInfoRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def PixelRDOAnalysisCfg(flags, name="PixelRDOAnalysis", **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    result = PixelReadoutGeometryCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "Pixel")
    kwargs.setdefault("HistPath", "/RDOAnalysis/Pixel/")

    result.addEventAlgo(CompFactory.PixelRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def SCT_RDOAnalysisCfg(flags, name="SCT_RDOAnalysis", **kwargs):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    result = SCT_ReadoutGeometryCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "SCT")
    kwargs.setdefault("HistPath", "/RDOAnalysis/SCT/")

    result.addEventAlgo(CompFactory.SCT_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def ITkPixelRDOAnalysisCfg(flags, name="ITkPixelRDOAnalysis", **kwargs):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    result = ITkPixelReadoutGeometryCfg(flags)

    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("HistPath", "/RDOAnalysis/ITkPixel/")
    kwargs.setdefault("SharedHistPath", "/RDOAnalysis/histos/")

    result.addEventAlgo(CompFactory.ITk.PixelRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def ITkStripRDOAnalysisCfg(flags, name="ITkStripRDOAnalysis", **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    result = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("HistPath", "/RDOAnalysis/ITkStrip/")
    kwargs.setdefault("SharedHistPath", "/RDOAnalysis/histos/")

    result.addEventAlgo(CompFactory.ITk.StripRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def PLR_RDOAnalysisCfg(flags, name="PLR_RDOAnalysis", **kwargs):
    from PLRGeoModelXml.PLR_GeoModelConfig import PLR_ReadoutGeometryCfg
    result = PLR_ReadoutGeometryCfg(flags)

    kwargs.setdefault("CollectionName", "PLR_RDOs")
    kwargs.setdefault("SDOCollectionName", "PLR_SDO_Map")
    kwargs.setdefault("HistPath", "/RDOAnalysis/PLR/")
    kwargs.setdefault("SharedHistPath", "/RDOAnalysis/histos/")
    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("NtupleName", "PLR")
    kwargs.setdefault("DetectorName", "PLR")
    kwargs.setdefault("PixelIDName", "PLR_ID")

    result.addEventAlgo(CompFactory.ITk.PixelRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def RDOAnalysisCfg(flags):
    acc = ComponentAccumulator()

    acc.merge(EventInfoRDOAnalysisCfg(flags))

    if flags.Detector.EnablePixel:
        acc.merge(PixelRDOAnalysisCfg(flags))

    if flags.Detector.EnableSCT:
        acc.merge(SCT_RDOAnalysisCfg(flags))

    if flags.Detector.EnableITkPixel:
        acc.merge(ITkPixelRDOAnalysisCfg(flags))
    
    if flags.Detector.EnableITkStrip:
        acc.merge(ITkStripRDOAnalysisCfg(flags))

    if flags.Detector.EnablePLR:
        acc.merge(PLR_RDOAnalysisCfg(flags))

    return acc
