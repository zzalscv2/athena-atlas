"""Define methods to construct configured HGTD Digitization tools and algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from Digitization.PileUpMergeSvcConfigNew import PileUpMergeSvcCfg, PileUpXingFolderCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg

# The earliest bunch crossing time for which interactions will be sent
# to the HGTD Digitization code.
def HGTD_FirstXing():
    return -50
# The latest bunch crossing time for which interactions will be sent
# to the HGTD Digitization code.
def HGTD_LastXing():
    return 25
# NOTE: related to 3BC mode?

def HGTD_FrontEndToolCfg(flags, name="HGTD_FrontEndTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HGTD_FrontEndTool(name, **kwargs))
    return acc

def HGTD_SurfaceChargesGeneratorCfg(flags, name="HGTD_SurfaceChargesGenerator", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HGTD_SurfaceChargesGenerator(name, **kwargs))
    return acc

def HGTD_DigitizationBasicToolCfg(flags, name="HGTD_DigitizationBasicTool", **kwargs):
    """Return ComponentAccumulator with configured HGTD_DigitizationTool"""
    acc = HGTD_ReadoutGeometryCfg(flags)
    # set up tool handles
    kwargs.setdefault("FrontEnd", acc.popToolsAndMerge(HGTD_FrontEndToolCfg(flags)))
    kwargs.setdefault("SurfaceChargesGenerator", acc.popToolsAndMerge(HGTD_SurfaceChargesGeneratorCfg(flags)))
    kwargs.setdefault("InputObjectName", "HGTD_Hits")
    kwargs.setdefault("HGTDDetEleCollKey", "HGTD_DetectorElementCollection")
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", HGTD_FirstXing(flags))
        kwargs.setdefault("LastXing", HGTD_LastXing(flags))
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.setPrivateTools(CompFactory.HGTD_DigitizationTool(name, **kwargs))
    return acc

def HGTD_DigitizationToolCfg(flags, name="HGTD_DigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured HGTD_DigitizationBasicTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(HGTD_RangeCfg(flags))]
        kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("MergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputObjectName", flags.Overlay.BkgPrefix + "HGTD_RDOs")
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "HGTD_SDO_Map")
    else:
        kwargs.setdefault("OutputObjectName", "HGTD_RDOs")
        kwargs.setdefault("OutputSDOName", "HGTD_SDO_Map")
    tool = acc.popToolsAndMerge(HGTD_DigitizationBasicToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc

def HGTD_RangeCfg(flags, name="HGTD_Range", **kwargs):
    """Return a configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", HGTD_FirstXing(flags))
    kwargs.setdefault("LastXing", HGTD_LastXing(flags))
    kwargs.setdefault("CacheRefreshFrequency", 1.0) # default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["SiHitCollection#HGTD_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)

def HGTD_OutputCfg(flags):
    """Return ComponentAccumulator with Output for HGTD. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["HGTD_RDOContainer#*"]
        if flags.Digitization.TruthOutput:
            ItemList += ["InDetSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc

def HGTD_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for HGTD digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(HGTD_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc

# with output defaults
def HGTD_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for HGTD digitization and Output"""
    acc = HGTD_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(HGTD_OutputCfg(flags))
    return acc
