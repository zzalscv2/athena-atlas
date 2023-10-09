"""Define methods to construct configured TRT Digitization tools and algorithms

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, ProductionStep
from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
from TRT_PAI_Process.TRT_PAI_ProcessConfig import TRT_PAI_Process_XeToolCfg, TRT_PAI_Process_ArToolCfg, TRT_PAI_Process_KrToolCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the TRT Digitization code
def TRT_FirstXing():
    return -50


def TRT_LastXing():
    return 50


def TRT_RangeCfg(flags, name="TRTRange", **kwargs):
    """Return an TRT configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", TRT_FirstXing())
    kwargs.setdefault("LastXing", TRT_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0) #default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["TRTUncompressedHitCollection#TRTUncompressedHits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def TRT_DigitizationBasicToolCfg(flags, name="TRT_DigitizationBasicTool", **kwargs):
    """Return ComponentAccumulator with common TRT digitization tool config"""
    acc = TRT_ReadoutGeometryCfg(flags)
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))
    PartPropSvc = CompFactory.PartPropSvc
    acc.addService(PartPropSvc(InputFile="PDGTABLE.MeV"))
    kwargs.setdefault("DigVersContainerKey", "")
    # default arguments
    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
    StrawStatusTool = acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags))
    acc.addPublicTool(StrawStatusTool)  # public as it is has many clients to save some memory
    kwargs.setdefault("InDetTRTStrawStatusSummaryTool", StrawStatusTool)
    kwargs.setdefault("PAI_Tool_Ar", TRT_PAI_Process_ArToolCfg(flags))
    kwargs.setdefault("PAI_Tool_Kr", TRT_PAI_Process_KrToolCfg(flags))
    kwargs.setdefault("PAI_Tool_Xe", TRT_PAI_Process_XeToolCfg(flags))
    kwargs.setdefault("Override_TrtRangeCutProperty", flags.Sim.TRTRangeCut)
    kwargs.setdefault("RandomSeedOffset", flags.Digitization.RandomSeedOffset)
    if not flags.Digitization.DoInnerDetectorNoise:
        kwargs.setdefault("Override_noiseInSimhits", 0)
        kwargs.setdefault("Override_noiseInUnhitStraws", 0)
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("PrintDigSettings", True)
        kwargs.setdefault("Override_cosmicFlag", 0)
        kwargs.setdefault("Override_doCosmicTimingPit", 1)
        kwargs.setdefault("Override_jitterTimeOffset", 0.)
        kwargs.setdefault("Override_timeCorrection", 0)
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", TRT_FirstXing())
        kwargs.setdefault("LastXing", TRT_LastXing())
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    TRTDigitizationTool = CompFactory.TRTDigitizationTool
    tool = TRTDigitizationTool(name, **kwargs)
    acc.setPrivateTools(tool)
    return acc


