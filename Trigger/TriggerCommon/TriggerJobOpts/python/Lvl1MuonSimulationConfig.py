# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from IOVDbSvc.IOVDbSvcConfig import addFolders

def TMDBConfig(flags):
    acc = ComponentAccumulator()

    # Read MuRcvRawChCnt from the input file (for POOL directly, for BS via converter)
    if flags.Input.Format is Format.POOL:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["TileRawChannelContainer/MuRcvRawChCnt"]))
    else:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags, ["TileRawChannelContainer/MuRcvRawChCnt"]))

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    tmdbAlg = CompFactory.TileMuonReceiverDecision('TileMuonReceiverDecision'
                                                   , TileRawChannelContainer = "MuRcvRawChCnt" # input
                                                   , TileMuonReceiverContainer = "rerunTileMuRcvCnt" # output
                                                   , ManualRunPeriod = 2 # forcing Run 2 format (=2) for now, until TGC implements Run 3 format (=3)
                                                   # run 2 thresholds
                                                   , MuonReceiverEneThreshCellD6Low = 500
                                                   , MuonReceiverEneThreshCellD6andD5Low = 500
                                                   , MuonReceiverEneThreshCellD6High = 600
                                                   , MuonReceiverEneThreshCellD6andD5High = 600
                                                   # run 3 thresholds
                                                   , MuonReceiverEneThreshCellD5 = 500
                                                   , MuonReceiverEneThreshCellD6 = 500
                                                   , MuonReceiverEneThreshCellD5andD6 = 500)
    acc.addEventAlgo(tmdbAlg)
    return acc

