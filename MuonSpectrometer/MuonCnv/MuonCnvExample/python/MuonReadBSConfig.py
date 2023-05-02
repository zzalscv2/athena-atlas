# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.DetFlags import DetFlags
from AthenaCommon.AppMgr import ServiceMgr
from AthenaCommon import CfgMgr
from OverlayCommonAlgs.OverlayFlags import overlayFlags

if DetFlags.readRDOBS.Muon_on():
    if not hasattr( ServiceMgr, "ByteStreamAddressProviderSvc" ):
        from ByteStreamCnvSvcBase. ByteStreamCnvSvcBaseConf import ByteStreamAddressProviderSvc
        ServiceMgr += ByteStreamAddressProviderSvc()

#================================================================================
# MDT Bytestream reading setup
#================================================================================

def MdtROD_Decoder(name="MdtROD_Decoder",**kwargs):
    # setup cabling service needed by this tool
    return CfgMgr.MdtROD_Decoder(name,**kwargs)


def MdtRawDataProviderTool(name="MdtRawDataProviderTool",**kwargs):
    kwargs.setdefault("Decoder", "MdtROD_Decoder")
    if DetFlags.overlay.MDT_on() and overlayFlags.isDataOverlay():
        if overlayFlags.isOverlayMT():
            kwargs.setdefault("RdoLocation",overlayFlags.bkgPrefix()+"MDTCSM")
        else:
            kwargs.setdefault("RdoLocation",overlayFlags.dataStore()+"+MDTCSM")
    return CfgMgr.Muon__MDT_RawDataProviderToolMT(name,**kwargs)


#================================================================================
# RPC Bytestream reading setup
#================================================================================
    
def RpcROD_Decoder(name="RpcROD_Decoder",**kwargs):
    # setup cabling service needed by this tool
    return CfgMgr.Muon__RpcROD_Decoder(name,**kwargs)


def RpcRawDataProviderTool(name = "RpcRawDataProviderTool",**kwargs):
    kwargs.setdefault("Decoder", "RpcROD_Decoder")
    if DetFlags.overlay.RPC_on() and overlayFlags.isDataOverlay():
        if overlayFlags.isOverlayMT():
            kwargs.setdefault("RdoLocation", overlayFlags.bkgPrefix()+"RPCPAD")
        else:
            kwargs.setdefault("RdoLocation", overlayFlags.dataStore()+"+RPCPAD")
    return CfgMgr.Muon__RPC_RawDataProviderToolMT(name,**kwargs)


#================================================================================
# TGC Bytestream reading setup
#================================================================================

def TgcROD_Decoder(name = "TgcROD_Decoder",**kwargs):
    # setup cabling service needed by this tool
    return CfgMgr.Muon__TGC_RodDecoderReadout(name,**kwargs)


def TgcRawDataProviderTool(name = "TgcRawDataProviderTool",**kwargs):
    kwargs.setdefault("Decoder", "TgcROD_Decoder")
    if DetFlags.overlay.TGC_on() and overlayFlags.isDataOverlay():
        if overlayFlags.isOverlayMT():
            kwargs.setdefault("RdoLocation", overlayFlags.bkgPrefix()+"TGCRDO")
        else:
            kwargs.setdefault("RdoLocation", overlayFlags.dataStore()+"+TGCRDO")
    return CfgMgr.Muon__TGC_RawDataProviderToolMT(name,**kwargs)



#================================================================================
# CSC Bytestream reading setup
#================================================================================

def CscROD_Decoder(name = "CscROD_Decoder",**kwargs):
    return CfgMgr.Muon__CscROD_Decoder(name,**kwargs)


def CscRawDataProviderTool(name = "CscRawDataProviderTool",**kwargs):
    kwargs.setdefault("Decoder", "CscROD_Decoder")
    if DetFlags.overlay.CSC_on() and overlayFlags.isDataOverlay():
        if overlayFlags.isOverlayMT():
            kwargs.setdefault("RdoLocation", overlayFlags.bkgPrefix()+"CSCRDO")
        else:
            kwargs.setdefault("RdoLocation", overlayFlags.dataStore()+"+CSCRDO")
    return CfgMgr.Muon__CSC_RawDataProviderToolMT(name,**kwargs)

