#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

## Small class to hold the names for cache containers, should help to avoid copy / paste errors
class MuonCacheNames(object):
    MdtCsmCache = "MdtCsmRdoCache"
    CscCache    = "CscRdoCache"
    RpcCache    = "RpcRdoCache"
    TgcCache    = "TgcRdoCache"
    
## This configuration function creates the IdentifiableCaches for RDO
#
# The function returns a ComponentAccumulator which should be loaded first
# If a configuration wants to use the cache, they need to use the same names as defined here
def MuonCacheCfg(flags):


    acc = ComponentAccumulator()

    MuonCacheCreator=CompFactory.MuonCacheCreator
    cacheCreator = MuonCacheCreator(MdtCsmCacheKey = MuonCacheNames.MdtCsmCache,
                                    CscCacheKey    = (MuonCacheNames.CscCache if flags.Detector.GeometryCSC else ""),
                                    RpcCacheKey    = MuonCacheNames.RpcCache,
                                    TgcCacheKey    = MuonCacheNames.TgcCache)

    acc.addEventAlgo( cacheCreator, primary=True )
    return acc

## This configuration function sets up everything for decoding RPC bytestream data into RDOs
#
# The function returns a ComponentAccumulator and the data-decoding algorithm, which should be added to the right sequence by the user
def RpcBytestreamDecodeCfg(flags, name="RpcRawDataProvider"):
    acc = ComponentAccumulator()
    
    # We need the RPC cabling to be setup
    from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
    acc.merge( RPCCablingConfigCfg(flags) )

    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags)) 
    
    # Setup the RPC ROD decoder
    RPCRodDecoder = CompFactory.Muon.RpcROD_Decoder(name	     = "RpcROD_Decoder" )


    # Setup the RAW data provider tool
    keyName = flags.Overlay.BkgPrefix + "RPCPAD" if flags.Common.isOverlay else "RPCPAD"
    MuonRpcRawDataProviderTool = CompFactory.Muon.RPC_RawDataProviderToolMT(name    = "RPC_RawDataProviderToolMT",
                                                                 Decoder = RPCRodDecoder,
                                                                 RdoLocation = keyName )
    if flags.Muon.MuonTrigger:
        MuonRpcRawDataProviderTool.RpcContainerCacheKey   = MuonCacheNames.RpcCache
        MuonRpcRawDataProviderTool.WriteOutRpcSectorLogic = False

    
    # Setup the RAW data provider algorithm
    Muon__RpcRawDataProvider=CompFactory.Muon.RpcRawDataProvider
    RpcRawDataProvider = Muon__RpcRawDataProvider(name         = name,
                                                  ProviderTool = MuonRpcRawDataProviderTool )

    if flags.Muon.MuonTrigger:
        # Configure the RAW data provider for ROI access
        from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
        RpcRawDataProvider.RoIs = mapThresholdToL1RoICollection("MU")
        RpcRawDataProvider.DoSeededDecoding = True
        # add RegSelTool
        from RegionSelector.RegSelToolConfig import regSelTool_RPC_Cfg
        RpcRawDataProvider.RegionSelectionTool = acc.popToolsAndMerge( regSelTool_RPC_Cfg( flags ) )


    acc.addEventAlgo(RpcRawDataProvider, primary=True)
    return acc