def MuonBytestream2RdoConfig(flags):
    acc = ComponentAccumulator()
    if flags.Input.isMC:
        return acc

    postFix = "_L1MuonSim"
    from MuonConfig.MuonBytestreamDecodeConfig import MuonCacheNames
    cacheCreator = CompFactory.MuonCacheCreator(RpcCacheKey = MuonCacheNames.RpcCache,
                                                TgcCacheKey = MuonCacheNames.TgcCache,
                                                MdtCsmCacheKey = MuonCacheNames.MdtCsmCache,
                                                CscCacheKey = (MuonCacheNames.CscCache if flags.Detector.GeometryCSC else ""))
    acc.addEventAlgo(cacheCreator)
    # for RPC
    RPCRodDecoder = CompFactory.Muon.RpcROD_Decoder(name = "RpcROD_Decoder" + postFix)
    MuonRpcRawDataProviderTool = CompFactory.Muon.RPC_RawDataProviderToolMT(name = "RPC_RawDataProviderToolMT" + postFix,
                                                                             RpcContainerCacheKey = MuonCacheNames.RpcCache,
                                                                             WriteOutRpcSectorLogic = False,
                                                                             Decoder = RPCRodDecoder,
                                                                             RdoLocation = "RPCPAD_L1" )
    RpcRawDataProvider = CompFactory.Muon.RpcRawDataProvider(name = "RpcRawDataProvider" + postFix,
                                                              ProviderTool = MuonRpcRawDataProviderTool)
    acc.addEventAlgo(RpcRawDataProvider)
    # for TGC
    TGCRodDecoder = CompFactory.Muon.TGC_RodDecoderReadout(name = "TGC_RodDecoderReadout" + postFix)
    MuonTgcRawDataProviderTool = CompFactory.Muon.TGC_RawDataProviderToolMT(name = "TGC_RawDataProviderToolMT" + postFix,
                                                                             TgcContainerCacheKey = MuonCacheNames.TgcCache,
                                                                             Decoder = TGCRodDecoder,
                                                                             RdoLocation = "TGCRDO_L1")
    TgcRawDataProvider = CompFactory.Muon.TgcRawDataProvider(name = "TgcRawDataProvider" + postFix,
                                                              ProviderTool = MuonTgcRawDataProviderTool)
    acc.addEventAlgo(TgcRawDataProvider)
    # for sTGC
    if flags.Detector.GeometrysTGC:
        Muon__STGC_RawDataProviderToolMT=CompFactory.Muon.STGC_RawDataProviderToolMT
        from MuonConfig.MuonBytestreamDecodeConfig import sTgcRODDecoderCfg
        MuonsTgcRawDataProviderTool = Muon__STGC_RawDataProviderToolMT(name    = "STGC_RawDataProviderToolMT"+postFix,
                                                                       Decoder = acc.popToolsAndMerge(sTgcRODDecoderCfg(flags,
                                                                                                     name = "sTgcROD_Decoder"+postFix)),
                                                                       RdoLocation = "sTGCRDO_L1")
        Muon__sTgcRawDataProvider=CompFactory.Muon.sTgcRawDataProvider
        sTgcRawDataProvider = Muon__sTgcRawDataProvider(name       = "sTgcRawDataProvider"+postFix,
                                                        ProviderTool = MuonsTgcRawDataProviderTool )
        acc.addEventAlgo(sTgcRawDataProvider)

    # for MM
    if flags.Detector.GeometryMM:
        from MuonConfig.MuonBytestreamDecodeConfig import MmRDODDecoderCfg
        Muon_MM_RawDataProviderToolMT = CompFactory.Muon.MM_RawDataProviderToolMT
        MuonMmRawDataProviderTool = Muon_MM_RawDataProviderToolMT(name  = "MM_RawDataProviderToolMT"+postFix,
                                                                  Decoder = acc.popToolsAndMerge(MmRDODDecoderCfg(flags, 
                                                                                                 name="MM_RODDecoder"+postFix)),
                                                                  RdoLocation = "MMRDO_L1")
        Muon__MmRawDataProvider = CompFactory.Muon.MM_RawDataProvider
        MmRawDataProvider = Muon__MmRawDataProvider(name = "MmRawDataProvider"+postFix, ProviderTool = MuonMmRawDataProviderTool )
        acc.addEventAlgo(MmRawDataProvider)

    if flags.Trigger.L1MuonSim.EmulateNSW and flags.Trigger.L1MuonSim.NSWVetoMode:
        # for MDT
        MDTRodDecoder = CompFactory.MdtROD_Decoder(name = "MdtROD_Decoder" + postFix)
        MuonMdtRawDataProviderTool = CompFactory.Muon.MDT_RawDataProviderToolMT(name = "MDT_RawDataProviderToolMT" + postFix,
                                                                                CsmContainerCacheKey = MuonCacheNames.MdtCsmCache,
                                                                                Decoder = MDTRodDecoder,
                                                                                RdoLocation = "MDTCSM_L1")
        MdtRawDataProvider = CompFactory.Muon.MdtRawDataProvider(name = "MdtRawDataProvider" + postFix,
                                                                 ProviderTool = MuonMdtRawDataProviderTool)
        acc.addEventAlgo(MdtRawDataProvider)
        # for CSC
        if flags.Detector.GeometryCSC:
            CSCRodDecoder = CompFactory.Muon.CscROD_Decoder(name = "CscROD_Decoder" + postFix,
                                                            IsCosmics = False,
                                                            IsOldCosmics = False )
            MuonCscRawDataProviderTool = CompFactory.Muon.CSC_RawDataProviderToolMT(name = "CSC_RawDataProviderToolMT" + postFix,
                                                                                    CscContainerCacheKey = MuonCacheNames.CscCache,
                                                                                    Decoder = CSCRodDecoder,
                                                                                    RdoLocation = "CSCRDO_L1" )
            CscRawDataProvider = CompFactory.Muon.CscRawDataProvider(name = "CscRawDataProvider" + postFix,
                                                                     ProviderTool = MuonCscRawDataProviderTool)
            acc.addEventAlgo(CscRawDataProvider)

    return acc