#================================================================================
# MM Bytestream reading setup
#================================================================================

def MmROD_Decoder(name="MmROD_Decoder",**kwargs):
    kwargs.setdefault("DcsKey", "")
    return CfgMgr.Muon__MM_ROD_Decoder(name,**kwargs)

def MmRawDataProviderTool(name="MmRawDataProviderTool",**kwargs):
    kwargs.setdefault("Decoder", "MmROD_Decoder")
    if DetFlags.overlay.MM_on() and overlayFlags.isDataOverlay():
        if overlayFlags.isOverlayMT():
            kwargs.setdefault("RdoLocation", overlayFlags.bkgPrefix() + "MMRDO")
        else:
            kwargs.setdefault("RdoLocation", overlayFlags.dataStore() + "+MMRDO")
    return CfgMgr.Muon__MM_RawDataProviderToolMT(name,**kwargs)
    
#================================================================================
# sTGC Bytestream reading setup
#================================================================================

def sTgcROD_Decoder(name = "sTgcROD_Decoder",**kwargs):
    kwargs.setdefault("DcsKey", "")
    return CfgMgr.Muon__STGC_ROD_Decoder(name,**kwargs)


def sTgcRawDataProviderTool(name = "sTgcRawDataProviderTool",**kwargs):
    kwargs.setdefault("Decoder", "sTgcROD_Decoder")
    if DetFlags.overlay.sTGC_on() and overlayFlags.isDataOverlay():
        if overlayFlags.isOverlayMT():
            kwargs.setdefault("RdoLocation", overlayFlags.bkgPrefix()+"sTGCRDO")
        else:
            kwargs.setdefault("RdoLocation", overlayFlags.dataStore()+"+sTGCRDO")
    return CfgMgr.Muon__STGC_RawDataProviderToolMT(name,**kwargs)

#================================================================================
# sTGC Pad Trigger Bytestream reading setup
#================================================================================

def sTgcPadTriggerROD_Decoder(name = "sTgcPadTriggerROD_Decoder", **kwargs):
    return CfgMgr.Muon__PadTrig_ROD_Decoder(name, **kwargs)

def sTgcPadTriggerRawDataProviderTool(name = "sTgcPadTriggerRawDataProviderTool", **kwargs):
    kwargs.setdefault("Decoder", "sTgcPadTriggerROD_Decoder")
    return CfgMgr.Muon__PadTrig_RawDataProviderToolMT(name, **kwargs)


#
# For backwards compat - TO BE REMOVED as soon as all clients get these tools via AthenaCommon.CfgGetter
#
from AthenaCommon.CfgGetter import getPublicTool
if DetFlags.readRDOBS.MDT_on():
    MuonMdtRawDataProviderTool = getPublicTool("MdtRawDataProviderTool")
else:
    MuonMdtRawDataProviderTool = None

if DetFlags.readRDOBS.RPC_on():
    MuonRpcRawDataProviderTool = getPublicTool("RpcRawDataProviderTool")
else:
    MuonRpcRawDataProviderTool = None

if DetFlags.readRDOBS.TGC_on():
    MuonTgcRawDataProviderTool = getPublicTool("TgcRawDataProviderTool")
else:
    MuonTgcRawDataProviderTool = None

if DetFlags.readRDOBS.CSC_on():
    MuonCscRawDataProviderTool = getPublicTool("CscRawDataProviderTool")
else:
    MuonCscRawDataProviderTool = None

if DetFlags.readRDOBS.MM_on():
    MuonMmRawDataProviderTool = getPublicTool("MmRawDataProviderTool")
else:
    MuonMmRawDataProviderTool = None

if DetFlags.readRDOBS.sTGC_on():
    MuonsTgcRawDataProviderTool = getPublicTool("sTgcRawDataProviderTool")
    MuonsTgcPadTriggerRawDataProviderTool = getPublicTool("sTgcPadTriggerRawDataProviderTool")
else:
    MuonsTgcRawDataProviderTool = None
    MuonsTgcPadTriggerRawDataProviderTool = None