def TgcBytestreamDecodeCfg(flags, name="TgcRawDataProvider"):
    acc = ComponentAccumulator()

    # We need the TGC cabling to be setup
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    acc.merge( TGCCablingConfigCfg(flags) )

    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags)) 

    # Setup the TGC ROD decoder
    Muon__TGC_RodDecoderReadout=CompFactory.Muon.TGC_RodDecoderReadout
    TGCRodDecoder = Muon__TGC_RodDecoderReadout(name = "TgcROD_Decoder")


    # Setup the RAW data provider tool
    keyName = flags.Overlay.BkgPrefix + "TGCRDO" if flags.Common.isOverlay else "TGCRDO"
    Muon__TGC_RawDataProviderToolMT=CompFactory.Muon.TGC_RawDataProviderToolMT
    MuonTgcRawDataProviderTool = Muon__TGC_RawDataProviderToolMT(name    = "TGC_RawDataProviderToolMT",
                                                                 Decoder = TGCRodDecoder,
                                                                 RdoLocation = keyName )

    if flags.Muon.MuonTrigger:
        MuonTgcRawDataProviderTool.TgcContainerCacheKey   = MuonCacheNames.TgcCache

    
    # Setup the RAW data provider algorithm
    Muon__TgcRawDataProvider=CompFactory.Muon.TgcRawDataProvider
    TgcRawDataProvider = Muon__TgcRawDataProvider(name         = name,
                                                  ProviderTool = MuonTgcRawDataProviderTool )
    if flags.Muon.MuonTrigger:
        TgcRawDataProvider.DoSeededDecoding = True
        # add RegSelTool
        from RegionSelector.RegSelToolConfig import regSelTool_TGC_Cfg
        TgcRawDataProvider.RegionSelectionTool = acc.popToolsAndMerge( regSelTool_TGC_Cfg( flags ) )


    acc.addEventAlgo(TgcRawDataProvider,primary=True)

    return acc

def MdtBytestreamDecodeCfg(flags, name="MdtRawDataProvider"):
    acc = ComponentAccumulator()

    # We need the MDT cabling to be setup
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    acc.merge( MDTCablingConfigCfg(flags) )

    # need the MagFieldSvc since MdtRdoToMdtPrepData.MdtRdoToMdtPrepDataTool.MdtCalibrationTool wants to retrieve it
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge( AtlasFieldCacheCondAlgCfg(flags) )

    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags)) 

    # Setup the MDT ROD decoder
    MdtROD_Decoder=CompFactory.MdtROD_Decoder
    MDTRodDecoder = MdtROD_Decoder(name="MdtROD_Decoder")


    # Setup the RAW data provider tool
    keyName = flags.Overlay.BkgPrefix + "MDTCSM" if flags.Common.isOverlay else "MDTCSM"
    Muon__MDT_RawDataProviderToolMT=CompFactory.Muon.MDT_RawDataProviderToolMT
    MuonMdtRawDataProviderTool = Muon__MDT_RawDataProviderToolMT(name    = "MDT_RawDataProviderToolMT",
                                                                 Decoder = MDTRodDecoder,
                                                                 RdoLocation = keyName)

    if flags.Muon.MuonTrigger:
        MuonMdtRawDataProviderTool.CsmContainerCacheKey = MuonCacheNames.MdtCsmCache

    # Setup the RAW data provider algorithm
    Muon__MdtRawDataProvider=CompFactory.Muon.MdtRawDataProvider
    MdtRawDataProvider = Muon__MdtRawDataProvider(name         = name,
                                                  ProviderTool = MuonMdtRawDataProviderTool )
    if flags.Muon.MuonTrigger:
        MdtRawDataProvider.DoSeededDecoding = True
        # add RegSelTool
        from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
        MdtRawDataProvider.RegionSelectionTool = acc.popToolsAndMerge( regSelTool_MDT_Cfg( flags ) )


    acc.addEventAlgo(MdtRawDataProvider,primary=True)

    return acc

