# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###########################################################################
#
# LArCalib_Delay_OFC_2Ntuple_POOL_jobOptions.py dumps the conntet of a 
# CaliWave + OFC  POOL files (from standard AP processing)
# to ROOT TTree. 
#
# Author: Pavol Strizenec (pavol @ mail.cern.ch)
#
# Created: Nov. 2010 by merging existing Wave and OFC 2 Ntuple jO 
# Modified: Mar. 2023, migrate to rel. 23
#
###########################################################################

CWOFC2NtupleLog = logging.getLogger( "CWOFC2NtupleLog" )

if not 'PoolFileName' in dir():
        CWOFC2NtupleLog.fatal("Please setup the input POOL file ('PoolFileName')!")
        theApp.exit(-1)

if not 'ContainerKey' in dir():
        ContainerKey = "Pedestal"

if not 'RootFileName' in dir():
        RootFileName = "LArPedestal2Ntuple_POOL.root"

if not 'RunNumber' in dir():
        RunNumber = 393960

if not 'SuperCells' in dir():
        SuperCells = False

###########################################################################

include("LArCalibProcessing/LArCalib_Flags.py")

include("LArCalibProcessing/LArCalib_GeomSetup.py")

from LArCabling.LArCablingAccess import LArOnOffIdMapping
LArOnOffIdMapping()

if SuperCells:
   from LArCabling.LArCablingAccess import LArOnOffIdMappingSC,LArCalibIdMappingSC,LArLATOMEMappingSC
   LArOnOffIdMappingSC()
   LArCalibIdMappingSC()
   LArLATOMEMappingSC()

## get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence 
topSequence = AlgSequence()

from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")



from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import LArGM
from LArAlignmentAlgs.LArAlignmentAlgsConf import LArAlignCondAlg
condSeq += LArAlignCondAlg("LArAlignCondAlg")
from CaloAlignmentAlgs.CaloAlignmentAlgsConf import CaloAlignCondAlg
condSeq += CaloAlignCondAlg("CaloAlignCondAlg")

## get a handle to the ApplicationManager, to the ServiceManager and to the ToolSvc
from AthenaCommon.AppMgr import (theApp, ServiceMgr as svcMgr,ToolSvc)

###########################################################################

include("AthenaPoolCnvSvc/AthenaPool_jobOptions.py")
include("LArCondAthenaPool/LArCondAthenaPool_joboptions.py")
from LArConditionsCommon import LArAlignable

if not 'DBConnectionCOOL' in dir():
        DBConnectionCOOL = "COOLONL_LAR/CONDBR2"

## define the DB Global Tag :
svcMgr.IOVDbSvc.GlobalTag   = LArCalib_Flags.globalFlagDB
svcMgr.IOVDbSvc.DBInstance=""
svcMgr.IOVDbSvc.forceRunNumber=RunNumber

from IOVDbSvc.CondDB import conddb
PoolFileList     = []

if 'InputBadChannelSQLiteFile' in dir():
   from string import *
   InputDBConnectionBadChannel = "sqlite://;schema="+InputBadChannelSQLiteFile+";dbname=CONDBR2"
else:
   InputDBConnectionBadChannel = DBConnectionCOOL

if ( not 'InputBadChannelSQLiteFile' in dir()) and ("ONL" in DBConnectionCOOL):
   if 'BadChannelsFolder' not in dir():
      BadChannelsFolder="/LAR/BadChannels/BadChannels"
   conddb.addFolder("",BadChannelsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className="CondAttrListCollection")
   MissingFEBsFolder="/LAR/BadChannels/MissingFEBs"
   conddb.addFolder("",BadChannelsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className="CondAttrListCollection")
   conddb.addFolder("",MissingFEBsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className="AthenaAttributeList")
else:   
   if 'BadChannelsFolder' not in dir():
      BadChannelsFolder="/LAR/BadChannelsOfl/BadChannels"
   conddb.addFolder("",BadChannelsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className="CondAttrListCollection")
   MissingFEBsFolder="/LAR/BadChannelsOfl/MissingFEBs"
   conddb.addFolder("",BadChannelsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className="CondAttrListCollection")
   conddb.addFolder("",MissingFEBsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className="AthenaAttributeList")

from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")

from LArBadChannelTool.LArBadChannelToolConf import LArBadChannelCondAlg, LArBadFebCondAlg
theLArBadChannelCondAlg=LArBadChannelCondAlg(ReadKey=BadChannelsFolder)
condSeq+=theLArBadChannelCondAlg

theLArBadFebCondAlg=LArBadFebCondAlg(ReadKey=MissingFEBsFolder)
condSeq+=theLArBadFebCondAlg

from AthenaCommon.ConfigurableDb import getConfigurable
svcMgr += getConfigurable( "ProxyProviderSvc" )()
svcMgr.ProxyProviderSvc.ProviderNames += [ "CondProxyProvider" ]
svcMgr += getConfigurable( "CondProxyProvider" )()
svcMgr.CondProxyProvider.InputCollections += [ PoolFileName ]

from LArCalibTools.LArCalibToolsConf import LArPedestals2Ntuple
LArPedestals2Ntuple = LArPedestals2Ntuple("LArPedestals2Ntuple")
LArPedestals2Ntuple.ContainerKey = ContainerKey
LArPedestals2Ntuple.OffId = True
LArPedestals2Ntuple.AddHash = True
LArPedestals2Ntuple.RealGeometry = True
topSequence += LArPedestals2Ntuple


theApp.HistogramPersistency = "ROOT"
from GaudiSvc.GaudiSvcConf import NTupleSvc
svcMgr += NTupleSvc()
svcMgr.NTupleSvc.Output = [ "FILE1 DATAFILE='"+RootFileName+"' OPT='NEW'" ]
   
##########################################################################

theApp.EvtMax = 1

###########################################################################

