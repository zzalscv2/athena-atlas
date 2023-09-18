"""Define methods to construct configured SCT Digitization tools and algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, ProductionStep
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ReadCalibChipDataCfg, SCT_SiliconConditionsCfg
from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
from SiPropertiesTool.SCT_SiPropertiesConfig import SCT_SiPropertiesToolCfg

import AthenaCommon.SystemOfUnits as Units

# The earliest and last bunch crossing times for which interactions will be sent
# to the SCT Digitization code
def SCT_FirstXing():
    return -50


def SCT_LastXing():
    return 25


def SCT_DigitizationCommonCfg(flags, name="SCT_DigitizationToolCommon", **kwargs):
    """Return ComponentAccumulator with common SCT digitization tool config"""
    acc = SCT_ReadoutGeometryCfg(flags)
    if not flags.Digitization.DoInnerDetectorNoise:
        kwargs.setdefault("OnlyHitElements", True)
    kwargs.setdefault("InputObjectName", "SCT_Hits")
    kwargs.setdefault("EnableHits", True)
    kwargs.setdefault("BarrelOnly", False)
    # Set FixedTime for cosmics for use in SurfaceChargesGenerator
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("CosmicsRun", True)
        kwargs.setdefault("FixedTime", 10)
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", SCT_FirstXing())
        kwargs.setdefault("LastXing", SCT_LastXing() )
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    
    SCT_DigitizationTool = CompFactory.SCT_DigitizationTool
    tool = SCT_DigitizationTool(name, **kwargs)
    # attach ToolHandles
    tool.FrontEnd = acc.popToolsAndMerge(SCT_FrontEndCfg(flags))
    tool.SurfaceChargesGenerator = acc.popToolsAndMerge(SCT_SurfaceChargesGeneratorCfg(flags))
    tool.RandomDisabledCellGenerator = acc.popToolsAndMerge(SCT_RandomDisabledCellGeneratorCfg(flags))
    acc.setPrivateTools(tool)
    return acc


def SCT_DigitizationToolCfg(flags, name="SCT_DigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured SCT digitization tool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(SCT_RangeCfg(flags))]
        kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("MergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", flags.Overlay.BkgPrefix + "SCT_RDOs")
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "SCT_SDO_Map")
    else:
        kwargs.setdefault("OutputObjectName", "SCT_RDOs")
        kwargs.setdefault("OutputSDOName", "SCT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    tool = acc.popToolsAndMerge(SCT_DigitizationCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def SCT_DigitizationHSToolCfg(flags, name="SCT_DigitizationHSTool", **kwargs):
    """Return ComponentAccumulator with hard scatter configured SCT digitization tool"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(SCT_RangeCfg(flags))
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("OutputObjectName", "SCT_RDOs")
    kwargs.setdefault("OutputSDOName", "SCT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 1)
    tool = acc.popToolsAndMerge(SCT_DigitizationCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def SCT_DigitizationPUToolCfg(flags, name="SCT_DigitizationPUTool",**kwargs):
    """Return ComponentAccumulator with pileup configured SCT digitization tool"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(SCT_RangeCfg(flags))
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("OutputObjectName", "SCT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "SCT_PU_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    tool = acc.popToolsAndMerge(SCT_DigitizationCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def SCT_OverlayDigitizationToolCfg(flags, name="SCT_OverlayDigitizationTool",**kwargs):
    """Return ComponentAccumulator with overlay configured SCT digitization tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "SCT_RDOs")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "SCT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("MergeSvc", '')
    tool = acc.popToolsAndMerge(SCT_DigitizationCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def SCT_DigitizationToolSplitNoMergePUCfg(flags, name="SCT_DigitizationToolSplitNoMergePU",**kwargs):
    """Return ComponentAccumulator with merged pileup configured SCT digitization tool"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(SCT_RangeCfg(flags))
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("InputObjectName", "PileupSCT_Hits")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("OutputObjectName", "SCT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "SCT_PU_SDO_Map")
    kwargs.setdefault("OnlyHitElements", True)
    kwargs.setdefault("FrontEnd", "PileupSCT_FrontEnd")
    tool = acc.popToolsAndMerge(SCT_DigitizationCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def SCT_DigitizationToolGeantinoTruthCfg(flags, name="SCT_GeantinoTruthDigitizationTool", **kwargs):
    """Return Geantino truth configured digitization tool"""
    kwargs.setdefault("ParticleBarcodeVeto", 0)
    return SCT_DigitizationToolCfg(flags, name, **kwargs)


def SCT_RandomDisabledCellGeneratorCfg(flags, name="SCT_RandomDisabledCellGenerator", **kwargs):
    """Return configured random cell disabling tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("TotalBadChannels", 0.01)
    acc.setPrivateTools(CompFactory.SCT_RandomDisabledCellGenerator(name, **kwargs))
    return acc


def SCT_AmpCfg(flags, name="SCT_Amp", **kwargs):
    """Return configured amplifier and shaper tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("CrossFactor2sides", 0.1)
    kwargs.setdefault("CrossFactorBack", 0.07)
    kwargs.setdefault("PeakTime", 21)
    kwargs.setdefault("deltaT", 1.0)
    kwargs.setdefault("Tmin", -25.0)
    kwargs.setdefault("Tmax", 150.0)
    acc.setPrivateTools(CompFactory.SCT_Amp(name, **kwargs))
    return acc


def SCT_SurfaceChargesGeneratorCfg(flags, name="SCT_SurfaceChargesGenerator", **kwargs):
    """Return ComponentAccumulator with configured surface charges tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("FixedTime", -999)
    kwargs.setdefault("SubtractTime", -999)
    kwargs.setdefault("SurfaceDriftTime", 10*Units.ns)
    kwargs.setdefault("NumberOfCharges", 1)
    kwargs.setdefault("SmallStepLength", 5*Units.micrometer)
    kwargs.setdefault("DepletionVoltage", 70)
    kwargs.setdefault("BiasVoltage", 150)
    kwargs.setdefault("isOverlay", flags.Common.isOverlay)
    # kwargs.setdefault("doTrapping", True) # ATL-INDET-INT-2016-019
    # experimental SCT_DetailedSurfaceChargesGenerator config dropped here
    SCT_SurfaceChargesGenerator, SCT_RadDamageSummaryTool = CompFactory.getComps("SCT_SurfaceChargesGenerator", "SCT_RadDamageSummaryTool",)
    tool = SCT_SurfaceChargesGenerator(name, **kwargs)
    tool.RadDamageSummaryTool = SCT_RadDamageSummaryTool()
    tool.SiConditionsTool = acc.popToolsAndMerge(SCT_SiliconConditionsCfg(flags))
    tool.SiPropertiesTool = acc.popToolsAndMerge(SCT_SiPropertiesToolCfg(flags, SiConditionsTool=tool.SiConditionsTool))
    tool.LorentzAngleTool = acc.popToolsAndMerge(SCT_LorentzAngleToolCfg(flags))
    acc.setPrivateTools(tool)
    return acc


def SCT_FrontEndCfg(flags, name="SCT_FrontEnd", **kwargs):
    """Return ComponentAccumulator with configured front-end electronics tool"""
    # Setup noise treament in SCT_FrontEnd
    # To set the mean noise values for the different module types
    # Default values set at 0 degrees, plus/minus ~5 enc per plus/minus degree
    kwargs.setdefault("NoiseBarrel", 1500.0)
    kwargs.setdefault("NoiseBarrel3", 1541.0)
    kwargs.setdefault("NoiseInners", 1090.0)
    kwargs.setdefault("NoiseMiddles", 1557.0)
    kwargs.setdefault("NoiseShortMiddles", 940.0)
    kwargs.setdefault("NoiseOuters", 1618.0)
    kwargs.setdefault("NOBarrel", 1.5e-5)
    kwargs.setdefault("NOBarrel3", 2.1e-5)
    kwargs.setdefault("NOInners", 5.0e-9)
    kwargs.setdefault("NOMiddles", 2.7e-5)
    kwargs.setdefault("NOShortMiddles", 2.0e-9)
    kwargs.setdefault("NOOuters", 3.5e-5)
    if not flags.Digitization.DoInnerDetectorNoise:
        log = logging.getLogger("SCT_FrontEndCfg")
        log.info("SCT_Digitization:::: Turned off Noise in SCT_FrontEnd")
        kwargs.setdefault("NoiseOn", False)
        kwargs.setdefault("AnalogueNoiseOn", False)
    else:
        kwargs.setdefault("NoiseOn", True)
        kwargs.setdefault("AnalogueNoiseOn", True)
    # In overlay MC, only analogue noise is on (off for data). Noise hits are not added.
    if flags.Common.isOverlay:
        kwargs["NoiseOn"] = False
        kwargs["AnalogueNoiseOn"] = flags.Input.isMC
    # Use Calibration data from Conditions DB, still for testing purposes only
    kwargs.setdefault("UseCalibData", True)
    # Setup the ReadCalibChip folders and Svc
    acc = SCT_ReadCalibChipDataCfg(flags)
    kwargs.setdefault("SCT_ReadCalibChipDataTool", acc.popPrivateTools())
    # DataCompressionMode: 1 is level mode X1X (default), 2 is edge mode 01X, 3 is any hit mode (1XX|X1X|XX1)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("DataCompressionMode", 3)
    elif flags.Common.isOverlay and flags.Input.isMC:
        kwargs.setdefault("DataCompressionMode", 2)
    elif flags.Beam.BunchSpacing <= 50:
        kwargs.setdefault("DataCompressionMode", 1)
    else:
        kwargs.setdefault("DataCompressionMode", 3)
    # DataReadOutMode: 0 is condensed mode and 1 is expanded mode
    if flags.Common.isOverlay and flags.Input.isMC:
        kwargs.setdefault("DataReadOutMode", 0)
    else:
        kwargs.setdefault("DataReadOutMode", 1)
    kwargs.setdefault("SCT_Amp", acc.popToolsAndMerge(SCT_AmpCfg(flags)))
    acc.setPrivateTools(CompFactory.SCT_FrontEnd(name, **kwargs))
    return acc


def SCT_FrontEndPileupCfg(flags, name="PileupSCT_FrontEnd", **kwargs):
    """Return ComponentAccumulator with pileup-configured front-end electronics tool"""
    kwargs.setdefault("NoiseBarrel", 0.0)
    kwargs.setdefault("NoiseBarrel3", 0.0)
    kwargs.setdefault("NoiseInners", 0.0)
    kwargs.setdefault("NoiseMiddles", 0.0)
    kwargs.setdefault("NoiseShortMiddles", 0.0)
    kwargs.setdefault("NoiseOuters", 0.0)
    kwargs.setdefault("NOBarrel", 0.0)
    kwargs.setdefault("NOBarrel3", 0.0)
    kwargs.setdefault("NOInners", 0.0)
    kwargs.setdefault("NOMiddles", 0.0)
    kwargs.setdefault("NOShortMiddles", 0.0)
    kwargs.setdefault("NOOuters", 0.0)
    kwargs.setdefault("NoiseOn", False)
    return SCT_FrontEndCfg(flags, name, **kwargs)


def SCT_RangeCfg(flags, name="SiliconRange", **kwargs):
    """Return an SCT configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", SCT_FirstXing())
    kwargs.setdefault("LastXing", SCT_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0) # default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["SiHitCollection#SCT_Hits"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)


def SCT_OutputCfg(flags):
    """Return ComponentAccumulator with Output for SCT. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["SCT_RDO_Container#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["InDetSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        if flags.InDet.saveSCTSiHits:
            ItemList += ["SiHitCollection#SCT_Hits"]
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def SCT_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for SCT digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(SCT_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def SCT_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with SCT Overlay digitization"""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["SiHitCollection#SCT_Hits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(SCT_OverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault('Cardinality', flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    SCT_Digitization = CompFactory.SCT_Digitization
    acc.addEventAlgo(SCT_Digitization(name="SCT_OverlayDigitization", **kwargs))
    return acc


# with output defaults
def SCT_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for SCT digitization and Output"""
    acc = SCT_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(SCT_OutputCfg(flags))
    return acc


def SCT_OverlayDigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator with SCT Overlay digitization and Output"""
    acc = SCT_OverlayDigitizationBasicCfg(flags, **kwargs)
    acc.merge(SCT_OutputCfg(flags))
    return acc


# additional specialisations
def SCT_DigitizationHSCfg(flags, name="SCT_DigitizationHS", **kwargs):
    """Return ComponentAccumulator for Hard-Scatter-only SCT digitization and Output"""
    acc = SCT_DigitizationHSToolCfg(flags)
    kwargs["PileUpTools"] = acc.popPrivateTools()
    acc = SCT_DigitizationBasicCfg(flags, name=name, **kwargs)
    acc.merge(SCT_OutputCfg(flags))
    return acc


def SCT_DigitizationPUCfg(flags, name="SCT_DigitizationPU", **kwargs):
    """Return ComponentAccumulator with Pile-up-only SCT digitization and Output"""
    acc = SCT_DigitizationPUToolCfg(flags)
    kwargs["PileUpTools"] = acc.popPrivateTools()
    acc = SCT_DigitizationBasicCfg(flags, name=name, **kwargs)
    acc.merge(SCT_OutputCfg(flags))
    return acc