def MuonRdo2PrdConfig(flags):
    acc = ComponentAccumulator()
    if not flags.Trigger.L1MuonSim.EmulateNSW or not flags.Trigger.L1MuonSim.NSWVetoMode:
        return acc
    postFix = "_L1MuonSim"
    suffix = "" if flags.Input.isMC else "_L1"
    if flags.Input.Format is Format.POOL:
        rdoInputs = [
            ('RpcPadContainer','RPCPAD'),
            ('TgcRdoContainer','TGCRDO'),
            ('CscRawDataContainer','CSCRDO'),
            ('MdtCsmContainer','MDTCSM')
        ]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, Load=rdoInputs))
    ### CSC RDO data ###
    if flags.Detector.GeometryCSC:
        CscRdoToCscPrepDataTool = CompFactory.Muon.CscRdoToCscPrepDataToolMT(name = "CscRdoToCscPrepDataToolMT" + postFix, RDOContainer = "CSCRDO"+suffix)
        CscRdoToCscPrepData = CompFactory.CscRdoToCscPrepData(name = "CscRdoToCscPrepData" + postFix,
                                                              CscRdoToCscPrepDataTool = CscRdoToCscPrepDataTool)
        acc.addEventAlgo(CscRdoToCscPrepData)
        CscClusterBuilderTool = CompFactory.CscThresholdClusterBuilderTool(name = "CscThresholdClusterBuilderTool" + postFix)
        CscClusterBuilder = CompFactory.CscThresholdClusterBuilder(name = "CscThresholdClusterBuilder"+postFix,
                                                                   cluster_builder = CscClusterBuilderTool)
        acc.addEventAlgo(CscClusterBuilder)
    ### MDT RDO data ###
    MdtRdoToMdtPrepDataTool = CompFactory.Muon.MdtRdoToPrepDataToolMT(name = "MdtRdoToPrepDataToolMT" + postFix, RDOContainer = "MDTCSM"+suffix)
    MdtRdoToMdtPrepData = CompFactory.MdtRdoToMdtPrepData(name = "MdtRdoToMdtPrepData" + postFix,
                                                          DecodingTool = MdtRdoToMdtPrepDataTool)
    acc.addEventAlgo(MdtRdoToMdtPrepData)
    ### RPC RDO data ###
    RpcRdoToRpcPrepDataTool = CompFactory.Muon.RpcRdoToPrepDataToolMT(name = "RpcRdoToPrepDataToolMT" + postFix, OutputCollection = "RPCPAD"+suffix)
    RpcRdoToRpcPrepData = CompFactory.RpcRdoToRpcPrepData(name = "RpcRdoToRpcPrepData" + postFix,
                                                          DecodingTool = RpcRdoToRpcPrepDataTool)
    acc.addEventAlgo(RpcRdoToRpcPrepData)
    ### TGC RDO data ###
    TgcRdoToTgcPrepDataTool = CompFactory.Muon.TgcRdoToPrepDataToolMT(name = "TgcRdoToPrepDataToolMT" + postFix, RDOContainer = "TGCRDO"+suffix)
    TgcRdoToTgcPrepData = CompFactory.TgcRdoToTgcPrepData(name = "TgcRdoToTgcPrepData" + postFix,
                                                          DecodingTool = TgcRdoToTgcPrepDataTool)
    acc.addEventAlgo(TgcRdoToTgcPrepData)
    return acc

def RecoMuonSegmentSequence(flags):
    acc = ComponentAccumulator()
    if not flags.Trigger.L1MuonSim.EmulateNSW or not flags.Trigger.L1MuonSim.NSWVetoMode:
        return acc
    postFix = "_L1MuonSim"
    theMuonLayerHough = CompFactory.MuonLayerHoughAlg("MuonLayerHoughAlg" + postFix,
                                                      TgcPrepDataContainer = "TGC_Measurements",
                                                      RpcPrepDataContainer = "RPC_Measurements",
                                                      CscPrepDataContainer = ("CSC_Clusters" if flags.Detector.GeometryCSC else ""),
                                                      MdtPrepDataContainer = "MDT_DriftCircles",
                                                      sTgcPrepDataContainer = ("STGC_Measurements" if flags.Detector.GeometrysTGC else ""),
                                                      MMPrepDataContainer = ("MM_Measurements" if flags.Detector.GeometryMM else "") )
    acc.addEventAlgo(theMuonLayerHough)
    theSegmentFinderAlg = CompFactory.MuonSegmentFinderAlg("MuonSegmentFinderAlg" + postFix)
    if not flags.Detector.GeometryCSC:
        theSegmentFinderAlg.CSC_clusterkey = ""
        theSegmentFinderAlg.Csc2dSegmentMaker = ""
        theSegmentFinderAlg.Csc4dSegmentMaker = ""
    acc.addEventAlgo(theSegmentFinderAlg)
    xAODMuonSegmentCnv = CompFactory.xAODMaker.MuonSegmentCnvAlg("MuonSegmentCnvAlg" + postFix)
    acc.addEventAlgo(xAODMuonSegmentCnv)
    return acc



