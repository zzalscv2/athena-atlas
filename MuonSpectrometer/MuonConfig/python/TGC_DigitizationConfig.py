"""Define methods to construct configured TGC Digitization tools and algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonByteStreamCnvTestConfig import TgcDigitToTgcRDOCfg
from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the TgcDigitizationTool.
def TGC_FirstXing():
    return -50


def TGC_LastXing():
    return 75


def TGC_RangeCfg(flags, name="TGC_Range", **kwargs):
    """Return a PileUpXingFolder tool configured for TGC"""
    kwargs.setdefault("FirstXing", TGC_FirstXing())
    kwargs.setdefault("LastXing", TGC_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    kwargs.setdefault("ItemList", ["TGCSimHitCollection#TGC_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def TGC_DigitizationToolCfg(flags, name="TgcDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured TgcDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp:
            kwargs.setdefault("FirstXing", TGC_FirstXing())
            kwargs.setdefault("LastXing", TGC_LastXing())
        else:
            intervals += [acc.popToolsAndMerge(TGC_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", TGC_FirstXing())
        kwargs.setdefault("LastXing", TGC_LastXing())
    kwargs.setdefault("OutputObjectName", "TGC_DIGITS")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "TGC_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "TGC_SDO")

    from MuonConfig.MuonCondAlgConfig import TgcDigitCondAlgCfg
    acc.merge(TgcDigitCondAlgCfg(flags))
    kwargs.setdefault("TGCDigitASDposKey", "TGCDigitASDposData")
    kwargs.setdefault("TGCDigitTimeOffsetKey", "TGCDigitTimeOffsetData")
    kwargs.setdefault("TGCDigitCrosstalkKey", "TGCDigitCrosstalkData")

    from AthenaConfiguration.Enums  import LHCPeriod
    if flags.GeoModel.Run < LHCPeriod.Run3:
        kwargs.setdefault("FourBunchDigitization", False)

    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    TgcDigitizationTool = CompFactory.TgcDigitizationTool
    acc.setPrivateTools(TgcDigitizationTool(name, **kwargs))
    return acc


def TGC_OverlayDigitizationToolCfg(flags, name="Tgc_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with TgcDigitizationTool configured for Overlay"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "TGC_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "TGC_SDO")

    from MuonConfig.MuonCondAlgConfig import TgcDigitCondAlgCfg
    acc.merge(TgcDigitCondAlgCfg(flags))
    kwargs.setdefault("TGCDigitASDposKey", "TGCDigitASDposData")
    kwargs.setdefault("TGCDigitTimeOffsetKey", "TGCDigitTimeOffsetData")
    kwargs.setdefault("TGCDigitCrosstalkKey", "TGCDigitCrosstalkData")

    from AthenaConfiguration.Enums  import LHCPeriod
    if flags.GeoModel.Run < LHCPeriod.Run3:
        kwargs.setdefault("FourBunchDigitization", False)

    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    kwargs.setdefault("PileUpMergeSvc", '')
    TgcDigitizationTool = CompFactory.TgcDigitizationTool
    acc.setPrivateTools(TgcDigitizationTool(name, **kwargs))
    return acc


def TGC_OutputCfg(flags):
    """Return ComponentAccumulator with Output for TGC. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["TgcRdoContainer#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["MuonSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def TGC_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for TGC digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(TGC_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def TGC_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with TGC Overlay digitization"""
    acc = MuonGeoModelCfg(flags)
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["TGCSimHitCollection#TGC_Hits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(TGC_OverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    TGCDigitizer = CompFactory.TGCDigitizer
    acc.addEventAlgo(TGCDigitizer(name="TGC_OverlayDigitizer", **kwargs))
    return acc


# with output defaults
def TGC_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for TGC digitization and Output"""
    acc = TGC_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(TGC_OutputCfg(flags))
    return acc


def TGC_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with TGC digitization and Digit to TGCRDO"""
    acc = TGC_DigitizationCfg(flags)
    acc.merge(TGCCablingConfigCfg(flags))
    acc.merge(TgcDigitToTgcRDOCfg(flags))
    return acc
