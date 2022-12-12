# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

#No input file -> use MC event selector

if 'DBInstance' not in dir():
    DBInstance="CONDBR2"

if "Folder" not in dir():
    Folder="/LAR/BadChannelsOfl/BadChannels"

if "TagPostfix" not in dir():
    TagPostfix="-UPD4-00"

if "InputFile" not in dir():
    InputFile="bc_input.txt"

if "IOVBeginRun" not in dir():
    IOVBeginRun=0

if "IOVBeginLB" not in dir():
    IOVBeginLB=0
    
if "IOVEndRun" not in dir():
    IOVEndRun=-1

if "IOVEndLB" not in dir():
    IOVEndLB=-1
    
if "sqlite" not in dir():
    sqlite="BadChannels.db"

if "isSC" not in dir():
   isSC=False

import AthenaCommon.AtlasUnixGeneratorJob

from AthenaCommon.GlobalFlags import  globalflags
globalflags.DataSource="data"
globalflags.InputFormat="bytestream"
if "OFLP" not in DBInstance:
   globalflags.DatabaseInstance=DBInstance

from AthenaCommon.JobProperties import jobproperties
jobproperties.Global.DetDescrVersion = "ATLAS-R2-2016-01-00-01"

from AthenaCommon.DetFlags import DetFlags
DetFlags.Calo_setOn()
DetFlags.ID_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.digitize.all_setOff()

#Set up GeoModel (not really needed but crashes without)
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import SetupRecoGeometry

#Get identifier mapping
if isSC:
   from LArCabling.LArCablingAccess import LArOnOffIdMappingSC
   LArOnOffIdMappingSC()
else:
   from LArCabling.LArCablingAccess import LArOnOffIdMapping
   LArOnOffIdMapping()

include( "IdDictDetDescrCnv/IdDictDetDescrCnv_joboptions.py" )
include( "CaloDetMgrDetDescrCnv/CaloDetMgrDetDescrCnv_joboptions.py" )


theApp.EvtMax = 1

svcMgr.EventSelector.RunNumber         = 999999
svcMgr.EventSelector.FirstEvent        = 1
#svcMgr.EventSelector.InitialTimeStamp  = 0
#svcMgr.EventSelector.TimeStampInterval = 5
#svcMgr.IOVDbSvc.GlobalTag="COMCOND-ES1P-003-00"
svcMgr.IOVDbSvc.GlobalTag="CONDBR2-ES1PA-2022-06"


## get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence 
topSequence = AlgSequence()  

from AthenaCommon.AppMgr import (theApp, ServiceMgr as svcMgr)

theApp.EvtMax=1

#Thats the registration algo
from LArBadChannelTool.LArBadChannelToolConf import LArBadChannelDBAlg
theLArDBAlg=LArBadChannelDBAlg()
theLArDBAlg.WritingMode = 0
theLArDBAlg.DBFolder=Folder
if isSC:
   theLArDBAlg.BadChanKey="LArBadChannelSC"
   theLArDBAlg.SuperCell=isSC

theLArDBAlg.OutputLevel=DEBUG
topSequence += theLArDBAlg

from LArBadChannelTool.LArBadChannelToolConf import LArBadChannelCondAlg
if isSC:
  theLArBadChannelCondAlg=LArBadChannelCondAlg(ReadKey="", InputFileName=InputFile, WriteKey="LArBadChannelSC", CablingKey="LArOnOffIdMapSC", isSC=isSC, OutputLevel=DEBUG)
else:
  theLArBadChannelCondAlg=LArBadChannelCondAlg(ReadKey="", InputFileName=InputFile, OutputLevel=DEBUG)
from AthenaCommon.AlgSequence import AthSequencer
condSeq = AthSequencer("AthCondSeq")
condSeq+=theLArBadChannelCondAlg

OutputList=[ "CondAttrListCollection#"+Folder ]
Tag=''.join(Folder.split ('/')) + TagPostfix
OutputTagList=[Tag] 

from RegistrationServices.OutputConditionsAlg import OutputConditionsAlg
theOutputConditionsAlg=OutputConditionsAlg("OutputConditionsAlg","dummy.pool.root",
                                           OutputList,OutputTagList,True)

theOutputConditionsAlg.Run1 = IOVBeginRun
theOutputConditionsAlg.LB1 = IOVBeginLB

if IOVEndRun > 0 and IOVEndLB >= 0:
   theOutputConditionsAlg.Run2 = IOVEndRun
   theOutputConditionsAlg.LB2 = IOVEndLB

svcMgr.IOVDbSvc.dbConnection  = "sqlite://;schema="+sqlite+";dbname="+DBInstance
from RegistrationServices.RegistrationServicesConf import IOVRegistrationSvc
svcMgr += IOVRegistrationSvc()
svcMgr.IOVRegistrationSvc.RecreateFolders = False #Allow add in a second tag

svcMgr.DetectorStore.Dump=True


from AthenaCommon                       import CfgMgr
svcMgr+=CfgMgr.AthenaEventLoopMgr(OutputLevel = WARNING)



svcMgr.MessageSvc.OutputLevel=INFO
svcMgr.MessageSvc.debugLimit=50000
svcMgr.MessageSvc.warningLimit=50000