def CscBytestreamDecodeCfg(flags, name="CscRawDataProvider"):
    acc = ComponentAccumulator()

    # We need the CSC cabling to be setup
    from MuonConfig.MuonCablingConfig import CSCCablingConfigCfg # Not yet been prepared
    acc.merge( CSCCablingConfigCfg(flags) )

    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags)) 

    # Setup the CSC ROD decoder
    Muon__CscROD_Decoder=CompFactory.Muon.CscROD_Decoder
    CSCRodDecoder = Muon__CscROD_Decoder(name	     = "CscROD_Decoder" )


    # Setup the RAW data provider tool
    keyName = flags.Overlay.BkgPrefix + "CSCRDO" if flags.Common.isOverlay else "CSCRDO"
    Muon__CSC_RawDataProviderToolMT=CompFactory.Muon.CSC_RawDataProviderToolMT
    MuonCscRawDataProviderTool = Muon__CSC_RawDataProviderToolMT(name    = "CSC_RawDataProviderToolMT",
                                                                 Decoder = CSCRodDecoder,
                                                                 RdoLocation = keyName)
    if flags.Muon.MuonTrigger:
        MuonCscRawDataProviderTool.CscContainerCacheKey = MuonCacheNames.CscCache
    
    # Setup the RAW data provider algorithm
    Muon__CscRawDataProvider=CompFactory.Muon.CscRawDataProvider
    CscRawDataProvider = Muon__CscRawDataProvider(name         = name,
                                                  ProviderTool = MuonCscRawDataProviderTool )
    if flags.Muon.MuonTrigger:
        CscRawDataProvider.DoSeededDecoding = True
        # add RegSelTool
        from RegionSelector.RegSelToolConfig import regSelTool_CSC_Cfg
        CscRawDataProvider.RegionSelectionTool = acc.popToolsAndMerge( regSelTool_CSC_Cfg( flags ) )


    acc.addEventAlgo(CscRawDataProvider,primary=True)

    return acc


def sTgcRODDecoderCfg(flags, name = "sTgcROD_Decoder", **kwargs):
    result = ComponentAccumulator()
    ### Disable the NSW DSC data for the time being
    if False and not flags.Muon.MuonTrigger and not flags.Input.isMC:
        from MuonConfig.MuonCondAlgConfig import NswDcsDbAlgCfg
        result.merge(NswDcsDbAlgCfg(flags))
    else:
        kwargs.setdefault("DcsKey", "")
    # Setup the sTGC ROD decoder
    STGCRodDecoder = CompFactory.Muon.STGC_ROD_Decoder(name = name, **kwargs)
    result.setPrivateTools(STGCRodDecoder)
    return result

def sTgcBytestreamDecodeCfg(flags, name="MuonsTgcRawDataProvider"):

    acc = ComponentAccumulator()

    # We need the sTGC cabling to be setup
    #from MuonConfig.MuonCablingConfig import STGCCablingConfigCfg
    #acc.merge( STGCCablingConfigCfg(flags) )

    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags))



    # Setup the RAW data provider tool
    keyName = flags.Overlay.BkgPrefix + "sTGCRDO" if flags.Common.isOverlay else "sTGCRDO"
    Muon__STGC_RawDataProviderToolMT=CompFactory.Muon.STGC_RawDataProviderToolMT
    MuonsTgcRawDataProviderTool = Muon__STGC_RawDataProviderToolMT(name    = "sTgcRawDataProviderTool",
                                                                   Decoder = acc.popToolsAndMerge(sTgcRODDecoderCfg(flags)),
                                                                   RdoLocation = keyName,
                                                                   SkipDecoding=flags.Muon.MuonTrigger and flags.Muon.runCommissioningChain )

    #if flags.Muon.MuonTrigger:
    #    MuonsTgcRawDataProviderTool.sTgcContainerCacheKey = MuonCacheNames.sTgcCache
    
    # Setup the RAW data provider algorithm
    Muon__sTgcRawDataProvider=CompFactory.Muon.sTgcRawDataProvider
    sTgcRawDataProvider = Muon__sTgcRawDataProvider(name       = name,
                                                  ProviderTool = MuonsTgcRawDataProviderTool )
    if flags.Muon.MuonTrigger:
        sTgcRawDataProvider.DoSeededDecoding = True
        # add RegSelTool
        from RegionSelector.RegSelToolConfig import regSelTool_STGC_Cfg
        sTgcRawDataProvider.RegionSelectionTool = acc.popToolsAndMerge( regSelTool_STGC_Cfg( flags ) )


    acc.addEventAlgo(sTgcRawDataProvider,primary=True)

    return acc