def MuonRdoToMuonDigitToolCfg(flags, name="MuonRdoToMuonDigitTool", **kwargs ):
    result = ComponentAccumulator()
    kwargs.setdefault("DecodeSTGC_RDO", flags.Detector.GeometrysTGC)
    kwargs.setdefault("DecodeMM_RDO", flags.Detector.GeometryMM)
    kwargs.setdefault("DecodeNrpcRDO", flags.Muon.enableNRPC)
    from MuonConfig.MuonByteStreamCnvTestConfig import STgcRdoDecoderCfg, MMRdoDecoderCfg, MdtRdoDecoderCfg
    kwargs.setdefault( "stgcRdoDecoderTool", result.popToolsAndMerge(STgcRdoDecoderCfg(flags))
                         if flags.Detector.GeometrysTGC else "" )
    kwargs.setdefault("mmRdoDecoderTool", result.popToolsAndMerge(MMRdoDecoderCfg(flags))
                         if flags.Detector.GeometryMM else "" )
    kwargs.setdefault("mdtRdoDecoderTool", result.popToolsAndMerge(MdtRdoDecoderCfg(flags)))
    the_tool = CompFactory.MuonRdoToMuonDigitTool (name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonRdo2DigitConfig(flags):
    acc = ComponentAccumulator()

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge( AtlasFieldCacheCondAlgCfg(flags) )
    # Read RPCPAD and TGCRDO from the input POOL file (for BS it comes from [Rpc|Tgc]RawDataProvider)
    suffix = "" if flags.Input.Format is Format.POOL else "_L1"
    RPCRdoName = "RPCPAD"+suffix
    TGCRdoName = "TGCRDO"+suffix
    MMRdoName = "MMRDO"+suffix
    sTGCRdoName = "sTGCRDO"+suffix
    if flags.Input.Format is Format.POOL:
        rdoInputs = [
            ('RpcPadContainer','RPCPAD'),
            ('TgcRdoContainer','TGCRDO')
        ]
        # Read MMRDO and sTGCRDO
        if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
            rdoInputs += [
                ('Muon::MM_RawDataContainer','MMRDO'),
                ('Muon::STGC_RawDataContainer','sTGCRDO')
            ]
        if flags.Muon.enableNRPC:
            rdoInputs += [
                ('xAOD::NRPCRDOContainer' , 'NRPCRDO'),
                ('xAOD::NRPCRDOAuxContainer',  'NRPCRDOAux.')
            ]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, Load=rdoInputs))


    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags))
    MuonRdoToMuonDigitTool = acc.popToolsAndMerge(MuonRdoToMuonDigitToolCfg(flags,DecodeRpcRDO = True,
                                                                 DecodeTgcRDO = True,
                                                                 DecodeCscRDO = False,
                                                                 DecodeMdtRDO = False,                                                                
                                                                 RpcRdoContainer = RPCRdoName,
                                                                 TgcRdoContainer = TGCRdoName,
                                                                 sTgcRdoContainer = sTGCRdoName,
                                                                 MmRdoContainer = MMRdoName,
                                                                 MmDigitContainer = "MM_DIGITS_L1",
                                                                 sTgcDigitContainer = "sTGC_DIGITS_L1",
                                                                 RpcDigitContainer = "RPC_DIGITS_L1",
                                                                 TgcDigitContainer = "TGC_DIGITS_L1"))



    acc.addPublicTool(MuonRdoToMuonDigitTool)
    rdo2digit = CompFactory.MuonRdoToMuonDigit( "MuonRdoToMuonDigit",
                                                MuonRdoToMuonDigitTool = MuonRdoToMuonDigitTool)
    acc.addEventAlgo(rdo2digit)
    return acc

