"""Define methods to construct configured MDT Digitization tools and algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonByteStreamCnvTestConfig import MdtDigitToMdtRDOCfg
from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
from MuonConfig.MuonCalibrationConfig import MdtCalibrationDbToolCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the MdtDigitizationTool.
def MDT_FirstXing():
    return -800


def MDT_LastXing():
    # was 800 for large time window
    return 150


def MDT_RangeCfg(flags, name="MDT_Range", **kwargs):
    """Return a PileUpXingFolder tool configured for MDT"""
    kwargs.setdefault("FirstXing", MDT_FirstXing())
    kwargs.setdefault("LastXing",  MDT_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    kwargs.setdefault("ItemList", ["MDTSimHitCollection#MDT_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def RT_Relation_DB_DigiToolCfg(flags, name="RT_Relation_DB_DigiTool", **kwargs):
    """Return an RT_Relation_DB_DigiTool"""
    acc = ComponentAccumulator()
    RT_Relation_DB_DigiTool = CompFactory.RT_Relation_DB_DigiTool
    calibDbTool = acc.popToolsAndMerge(MdtCalibrationDbToolCfg(flags))
    kwargs.setdefault("CalibrationDbTool", calibDbTool)
    acc.setPrivateTools(RT_Relation_DB_DigiTool(name, **kwargs))
    return acc


def MDT_Response_DigiToolCfg(flags, name="MDT_Response_DigiTool",**kwargs):
    """Return a configured MDT_Response_DigiTool"""
    acc = ComponentAccumulator()
    QballConfig = (flags.Input.SpecialConfiguration.get("MDT_QballConfig", "False") == "True")
    kwargs.setdefault("DoQballGamma", QballConfig)
    MDT_Response_DigiTool = CompFactory.MDT_Response_DigiTool
    acc.setPrivateTools(MDT_Response_DigiTool(name, **kwargs))
    return acc


def MDT_DigitizationToolCommonCfg(flags, name="MdtDigitizationTool", **kwargs):
    """Return ComponentAccumulator with common MdtDigitizationTool config"""
    from MuonConfig.MuonCondAlgConfig import MdtCondDbAlgCfg # MT-safe conditions access
    acc = MdtCondDbAlgCfg(flags)
    calibDbTool = acc.popToolsAndMerge(MdtCalibrationDbToolCfg(flags))
    kwargs.setdefault("CalibrationDbTool", calibDbTool)
    kwargs.setdefault("DiscardEarlyHits", True)
    kwargs.setdefault("UseTof", flags.Beam.Type is not BeamType.Cosmics)
    # "RT_Relation_DB_DigiTool" in jobproperties.Digitization.experimentalDigi() not migrated
    digiTool = acc.popToolsAndMerge(MDT_Response_DigiToolCfg(flags))
    kwargs.setdefault("DigitizationTool", digiTool)
    QballConfig = (flags.Input.SpecialConfiguration.get("MDT_QballConfig", "False") == "True")
    kwargs.setdefault("DoQballCharge", QballConfig)
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", MDT_FirstXing())
        kwargs.setdefault("LastXing", MDT_LastXing())
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    MdtDigitizationTool = CompFactory.MdtDigitizationTool
    acc.setPrivateTools(MdtDigitizationTool(name, **kwargs))
    return acc


def MDT_DigitizationToolCfg(flags, name="MdtDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured MdtDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(MDT_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("OutputObjectName", "MDT_DIGITS")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "MDT_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "MDT_SDO")
    tool = acc.popToolsAndMerge(MDT_DigitizationToolCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def MDT_OverlayDigitizationToolCfg(flags, name="Mdt_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with MdtDigitizationTool configured for Overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "MDT_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "MDT_SDO")
    kwargs.setdefault("PileUpMergeSvc", '')
    return MDT_DigitizationToolCommonCfg(flags, name, **kwargs)


def MDT_OutputCfg(flags):
    """Return ComponentAccumulator with Output for MDT. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["MdtCsmContainer#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["MuonSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def MDT_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for MDT digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(MDT_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def MDT_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with MDT Overlay digitization"""
    acc = MuonGeoModelCfg(flags)

    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["MDTSimHitCollection#MDT_Hits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(MDT_OverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
       kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    MDT_Digitizer = CompFactory.MDT_Digitizer
    acc.addEventAlgo(MDT_Digitizer(name="MDT_OverlayDigitizer", **kwargs))
    return acc


# with output defaults
def MDT_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for MDT digitization and Output"""
    acc = MDT_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(MDT_OutputCfg(flags))
    return acc


def MDT_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with MDT digitization and Digit to MDTCSM RDO"""
    acc = MDT_DigitizationCfg(flags)
    acc.merge(MDTCablingConfigCfg(flags))
    acc.merge(MdtDigitToMdtRDOCfg(flags))
    return acc
