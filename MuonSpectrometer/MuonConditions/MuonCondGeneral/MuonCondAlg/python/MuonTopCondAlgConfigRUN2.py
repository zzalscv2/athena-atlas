# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

## Configuration Access to OFFLINE DB (e.g. COMP200)

from IOVDbSvc.CondDB import conddb

from AthenaCommon import CfgMgr
from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.Logging import logging
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags

def addFolder(parent, db, folder):
    log = logging.getLogger(parent.getName())        
    log.info("Adding folder %s:%s to IOVDbSvc", db, folder)
    conddb.addFolder(db, folder, className='CondAttrListCollection')


class MdtCondDbAlg(CfgMgr.MdtCondDbAlg):
    def __init__(self,name="MdtCondDbAlg",**kwargs):
        if athenaCommonFlags.isOnline:
            kwargs['isOnline'] = True  # COOL folders not available online
            kwargs['ReadKey_MC_DE'] = ''
            kwargs['ReadKey_MC_DT'] = ''
        else:    
            kwargs['isOnline'] = False # COOL folders not available online
            if globalflags.DataSource != 'data':
                kwargs['isData'] = False
                kwargs['ReadKey_MC_DE'] = ''
                kwargs['ReadKey_MC_DT'] = ''
                kwargs['ReadKey_DataR1_DC'] = ''
                kwargs['ReadKey_DataR1_HV'] = ''
                kwargs['ReadKey_DataR1_LV'] = ''
                kwargs['ReadKey_DataR1_V0'] = ''
                kwargs['ReadKey_DataR1_V1'] = ''
                kwargs['ReadKey_DataR2_HV'] = ''
                kwargs['ReadKey_DataR2_LV'] = ''
            if globalflags.DataSource == 'data':
                kwargs['isData'] = True
                if conddb.dbname == 'COMP200':
                    kwargs['isRun1'] = True
                    kwargs['ReadKey_DataR2_HV'] = ''
                    kwargs['ReadKey_DataR2_LV'] = ''
                else:
                    kwargs['isRun1'] = False
                    kwargs['ReadKey_MC_DE'] = ''
                    kwargs['ReadKey_MC_DT'] = ''
                    kwargs['ReadKey_DataR1_DC'] = ''
                    kwargs['ReadKey_DataR1_HV'] = ''
                    kwargs['ReadKey_DataR1_LV'] = ''
                    kwargs['ReadKey_DataR1_V0'] = ''
                    kwargs['ReadKey_DataR1_V1'] = ''
                kwargs['useRun1SetPoints'] = False
        super(MdtCondDbAlg,self).__init__(name,**kwargs)
        if athenaCommonFlags.isOnline: return
        if globalflags.DataSource == 'data':
            if kwargs['isRun1']:
                addFolder(self, "DCS_OFL", "/MDT/DCS/PSHVMLSTATE")
                addFolder(self, "DCS_OFL", "/MDT/DCS/PSLVCHSTATE")
                addFolder(self, "DCS_OFL", "/MDT/DCS/DROPPEDCH")
                if kwargs['useRun1SetPoints']:
                    addFolder(self, "DCS_OFL", "/MDT/DCS/PSV0SETPOINTS")
                    addFolder(self, "DCS_OFL", "/MDT/DCS/PSV1SETPOINTS")
            else:
                addFolder(self, "DCS_OFL", "/MDT/DCS/HV")
                addFolder(self, "DCS_OFL", "/MDT/DCS/LV")
        else: # if MC or simulation
            addFolder(self, "DCS_OFL", "/MDT/DCS/DROPPEDCH"  )
            addFolder(self, "DCS_OFL", "/MDT/DCS/PSLVCHSTATE")
            kwargs['ReadKey_DataR1_DC'] = ''
            kwargs['ReadKey_DataR1_HV'] = ''
            kwargs['ReadKey_DataR1_LV'] = ''
            kwargs['ReadKey_DataR1_V0'] = ''
            kwargs['ReadKey_DataR1_V1'] = ''
            kwargs['ReadKey_DataR2_HV'] = ''
            kwargs['ReadKey_DataR2_LV'] = ''