def NSWTriggerConfig(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometrysTGC and not flags.Detector.GeometryMM:
        return acc

    if flags.Input.Format is Format.POOL and flags.Input.isMC:
        rdoInputs = [
            ('McEventCollection','TruthEvent'), # for MM trigger
            ('TrackRecordCollection','MuonEntryLayer'), # for MM trigger
            ('MuonSimDataCollection','sTGC_SDO') # for sTGC Pad trigger
        ]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, Load=rdoInputs))

    PadTdsTool = CompFactory.NSWL1.PadTdsOfflineTool("NSWL1__PadTdsOfflineTool",DoNtuple=flags.Trigger.L1MuonSim.WritesTGCBranches, IsMC = flags.Input.isMC, sTGC_DigitContainerName="sTGC_DIGITS_L1")
    PadTriggerLogicTool = CompFactory.NSWL1.PadTriggerLogicOfflineTool("NSWL1__PadTriggerLogicOfflineTool",DoNtuple=flags.Trigger.L1MuonSim.WritesTGCBranches)
    PadTriggerLookupTool = CompFactory.NSWL1.PadTriggerLookupTool("NSWL1__PadTriggerLookupTool")
    StripTdsTool = CompFactory.NSWL1.StripTdsOfflineTool("NSWL1__StripTdsOfflineTool",DoNtuple=flags.Trigger.L1MuonSim.WritesTGCBranches,IsMC=flags.Input.isMC,sTGC_DigitContainerName="sTGC_DIGITS_L1")
    StripClusterTool = CompFactory.NSWL1.StripClusterTool("NSWL1__StripClusterTool",DoNtuple=flags.Trigger.L1MuonSim.WritesTGCBranches,IsMC=flags.Input.isMC)
    StripSegmentTool = CompFactory.NSWL1.StripSegmentTool("NSWL1__StripSegmentTool",DoNtuple=flags.Trigger.L1MuonSim.WritesTGCBranches)
    MMStripTdsTool = CompFactory.NSWL1.MMStripTdsOfflineTool("NSWL1__MMStripTdsOfflineTool",DoNtuple=False)
    MMTriggerTool = CompFactory.NSWL1.MMTriggerTool("NSWL1__MMTriggerTool",DoNtuple=flags.Trigger.L1MuonSim.WriteMMBranches, IsMC = flags.Input.isMC, MmDigitContainer="MM_DIGITS_L1")
    TriggerProcessorTool = CompFactory.NSWL1.TriggerProcessorTool("NSWL1__TriggerProcessorTool")

    dosTGC =  flags.Trigger.L1MuonSim.doPadTrigger or flags.Trigger.L1MuonSim.doStripTrigger
    if dosTGC:
        from RegionSelector.RegSelToolConfig import regSelTool_STGC_Cfg
        stgcRegSel = acc.popToolsAndMerge(regSelTool_STGC_Cfg( flags ))  # noqa: F841 (adds a conditions algo as a side-effect)

    nswAlg = CompFactory.NSWL1.NSWL1Simulation("NSWL1Simulation",
                                               UseLookup = False,
                                               DoNtuple = flags.Trigger.L1MuonSim.WriteNSWDebugNtuple,
                                               DoMM = flags.Trigger.L1MuonSim.doMMTrigger,
                                               DoMMDiamonds = flags.Trigger.L1MuonSim.doMMTrigger,
                                               DosTGC = dosTGC,
                                               DoPad = flags.Trigger.L1MuonSim.doPadTrigger,
                                               DoStrip = flags.Trigger.L1MuonSim.doStripTrigger,
                                               PadTdsTool = PadTdsTool,
                                               PadTriggerTool = PadTriggerLogicTool,
                                               PadTriggerLookupTool = PadTriggerLookupTool,
                                               StripTdsTool = StripTdsTool,
                                               StripClusterTool = StripClusterTool,
                                               StripSegmentTool = StripSegmentTool,
                                               MMStripTdsTool = MMStripTdsTool,
                                               MMTriggerTool = MMTriggerTool,
                                               MMTriggerProcessorTool = TriggerProcessorTool,
                                               NSWTrigRDOContainerName = "L1_NSWTrigContainer" )
    acc.addEventAlgo(nswAlg)
    return acc