def NswTrigProcessorRodDecoderCfg(flags, name ="NswTrigProcessorDecoder", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.Muon.NSWTP_ROD_Decoder(name = name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
def NswTrigProccesorRawDataProviderToolCfg(flags, name = "NswTrigProcessorRawDataTool", **kwargs ):
    result = ComponentAccumulator()
    kwargs.setdefault("Decoder", result.popToolsAndMerge(NswTrigProcessorRodDecoderCfg(flags)))
    kwargs.setdefault( "RdoLocation", ( flags.Overlay.BkgPrefix  if flags.Common.isOverlay else "") + "NSW_TrigProcessor_RDO" )
    the_tool = CompFactory.Muon.NSWTP_RawDataProviderToolMT(name = name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
def NswTrigProcByteStreamDecodeCfg(flags, name = "NswProcByteStream"):
    result = ComponentAccumulator()
    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    the_alg = CompFactory.Muon.sTgcPadTriggerRawDataProvider(name = name,
                                                             ProviderTool = result.popToolsAndMerge(NswTrigProccesorRawDataProviderToolCfg(flags)) )
    result.addEventAlgo(the_alg, primary = True)
    return result

def sTgcPadTriggerBytestreamDecodeCfg(flags, name="MuonsTgcPadTriggerRawDataProvider"):

    acc = ComponentAccumulator()

    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags))

    # Setup the sTGC ROD decoder
    Muon__PadTrig_ROD_Decoder = CompFactory.Muon.PadTrig_ROD_Decoder
    STGCPadTriggerRodDecoder = Muon__PadTrig_ROD_Decoder(name = "sTgcPadTriggerROD_Decoder")


    # Setup the RAW data provider tool
    keyName = flags.Overlay.BkgPrefix + "NSW_PadTrigger_RDO" if flags.Common.isOverlay else "NSW_PadTrigger_RDO"
    Muon__PadTrig_RawDataProviderToolMT = CompFactory.Muon.PadTrig_RawDataProviderToolMT
    MuonsTgcPadTriggerRawDataProviderTool = Muon__PadTrig_RawDataProviderToolMT(name = "sTgcPadTriggerRawDataProviderTool",
                                                                                Decoder = STGCPadTriggerRodDecoder,
                                                                                RdoLocation = keyName)


    # Setup the RAW data provider algorithm
    the_alg = CompFactory.Muon.sTgcPadTriggerRawDataProvider(name = name,
                                                             ProviderTool = MuonsTgcPadTriggerRawDataProviderTool )

    acc.addEventAlgo(the_alg, primary = True)

    return acc

def MmRDODDecoderCfg(flags, name="MmROD_Decoder", **kwargs):
    result = ComponentAccumulator()
    ### Disable the NSW DSC data for the time being
    if not flags.Muon.MuonTrigger and not flags.Input.isMC:        
        from MuonConfig.MuonCondAlgConfig import NswDcsDbAlgCfg
        if False: result.merge(NswDcsDbAlgCfg(flags))
        from MuonConfig.MuonCablingConfig import MicroMegaCablingCfg
        result.merge(MicroMegaCablingCfg(flags))
    else:
        kwargs.setdefault("CablingMap", "")
    
    kwargs.setdefault("DcsKey", "")
    
    the_tool = CompFactory.Muon.MM_ROD_Decoder(name = name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
def MmBytestreamDecodeCfg(flags, name="MmRawDataProvider"):
    acc = ComponentAccumulator()

    # We need the MM cabling to be setup
    #from MuonConfig.MuonCablingConfig import MM_CablingConfigCfg
    #acc.merge( MM_CablingConfigCfg(flags) )

    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags)) 
   
    # Setup the RAW data provider tool
    keyName = flags.Overlay.BkgPrefix + "MMRDO" if flags.Common.isOverlay else "MMRDO"
    MuonMmRawDataProviderTool = CompFactory.Muon.MM_RawDataProviderToolMT(name  = "MM_RawDataProviderToolMT",
                                                              Decoder = acc.popToolsAndMerge(MmRDODDecoderCfg(flags)),
                                                              RdoLocation = keyName,
                                                              SkipDecoding=flags.Muon.MuonTrigger and flags.Muon.runCommissioningChain)

    #if flags.Muon.MuonTrigger:
    #    MuonMmRawDataProviderTool.RawDataContainerCacheKey = MuonCacheNames.MicromegasCache

    # Setup the RAW data provider algorithm
    Muon__MmRawDataProvider = CompFactory.Muon.MM_RawDataProvider
    MmRawDataProvider = Muon__MmRawDataProvider(name = name, ProviderTool = MuonMmRawDataProviderTool )
    if flags.Muon.MuonTrigger:
        MmRawDataProvider.DoSeededDecoding = True
        # add RegSelTool
        from RegionSelector.RegSelToolConfig import regSelTool_MM_Cfg
        MmRawDataProvider.RegionSelectionTool = acc.popToolsAndMerge( regSelTool_MM_Cfg( flags ) )

    acc.addEventAlgo(MmRawDataProvider,primary=True)

    return acc

