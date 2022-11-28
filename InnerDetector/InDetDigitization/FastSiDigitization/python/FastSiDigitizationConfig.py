# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest bunch crossing time for which interactions will be sent
# to the Fast Pixel Digitization code.
def FastPixel_FirstXing():
    return 0


# The latest bunch crossing time for which interactions will be sent
# to the Fast Pixel Digitization code.
def FastPixel_LastXing():
    return 0


# The earliest bunch crossing time for which interactions will be sent
# to the Fast SCT Digitization code.
def FastSCT_FirstXing():
    return 0


# The latest bunch crossing time for which interactions will be sent
# to the Fast SCT Digitization code.
def FastSCT_LastXing():
    return 0


# TODO This is very similar to ClusterMakerToolCfg in InDetConfig.SiClusterizationToolConfig - consider merging?
def FastClusterMakerToolCfg(flags, name="FastClusterMakerTool", **kwargs) :
    acc = ComponentAccumulator()
    
    # This directly needs the following Conditions data:
    # PixelChargeCalibCondData & PixelOfflineCalibData
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelChargeLUTCalibCondAlgCfg, PixelChargeCalibCondAlgCfg, PixelOfflineCalibCondAlgCfg
    if 'SCT' in flags.Digitization.DoFastDigi and 'Pixel' not in flags.Digitization.DoFastDigi:
        if flags.GeoModel.Run is LHCPeriod.Run3:
            acc.merge(PixelChargeLUTCalibCondAlgCfg(flags, ReadKey=""))
        else:
            acc.merge(PixelChargeCalibCondAlgCfg(flags, ReadKey=""))
    else:
        if flags.GeoModel.Run is LHCPeriod.Run3:
            acc.merge(PixelChargeLUTCalibCondAlgCfg(flags))
        else:
            acc.merge(PixelChargeCalibCondAlgCfg(flags))
    acc.merge(PixelOfflineCalibCondAlgCfg(flags))

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import PixelReadoutManagerCfg
    acc.merge(PixelReadoutManagerCfg(flags))

    from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
    PixelLorentzAngleTool = acc.popToolsAndMerge(PixelLorentzAngleToolCfg(flags))
    from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
    SCTLorentzAngleTool = acc.popToolsAndMerge( SCT_LorentzAngleToolCfg(flags) )

    kwargs.setdefault("PixelLorentzAngleTool", PixelLorentzAngleTool)
    kwargs.setdefault("SCTLorentzAngleTool", SCTLorentzAngleTool)

    InDetClusterMakerTool = CompFactory.InDet.ClusterMakerTool(name, **kwargs)
    acc.setPrivateTools(InDetClusterMakerTool)
    return acc


def commonPixelFastDigitizationCfg(flags, name,**kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterMaker", acc.popToolsAndMerge(FastClusterMakerToolCfg(flags)))

    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    kwargs.setdefault("RndmEngine", "FastPixelDigitization")

    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", FastPixel_FirstXing())
        kwargs.setdefault("LastXing",  FastPixel_LastXing() )

    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelConfigCondAlgCfg
    acc.merge(PixelConfigCondAlgCfg(flags, name="PixelConfigCondAlg", ReadDeadMapKey = ""))

    from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(PixelLorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.PixelFastDigitizationTool(name,**kwargs))
    return acc


def commonSCT_FastDigitizationCfg(flags, name,**kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("ClusterMaker", acc.popToolsAndMerge(FastClusterMakerToolCfg(flags)))

    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    kwargs.setdefault("RndmEngine", "FastSCT_Digitization")

    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", FastSCT_FirstXing())
        kwargs.setdefault("LastXing",  FastSCT_LastXing() )

    from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(SCT_LorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.SCT_FastDigitizationTool(name,**kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolCfg(flags, name="PixelFastDigitizationTool", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolHSCfg(flags, name="PixelFastDigitizationToolHS", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("HardScatterSplittingMode", 1)
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolPUCfg(flags, name="PixelFastDigitizationToolPU", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("PixelClusterContainerName", "Pixel_PU_Clusters")
    kwargs.setdefault("TruthNamePixel", "PRD_MultiTruthPixel_PU")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolSplitNoMergePUCfg(flags, name="PixelFastDigitizationToolSplitNoMergePU", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("InputObjectName", "PileupPixelHits")
    kwargs.setdefault("PixelClusterContainerName", "PixelFast_PU_Clusters")
    kwargs.setdefault("TruthNamePixel", "PRD_MultiTruthPixel_PU")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("PixelClusterAmbiguitiesMapName", "PixelClusterAmbiguitiesMapPU")
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def SCT_FastDigitizationToolCfg(flags, name="SCT_FastDigitizationTool", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc
    return commonSCT_FastDigitizationCfg(flags, name,**kwargs)


######################################################################################
def SCT_FastDigitizationToolHSCfg(flags, name="SCT_FastDigitizationToolHS",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("HardScatterSplittingMode", 1)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def SCT_FastDigitizationToolPUCfg(flags, name="SCT_FastDigitizationToolPU",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("SCT_ClusterContainerName", "SCT_PU_Clusters")
    kwargs.setdefault("TruthNameSCT", "PRD_MultiTruthSCT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def SCT_FastDigitizationToolSplitNoMergePUCfg(flags, name="SCT_FastDigitizationToolSplitNoMergePU",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("InputObjectName", "PileupSCT_Hits")
    kwargs.setdefault("SCT_ClusterContainerName", "SCT_PU_Clusters")
    kwargs.setdefault("TruthNameSCT", "PRD_MultiTruthSCT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def FastPixelRangeCfg(flags, name="FastPixelRange" , **kwargs):
    kwargs.setdefault('FirstXing', FastPixel_FirstXing() )
    kwargs.setdefault('LastXing',  FastPixel_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["SiHitCollection#PixelHits"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)


######################################################################################
def FastSCTRangeCfg(flags, name="FastSCTRange" , **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', FastSCT_FirstXing() )
    kwargs.setdefault('LastXing',  FastSCT_LastXing()  )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["SiHitCollection#SCT_Hits"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)