def RPCTriggerConfig(flags):
    acc = ComponentAccumulator()
    rpcAlg = CompFactory.TrigT1RPC("TrigT1RPC",
                                Hardware          = True,
                                DataDetail        = False,
                                RPCbytestream     = False,
                                RPCbytestreamFile = "",
                                RPCDigitContainer = "RPC_DIGITS_L1",
                                useRun3Config = True )
    acc.addEventAlgo(rpcAlg)
    from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
    acc.merge( RPCCablingConfigCfg(flags) ) # trigger roads
    return acc

def TGCTriggerConfig(flags):
    acc = ComponentAccumulator()
    tgcAlg = CompFactory.LVL1TGCTrigger.LVL1TGCTrigger("LVL1TGCTrigger",
                                                       InputData_perEvent  = "TGC_DIGITS_L1",
                                                       InputRDO = "TGCRDO" if flags.Input.isMC else "TGCRDO_L1",
                                                       useRun3Config = True,
                                                       TileMuRcv_Input = "rerunTileMuRcvCnt",
                                                       TILEMU = True)
    if (flags.Detector.GeometrysTGC or flags.Detector.GeometryMM):
        tgcAlg.MaskFileName12 = "TrigT1TGCMaskedChannel.noFI._12.db"
        tgcAlg.USENSW = True
        tgcAlg.NSWSideInfo = "AC"
        tgcAlg.NSWTrigger_Input = "L1_NSWTrigContainer"
        tgcAlg.FORCENSWCOIN = not flags.Trigger.L1MuonSim.NSWVetoMode
        tgcAlg.USEBIS78 = flags.Trigger.L1MuonSim.doBIS78
    else:
        tgcAlg.MaskFileName12 = "TrigT1TGCMaskedChannel._12.db"

    if flags.Trigger.L1MuonSim.EmulateNSW:
        tgcAlg.MuctpiPhase1LocationTGC = "L1MuctpiStoreTGCint"

    if flags.Input.Format is Format.BS:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        readBSConfig = ByteStreamReadCfg(flags, ['ByteStreamMetadataContainer/ByteStreamMetadata'])
        acc.merge(readBSConfig)
    else:
        tgcAlg.ByteStreamMetadataRHKey = ''
    acc.addEventAlgo(tgcAlg)

    from PathResolver import PathResolver
    bwCW_Run3_filePath=PathResolver.FindCalibFile("TrigT1TGC_CW/BW/CW_BW_Run3.v01.db")
    acc.merge(addFolders(flags, '<db>sqlite://;schema={0};dbname=OFLP200</db> /TGC/TRIGGER/CW_BW_RUN3'.format(bwCW_Run3_filePath),
                                tag='TgcTriggerCwBwRun3-01',
                                className='CondAttrListCollection'))
    acc.addCondAlgo(CompFactory.TGCTriggerCondAlg())
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    acc.merge( TGCCablingConfigCfg(flags) )
    return acc

def TGCModifierConfig(flags):
    acc = ComponentAccumulator()
    if not flags.Trigger.L1MuonSim.EmulateNSW:
        return acc
    recTool = CompFactory.LVL1.TrigT1TGCRecRoiTool("TrigT1TGCRecRoiToolLegacy")
    recTool.UseRun3Config=False # this is intentional
    tgcModifier = CompFactory.LVL1TGCTrigger.TGCOutputModifier("TGCOutputModifier",
                                                                TrigT1TGCRecRoiTool=recTool,
                                                                InputMuctpiLocation = "L1MuctpiStoreTGCint",
                                                                OutputMuctpiLocation = "L1MuctpiStoreTGC",
                                                                EmulateA = True,
                                                                EmulateC = True,
                                                                NSWVetoMode = flags.Trigger.L1MuonSim.NSWVetoMode )
    acc.addEventAlgo(tgcModifier)
    return acc