class RpcCondDbAlg(CfgMgr.RpcCondDbAlg):
    def __init__(self,name="RpcCondDbAlg",**kwargs):
        if athenaCommonFlags.isOnline:
            kwargs['isOnline'] = True  # COOL folders not available online
        else:    
            kwargs['isOnline'] = False # COOL folders not available online
            if globalflags.DataSource != 'data':
                kwargs['isData'] = False
            if globalflags.DataSource == 'data':
                kwargs['isData'] = True
                kwargs['isRun1'] = conddb.dbname == 'COMP200'
        super(RpcCondDbAlg,self).__init__(name,**kwargs)
        if athenaCommonFlags.isOnline: return
        if globalflags.DataSource == 'data':
            addFolder(self, "DCS_OFL", "/RPC/DCS/DeadRopanels"   )
            addFolder(self, "DCS_OFL", "/RPC/DCS/OffRopanels"    )
            addFolder(self, "RPC_OFL", "/RPC/DQMF/ELEMENT_STATUS")
        else: # if MC or simulation
            addFolder(self, "RPC_OFL", "/RPC/DQMF/ELEMENT_STATUS")

class CscCondDbAlg(CfgMgr.CscCondDbAlg):
    def __init__(self,name="CscCondDbAlg",**kwargs):
        pslope_from_db = False
        kwargs['ReadKey_HV'] = '' # Never used at present
        if athenaCommonFlags.isOnline:
            kwargs['isOnline'  ] = True  # COOL folders not available online
            kwargs['isData'    ] = True
            kwargs['ReadKey_FT'] = '/CSC/FTHOLD' # 'ConditionsStore+' prefix not necessarily needed in ReadKey
            kwargs['ReadKey_NO'] = '/CSC/NOISE'
            kwargs['ReadKey_PD'] = '/CSC/PED'
            kwargs['ReadKey_PS'] = '/CSC/PSLOPE'
            kwargs['ReadKey_RM'] = '/CSC/RMS'
            kwargs['ReadKey_ST'] = '/CSC/STAT'
            kwargs['ReadKey_TB'] = ''
            kwargs['ReadKey_TP'] = ''
        else:    
            kwargs['isOnline'] = False # COOL folders not available online
            if globalflags.DataSource != 'data':
                kwargs['isData'] = False
            if globalflags.DataSource == 'data':
                kwargs['isData'] = True
                kwargs['isRun1'] = conddb.dbname == 'COMP200'
        super(CscCondDbAlg,self).__init__(name,**kwargs)
        if athenaCommonFlags.isOnline: 
            addFolder(self, "CSC_ONL", "/CSC/ONL/FTHOLD" )
            addFolder(self, "CSC_ONL", "/CSC/ONL/NOISE"  )
            addFolder(self, "CSC_ONL", "/CSC/ONL/PED"    )
            if pslope_from_db:
                addFolder(self, "CSC_ONL", "/CSC/ONL/PSLOPE" )
            addFolder(self, "CSC_ONL", "/CSC/ONL/RMS"    )
            addFolder(self, "CSC_ONL", "/CSC/ONL/STAT"   )
        else:
            addFolder(self, "CSC_OFL", "/CSC/FTHOLD" )
            addFolder(self, "CSC_OFL", "/CSC/NOISE"  )
            addFolder(self, "CSC_OFL", "/CSC/PED"    )
            if pslope_from_db:
                addFolder(self, "CSC_OFL", "/CSC/PSLOPE" )
            addFolder(self, "CSC_OFL", "/CSC/RMS"    )
            addFolder(self, "CSC_OFL", "/CSC/STAT"   )
            addFolder(self, "CSC_OFL", "/CSC/T0BASE" )
            addFolder(self, "CSC_OFL", "/CSC/T0PHASE")

class TgcCondDbAlg(CfgMgr.TgcCondDbAlg):
    def __init__(self,name="TgcCondDbAlg",**kwargs):
        if athenaCommonFlags.isOnline:
            kwargs['isOnline'] = True  # COOL folders not available online
        else:    
            kwargs['isOnline'] = False # COOL folders not available online
            if globalflags.DataSource != 'data':
                kwargs['isData'] = False
            if globalflags.DataSource == 'data':
                kwargs['isData'] = True
                kwargs['isRun1'] = conddb.dbname == 'COMP200'
        super(TgcCondDbAlg,self).__init__(name,**kwargs)
        if athenaCommonFlags.isOnline: return
        addFolder(self, "", "") # which folder?

