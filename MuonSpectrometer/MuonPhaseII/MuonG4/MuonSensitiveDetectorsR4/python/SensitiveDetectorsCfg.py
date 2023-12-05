# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MdtSensitiveDetectorToolCfg(flags, name = "MdtSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawMdtSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::MDTDriftGas"])
    the_tool = CompFactory.MuonG4R4.MdtSensitiveDetectorTool(name, **kwargs)
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="MdtSimHitSorterAlg",
                                               InContainers=["xRawMdtSimHits"],
                                               OutContainer ="xMdtSimHits",
                                               deepCopy = True))
    result.setPrivateTools(the_tool)
    return result

def MmSensitiveDetectorToolCfg(flags, name = "MmSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawMmSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::MicroMegasGas"])
    the_tool = CompFactory.MuonG4R4.MmSensitiveDetectorTool(name, **kwargs)
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="MmSimHitSorterAlg",
                                               InContainers=["xRawMmSimHits"],
                                               OutContainer ="xMmSimHits",
                                               deepCopy = True))
    result.setPrivateTools(the_tool)
    return result

def RpcSensitiveDetectorToolCfg(flags, name = "RpcSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawRpcSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::RpcGasGap"])
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="RpcSimHitSorterAlg",
                                               InContainers=["xRawRpcSimHits"],
                                               OutContainer ="xRpcSimHits",
                                               deepCopy = True))
    the_tool = CompFactory.MuonG4R4.RpcSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def TgcSensitiveDetectorToolCfg(flags, name = "TgcSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawTgcSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::TgcGas"])
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="TgcSimHitSorterAlg",
                                               InContainers=["xRawTgcSimHits"],
                                               OutContainer ="xTgcSimHits",
                                               deepCopy = True))
    the_tool = CompFactory.MuonG4R4.TgcSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def SetupSensitiveDetectorsCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableMDT:
        tools += [result.popToolsAndMerge(MdtSensitiveDetectorToolCfg(flags))]
    
    if flags.Detector.EnableRPC:
        tools += [result.popToolsAndMerge(RpcSensitiveDetectorToolCfg(flags))]

    if flags.Detector.EnableMM:
        tools += [result.popToolsAndMerge(MmSensitiveDetectorToolCfg(flags))]

    if flags.Detector.EnableTGC:
        tools += [result.popToolsAndMerge(TgcSensitiveDetectorToolCfg(flags))]

    result.setPrivateTools(tools)
    return result

def SimHitContainerListCfg(flags):
    simHitContainers = []
    if flags.Detector.EnableMDT:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMdtSimHits")]
    if flags.Detector.EnableMM:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMmSimHits")]
    if flags.Detector.EnableRPC:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawRpcSimHits")]
    if flags.Detector.EnableTGC:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawTgcSimHits")]
    return simHitContainers