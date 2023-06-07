"""Define methods to construct configured RPC Digitization tools and algorithms

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
from MuonConfig.MuonByteStreamCnvTestConfig import RpcDigitToRpcRDOCfg
from MuonConfig.MuonByteStreamCnvTestConfig import NrpcDigitToNrpcRDOCfg
from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
from Digitization.TruthDigitizationOutputConfig import TruthDigitizationOutputCfg
from Digitization.PileUpToolsConfig import PileUpToolsCfg
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest and last bunch crossing times for which interactions will be sent
# to the RpcDigitizationTool.
def RPC_FirstXing():
    return -150


def RPC_LastXing():
    return 125


def RPC_RangeCfg(flags, name="RPC_Range", **kwargs):
    """Return a PileUpXingFolder tool configured for RPC"""
    kwargs.setdefault("FirstXing", RPC_FirstXing())
    kwargs.setdefault("LastXing", RPC_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    kwargs.setdefault("ItemList", ["RPCSimHitCollection#RPC_Hits"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def RPC_DigitizationToolCommonCfg(flags, name="RpcDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured RpcDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.GeoModel.Run < LHCPeriod.Run3:  # Run 3 and later currently do not use conditions
        from MuonConfig.MuonCondAlgConfig import RpcCondDbAlgCfg
        acc.merge(RpcCondDbAlgCfg(flags))
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", RPC_FirstXing())
        kwargs.setdefault("LastXing", RPC_LastXing())
    kwargs.setdefault("OutputObjectName", "RPC_DIGITS")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "RPC_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "RPC_SDO")
    # config
    kwargs.setdefault("DeadTime", 100)
    kwargs.setdefault("PatchForRpcTime", True)
    # kwargs.setdefault("PatchForRpcTimeShift", 9.6875)
    kwargs.setdefault("PatchForRpcTimeShift", 12.5)
    kwargs.setdefault("turnON_efficiency", True)
    kwargs.setdefault("turnON_clustersize", True)
    kwargs.setdefault("testbeam_clustersize", 0)
    kwargs.setdefault("ClusterSize1_2uncorr", False)
    kwargs.setdefault("CutProjectedTracks", 100)
    kwargs.setdefault("RPCInfoFromDb", True)
    kwargs.setdefault("Efficiency_fromCOOL", True)
    kwargs.setdefault("EfficiencyPatchForBMShighEta", False)
    kwargs.setdefault("ClusterSize_fromCOOL", True)
    kwargs.setdefault("DumpFromDbFirst", False)
    kwargs.setdefault("PanelId_OFF_fromlist", False)
    kwargs.setdefault("PanelId_OK_fromlist", False)
    kwargs.setdefault("IgnoreRunDependentConfig", False)
    kwargs.setdefault("PrintCalibrationVector",False )
    kwargs.setdefault("PhiAndEtaEff_A",[0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938])
    kwargs.setdefault("OnlyPhiEff_A"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("OnlyEtaEff_A"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("PhiAndEtaEff_C",[0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938, 0.938])
    kwargs.setdefault("OnlyPhiEff_C"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("OnlyEtaEff_C"  ,[0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022, 0.022])
    kwargs.setdefault("FracClusterSize1_A",   [0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664])
    kwargs.setdefault("FracClusterSize2_A",   [0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986])
    kwargs.setdefault("FracClusterSizeTail_A",[0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035, 0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035 ])
    kwargs.setdefault("MeanClusterSizeTail_A",[0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598])
    kwargs.setdefault("FracClusterSize1_C",   [0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664, 0.609664])
    kwargs.setdefault("FracClusterSize2_C",   [0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986, 0.259986])
    kwargs.setdefault("FracClusterSizeTail_C",[0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035, 0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035,  0.13035 ])
    kwargs.setdefault("MeanClusterSizeTail_C",[0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598, 0.548598])
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    RpcDigitizationTool = CompFactory.RpcDigitizationTool
    acc.setPrivateTools(RpcDigitizationTool(name, **kwargs))
    return acc


def RPC_DigitizationToolCfg(flags, name="RpcDigitizationTool", **kwargs):
    """Return ComponentAccumulator with configured RpcDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.PileUp:
        intervals = []
        if not flags.Digitization.DoXingByXingPileUp:
            intervals += [acc.popToolsAndMerge(RPC_RangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    kwargs.setdefault("OutputObjectName", "RPC_DIGITS")
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("OutputSDOName", flags.Overlay.BkgPrefix + "RPC_SDO")
    else:
        kwargs.setdefault("OutputSDOName", "RPC_SDO")
    tool = acc.popToolsAndMerge(RPC_DigitizationToolCommonCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def RPC_OverlayDigitizationToolCfg(flags, name="Rpc_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with RpcDigitizationTool configured for Overlay"""
    kwargs.setdefault("OnlyUseContainerName", False)
    kwargs.setdefault("OutputObjectName", flags.Overlay.SigPrefix + "RPC_DIGITS")
    kwargs.setdefault("OutputSDOName", flags.Overlay.SigPrefix + "RPC_SDO")
    kwargs.setdefault("PileUpMergeSvc", '')
    return RPC_DigitizationToolCommonCfg(flags, name, **kwargs)


def RPC_OutputCfg(flags):
    """Return ComponentAccumulator with Output for RPC. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["RpcPadContainer#*"]
        if flags.Muon.enableNRPC:
            ItemList += [ 'xAOD::NRPCRDOContainer#NRPCRDO' , 'xAOD::NRPCRDOAuxContainer#NRPCRDOAux.' ]
        if flags.Digitization.EnableTruth:
            ItemList += ["MuonSimDataCollection#*"]
            acc.merge(TruthDigitizationOutputCfg(flags))
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList))
    return acc


def RPC_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for RPC digitization"""
    acc = MuonGeoModelCfg(flags)
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(RPC_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def RPC_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with RPC Overlay digitization"""
    acc = MuonGeoModelCfg(flags, forceDisableAlignment=not flags.Overlay.DataOverlay)

    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["RPCSimHitCollection#RPC_Hits"]))

    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(RPC_OverlayDigitizationToolCfg(flags))
        kwargs["DigitizationTool"] = tool

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    RPC_Digitizer = CompFactory.RPC_Digitizer
    acc.addEventAlgo(RPC_Digitizer(name="RPC_OverlayDigitizer", **kwargs))
    return acc


# with output defaults
def RPC_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for RPC digitization and Output"""
    acc = RPC_DigitizationBasicCfg(flags, **kwargs)
    acc.merge(RPC_OutputCfg(flags))
    return acc


def RPC_DigitizationDigitToRDOCfg(flags):
    """Return ComponentAccumulator with RPC digitization and Digit to RPCPAD RDO"""
    acc = RPC_DigitizationCfg(flags)
    acc.merge(RPCCablingConfigCfg(flags))
    acc.merge(RpcDigitToRpcRDOCfg(flags))
    if flags.Muon.enableNRPC:
        acc.merge(NrpcDigitToNrpcRDOCfg(flags))
    return acc