def TRT_DigitizationToolCfg(flags, name="TRTDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured TRT digitization tool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(TRT_RangeCfg(flags))]
        kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("MergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", flags.Overlay.BkgPrefix + "TRT_RDOs")
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "TRT_SDO_Map")
    else:
        kwargs.setdefault("OutputObjectName", "TRT_RDOs")
        kwargs.setdefault("OutputSDOName", "TRT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    tool = acc.popToolsAndMerge(TRT_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def TRT_DigitizationGeantinoTruthToolCfg(flags, name="TRT_GeantinoTruthDigitizationTool", **kwargs):
    """Return ComponentAccumulator with Geantino configured TRT digitization tool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(TRT_RangeCfg(flags))]
        kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("MergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("VetoPileUpTruthLinks", False)
    tool = acc.popToolsAndMerge(TRT_DigitizationToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def TRT_DigitizationHSToolCfg(flags, name="TRT_DigitizationToolHS", **kwargs):
    """Return ComponentAccumulator with Hard Scatter configured TRT digitization tool"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(TRT_RangeCfg(flags))
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("OutputObjectName", "TRT_RDOs")
    kwargs.setdefault("OutputSDOName", "TRT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 1)
    tool = acc.popToolsAndMerge(TRT_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def TRT_DigitizationPUToolCfg(flags, name="TRT_DigitizationToolPU", **kwargs):
    """Return ComponentAccumulator with Pile Up configured TRT digitization tool"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(TRT_RangeCfg(flags))
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("OutputObjectName", "TRT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "TRT_PU_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    acc.merge(TRT_DigitizationBasicToolCfg(flags, name, **kwargs))
    return acc


def TRT_DigitizationSplitNoMergePUToolCfg(flags, name="TRT_DigitizationToolSplitNoMergePU", **kwargs):
    """Return ComponentAccumulator with PileUpTRT_Hits configured TRT digitization tool"""
    acc = ComponentAccumulator()
    rangetool = acc.popToolsAndMerge(TRT_RangeCfg(flags))
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=rangetool)).name)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("DataObjectName", "PileupTRTUncompressedHits")
    kwargs.setdefault("OutputObjectName", "TRT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "TRT_PU_SDO_Map")
    kwargs.setdefault("Override_noiseInSimhits", 0)
    kwargs.setdefault("Override_noiseInUnhitStraws", 0)
    tool = acc.popToolsAndMerge(TRT_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def TRT_OverlayDigitizationToolCfg(flags, name="TRT_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured Overlay TRT digitization tool"""
    acc = ComponentAccumulator()
    if flags.Overlay.DataOverlay:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        # TODO: remove the hardcode
        acc.merge(addFolders(flags, "/TRT/Cond/DigVers", "TRT_OFL", tag="TRTCondDigVers-Collisions-01", db="OFLP200", className="AthenaAttributeList"))
        kwargs.setdefault("DigVersContainerKey", "/TRT/Cond/DigVers")

    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "TRT_RDOs")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "TRT_SDO_Map")
    kwargs.setdefault("Override_isOverlay", 1)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("Override_getT0FromData", 0)
    kwargs.setdefault("Override_noiseInSimhits", 0)
    kwargs.setdefault("Override_noiseInUnhitStraws", 0)
    kwargs.setdefault("MergeSvc", '')
    tool = acc.popToolsAndMerge(TRT_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def TRT_OutputCfg(flags):
    """Return ComponentAccumulator with Output for TRT. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["TRT_RDO_Container#*"]
        if flags.Digitization.EnableTruth:
            ItemList += ["InDetSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def TRT_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for TRT digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(TRT_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def TRT_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with TRT Overlay digitization"""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["TRTUncompressedHitCollection#TRTUncompressedHits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(TRT_OverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    TRTDigitization = CompFactory.TRTDigitization
    acc.addEventAlgo(TRTDigitization(name="TRT_OverlayDigitization", **kwargs))
    return acc


# with output defaults
def TRT_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for TRT digitization and Output"""
    acc = TRT_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(TRT_OutputCfg(flags))
    return acc


def TRT_OverlayDigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator with TRT Overlay digitization and Output"""
    acc = TRT_OverlayDigitizationBasicCfg(flags, **kwargs)
    acc.merge(TRT_OutputCfg(flags))
    return acc


# additional specialisations
def TRT_DigitizationHSCfg(flags, name="TRT_DigitizationHS", **kwargs):
    """Return ComponentAccumulator for Hard-Scatter-only TRT digitization and Output"""
    acc = TRT_DigitizationHSToolCfg(flags)
    kwargs["PileUpTools"] = acc.popPrivateTools()
    acc = TRT_DigitizationBasicCfg(flags, name=name, **kwargs)
    acc.merge(TRT_OutputCfg(flags))
    return acc


def TRT_DigitizationPUCfg(flags, name="TRT_DigitizationPU", **kwargs):
    """Return ComponentAccumulator with Pile-up-only TRT digitization and Output"""
    acc = TRT_DigitizationPUToolCfg(flags)
    kwargs["PileUpTools"] = acc.popPrivateTools()
    acc = TRT_DigitizationBasicCfg(flags, name=name, **kwargs)
    acc.merge(TRT_OutputCfg(flags))
    return acc