def MuctpiConfig(flags):
    acc = ComponentAccumulator()
    rpcRecRoiTool = CompFactory.LVL1.TrigT1RPCRecRoiTool("TrigT1RPCRecRoiTool", UseRun3Config=True)
    tgcRecRoiTool = CompFactory.LVL1.TrigT1TGCRecRoiTool("TrigT1TGCRecRoiTool", UseRun3Config=True)
    trigThresholdDecTool = CompFactory.LVL1.TrigThresholdDecisionTool(name="TrigThresholdDecisionTool",
                                                                       RPCRecRoiTool = rpcRecRoiTool,
                                                                       TGCRecRoiTool = tgcRecRoiTool)
    muctpiTool = CompFactory.LVL1MUCTPIPHASE1.MUCTPI_AthTool(name="MUCTPI_AthTool",
                                                              MuCTPICTPLocation = 'L1MuCTPItoCTPLocation',
                                                              OverlapStrategyName = flags.Trigger.MUCTPI.OverlapStrategy,
                                                              LUTXMLFile = flags.Trigger.MUCTPI.LUTXMLFile,
                                                              BarrelRoIFile = flags.Trigger.MUCTPI.BarrelRoIFile,
                                                              EndcapForwardRoIFile = flags.Trigger.MUCTPI.EndcapForwardRoIFile,
                                                              Side0LUTFile = flags.Trigger.MUCTPI.Side0LUTFile,
                                                              Side1LUTFile = flags.Trigger.MUCTPI.Side1LUTFile,
                                                              InputSource = 'DIGITIZATION',
                                                              RPCRecRoiTool = rpcRecRoiTool,
                                                              TGCRecRoiTool = tgcRecRoiTool,
                                                              TrigThresholdDecisionTool = trigThresholdDecTool)
    muctpiAlg = CompFactory.LVL1MUCTPIPHASE1.MUCTPI_AthAlg(name="MUCTPI_AthAlg",
                                                         MUCTPI_AthTool = muctpiTool)
    acc.addEventAlgo(muctpiAlg)
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
    acc.merge(L1ConfigSvcCfg(flags))
    return acc


def Lvl1MuonSimulationCfg(flags):
    acc = ComponentAccumulator()

    acc.merge(MuonBytestream2RdoConfig(flags)) # data prep for muon bytestream data
    acc.merge(MuonRdo2DigitConfig(flags)) # input for rpc/tgc trigger simulation
    acc.merge(RPCTriggerConfig(flags)) # rpc trigger simulation, including bis78 to prepare for bis78-tgc coincidence
    acc.merge(TMDBConfig(flags)) # for tmdb decision to prepare for tile-muon coincidence
    acc.merge(NSWTriggerConfig(flags)) # nsw trigger simulation to prepare input for nsw-tgc coincidence
    acc.merge(TGCTriggerConfig(flags)) # tgc trigger simulation
    acc.merge(MuonRdo2PrdConfig(flags)) # data prep for nsw-tgc coincidence emulator
    acc.merge(RecoMuonSegmentSequence(flags)) # segment reco for nsw-tgc coincidence emulator
    acc.merge(TGCModifierConfig(flags)) # overwrite output from tgc by nsw-tgc coincidence emulator
    acc.merge(MuctpiConfig(flags)) # muctpi simulation

    return acc

if __name__ == "__main__":
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TriggerTest/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.merge.RDO.e4993_s3214_r11315/RDO.17533168._000001.pool.root.1']
    flags.Common.isOnline=False
    flags.Exec.MaxEvents=25
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents=1
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Scheduler.ShowDataFlow=True
    flags.Trigger.enableL1MuonPhase1=True
    flags.Trigger.triggerMenuSetup='Dev_pp_run3_v1'
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
    generateL1Menu(flags)

    acc.merge(Lvl1MuonSimulationCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

    sys.exit(acc.run().isFailure())
