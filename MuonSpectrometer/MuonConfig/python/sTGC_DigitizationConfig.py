"""Define methods to construct configured STGC Digitization tools and algorithms

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonByteStreamCnvTestConfig import STGC_DigitToRDOCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the sTgcDigitizationTool.
def sTGC_FirstXing():
    return -100


def sTGC_LastXing():
    return 100


def sTGC_RangeCfg(flags, name="sTgcRange", **kwargs):
    """Return a PileUpXingFolder tool configured for sTGC"""
    kwargs.setdefault("FirstXing", sTGC_FirstXing())
    kwargs.setdefault("LastXing", sTGC_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    if 'sTGCSimHitCollection#sTGCSensitiveDetector' in flags.Input.TypedCollections:
        kwargs.setdefault("ItemList", ["sTGCSimHitCollection#sTGCSensitiveDetector"])
    else:
        kwargs.setdefault("ItemList", ["sTGCSimHitCollection#sTGC_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def sTGC_DigitizationToolCfg(flags, name="sTgcDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured sTgcDigitizationTool"""
    from MuonConfig.MuonCalibrationConfig import NSWCalibToolCfg, STgcCalibSmearingToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("CalibrationTool", result.popToolsAndMerge(NSWCalibToolCfg(flags)))
    kwargs.setdefault("SmearingTool", result.popToolsAndMerge(STgcCalibSmearingToolCfg(flags)))
    kwargs.setdefault("padChargeSharing", False)
    # sTGC VMM configurables
    kwargs.setdefault("deadtimeStrip", 250)
    kwargs.setdefault("deadtimePad"  , 250)
    kwargs.setdefault("deadtimeWire" , 250)
    kwargs.setdefault("neighborOn", True)
    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp:
            kwargs.setdefault("FirstXing", sTGC_FirstXing())
            kwargs.setdefault("LastXing", sTGC_LastXing())
        else:
            intervals += [result.popToolsAndMerge(sTGC_RangeCfg(flags))]
        kwargs.setdefault("MergeSvc", result.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    else:
        kwargs.setdefault("MergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("doToFCorrection", True)
    kwargs.setdefault("doEfficiencyCorrection", False)
    # Operating voltage in the sTGC in kV. Sets the gas gain from electron avalance
    # Every 100V increase roughly doubles the total electric charge per hit
    kwargs.setdefault("operatingHVinkV", 2.8)

    if 'sTGCSimHitCollection#sTGCSensitiveDetector' in flags.Input.TypedCollections:
        kwargs.setdefault("InputObjectName", "sTGCSensitiveDetector")
    else:
        kwargs.setdefault("InputObjectName", "sTGC_Hits")
    kwargs.setdefault("OutputObjectName", "sTGC_DIGITS")
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "sTGC_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "sTGC_SDO")
    sTgcDigitizationTool = CompFactory.sTgcDigitizationTool(name, **kwargs)
    result.setPrivateTools(sTgcDigitizationTool)
    return result


def sTGC_OverlayDigitizationToolCfg(flags, name="STGC_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with TgcDigitizationTool configured for Overlay"""
    acc = ComponentAccumulator()
    kwargs.setdefault("doToFCorrection", True)
    kwargs.setdefault("doEfficiencyCorrection", False)
    # Operating voltage in the sTGC in kV. Sets the gas gain from electron avalance
    # Every 100V increase roughly doubles the total electric charge per hit
    kwargs.setdefault("operatingHVinkV", 2.8)
    kwargs.setdefault("MergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", False)
    if 'sTGCSimHitCollection#sTGCSensitiveDetector' in flags.Input.SecondaryTypedCollections:
        kwargs.setdefault("InputObjectName", "sTGCSensitiveDetector")
    else:
        kwargs.setdefault("InputObjectName", "sTGC_Hits")
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "sTGC_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "sTGC_SDO")
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    sTgcDigitizationTool = CompFactory.sTgcDigitizationTool
    acc.setPrivateTools(sTgcDigitizationTool(name, **kwargs))
    return acc


def sTGC_OutputCfg(flags):
    """Return ComponentAccumulator with Output for sTGC. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["Muon::STGC_RawDataContainer#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["MuonSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def sTGC_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for sTGC digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(sTGC_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def sTGC_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with sTGC Overlay digitization"""
    acc = MuonGeoModelCfg(flags)
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        if 'sTGCSimHitCollection#sTGCSensitiveDetector' in flags.Input.SecondaryTypedCollections:
            acc.merge(SGInputLoaderCfg(flags, ["sTGCSimHitCollection#sTGCSensitiveDetector"]))
        else:
            acc.merge(SGInputLoaderCfg(flags, ["sTGCSimHitCollection#sTGC_Hits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(sTGC_OverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.sTGC_Digitizer(name="STGC_OverlayDigitizer", **kwargs))
    return acc


# with output defaults
def sTGC_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for sTGC digitization and Output"""
    acc = sTGC_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(sTGC_OutputCfg(flags))
    return acc


def sTGC_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with sTGC digitization and Digit to TGCRDO"""
    acc = sTGC_DigitizationCfg(flags)
    acc.merge(STGC_DigitToRDOCfg(flags))
    return acc