class NswCalibDbAlg(CfgMgr.NswCalibDbAlg):
    def __init__(self,name="NswCalibDbAlg",**kwargs):
        kwargs['isData'] = (globalflags.DataSource == 'data')
        ## data online
        if athenaCommonFlags.isOnline: 
            kwargs["ReadKey_MM_SIDEA_TDO"] = "/MDT/Onl/MM/TIME/SIDEA"
            kwargs["ReadKey_MM_SIDEC_TDO"] = "/MDT/Onl/MM/TIME/SIDEC"
            kwargs["ReadKey_MM_SIDEA_PDO"] = "/MDT/Onl/MM/CHARGE/SIDEA"
            kwargs["ReadKey_MM_SIDEC_PDO"] = "/MDT/Onl/MM/CHARGE/SIDEC"
            kwargs["ReadKey_STGC_SIDEA_TDO"] = "/TGC/Onl/NSW/TIME/SIDEA"
            kwargs["ReadKey_STGC_SIDEC_TDO"] = "/TGC/Onl/NSW/TIME/SIDEC"
            kwargs["ReadKey_STGC_SIDEA_PDO"] = "/TGC/Onl/NSW/CHARGE/SIDEA"
            kwargs["ReadKey_STGC_SIDEC_PDO"] = "/TGC/Onl/NSW/CHARGE/SIDEC"
        ## init algo
        super(NswCalibDbAlg,self).__init__(name,**kwargs)
        ## data online
        if athenaCommonFlags.isOnline: 
            addFolder(self, "MDT_ONL", "/MDT/Onl/MM/TIME/SIDEA"   )
            addFolder(self, "MDT_ONL", "/MDT/Onl/MM/TIME/SIDEC"   )
            addFolder(self, "MDT_ONL", "/MDT/Onl/MM/CHARGE/SIDEA" )
            addFolder(self, "MDT_ONL", "/MDT/Onl/MM/CHARGE/SIDEC" )
            addFolder(self, "TGC_ONL", "/TGC/Onl/NSW/TIME/SIDEA"  )
            addFolder(self, "TGC_ONL", "/TGC/Onl/NSW/TIME/SIDEC"  )
            addFolder(self, "TGC_ONL", "/TGC/Onl/NSW/CHARGE/SIDEA")
            addFolder(self, "TGC_ONL", "/TGC/Onl/NSW/CHARGE/SIDEC")
            return
        ## data offline
        addFolder(self, "MDT_OFL", "/MDT/MM/TIME/SIDEA"   )
        addFolder(self, "MDT_OFL", "/MDT/MM/TIME/SIDEC"   )
        addFolder(self, "MDT_OFL", "/MDT/MM/CHARGE/SIDEA" )
        addFolder(self, "MDT_OFL", "/MDT/MM/CHARGE/SIDEC" )
        addFolder(self, "TGC_OFL", "/TGC/NSW/TIME/SIDEA"  )
        addFolder(self, "TGC_OFL", "/TGC/NSW/TIME/SIDEC"  )
        addFolder(self, "TGC_OFL", "/TGC/NSW/CHARGE/SIDEA")
        addFolder(self, "TGC_OFL", "/TGC/NSW/CHARGE/SIDEC")
        if not kwargs['isData']: ## MC
            addFolder(self, "MDT_OFL", "/MDT/MM/THR/SIDEA" )
            addFolder(self, "MDT_OFL", "/MDT/MM/THR/SIDEC" )
            addFolder(self, "TGC_OFL", "/TGC/NSW/THR/SIDEA")
            addFolder(self, "TGC_OFL", "/TGC/NSW/THR/SIDEC")

class NswPassivationDbAlg(CfgMgr.NswPassivationDbAlg):
    def __init__(self,name="NswPassivationDbAlg",**kwargs):
        if athenaCommonFlags.isOnline: return
        if CommonGeometryFlags.Run in ["RUN1","RUN2"]: return
        super(NswPassivationDbAlg,self).__init__(name,**kwargs)
        addFolder(self, "MDT_OFL", "/MDT/MM/PASSIVATION")