def MuonByteStreamDecodersCfg(flags):
    cfg=ComponentAccumulator()
    
    # Adds all services needed to read BS
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags ))

    # Schedule Rpc data decoding
    rpcdecodingAcc = RpcBytestreamDecodeCfg( flags ) 
    cfg.merge( rpcdecodingAcc )

    # Schedule Tgc data decoding
    tgcdecodingAcc = TgcBytestreamDecodeCfg( flags ) 
    cfg.merge( tgcdecodingAcc )

    # Schedule Mdt data decoding
    mdtdecodingAcc  = MdtBytestreamDecodeCfg( flags )
    cfg.merge( mdtdecodingAcc )


    if flags.Detector.GeometryCSC:
        # Schedule Csc data decoding
        cscdecodingAcc = CscBytestreamDecodeCfg( flags ) 
        cfg.merge( cscdecodingAcc )

    if (flags.Detector.GeometrysTGC and flags.Detector.GeometryMM):
        # Schedule MM data decoding
        cfg.merge( MmBytestreamDecodeCfg( flags ) )

        # Schedule sTGC data decoding
        cfg.merge( sTgcBytestreamDecodeCfg( flags )  )

        # Schedule sTGC Pad Trigger data decoding
        cfg.merge( sTgcPadTriggerBytestreamDecodeCfg( flags ) )
        cfg.merge(NswTrigProcByteStreamDecodeCfg(flags))

    return cfg


if __name__=="__main__":
    # To run this, do e.g. 
    # python ../athena/MuonSpectrometer/MuonConfig/python/MuonBytestreamDecode.py

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    # Set global tag by hand for now
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2018-13"#"CONDBR2-BLKPA-2015-17"
    flags.GeoModel.AtlasVersion = "ATLAS-R2-2016-01-00-01"#"ATLAS-R2-2015-03-01-00"

    flags.lock()
    flags.dump()

    from AthenaCommon.Logging import log 

    log.setLevel(DEBUG)
    log.info('About to setup Rpc Raw data decoding')

    cfg = MuonByteStreamDecodersCfg(flags)

    # Need to add POOL converter  - may be a better way of doing this?
    from AthenaCommon import CfgMgr
    cfg.addService( CfgMgr.AthenaPoolCnvSvc() )
    cfg.getService("EventPersistencySvc").CnvServices += [ "AthenaPoolCnvSvc" ]

    log.info('Print Config')
    cfg.printConfig(withDetails=True)

    # Store config as pickle
    log.info('Save Config')
    with open('MuonBytestreamDecode.pkl','wb') as f:
        cfg.store(f)
        f.close()

