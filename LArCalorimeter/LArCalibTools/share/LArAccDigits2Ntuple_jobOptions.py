# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import os


if not 'SuperCells' in dir():   
   SuperCells=False
if not SuperCells: include("LArCalibProcessing/LArCalib_Flags.py")
else: SuperCells:     include("LArCalibProcessing/LArCalib_FlagsSC.py")

if not 'SubDet' in dir():
   SubDet = "Barrel"

if not 'RunNumberList' in dir():
   RunNumberList = [ '0018660' ]
   
if not 'BaseFileName' in dir():
   BaseFileName = "LArAccDigits"
   
   for RunNumber in RunNumberList :
      BaseFileName = BaseFileName+"_"+str(RunNumber)

if not 'OutputRootFileName' in dir():
   OutputRootFileName = BaseFileName+".root"
   
if not 'OutputDir' in dir():
   OutputDir = os.getcwd()

if not 'FilePrefix' in dir():
   if (int(RunNumberList[0]))<99800 :
      FilePrefix = "daq.Ramp"
   else :
      FilePrefix = "data*"
     
if not 'NSamples' in dir():
   NSamples=32

if not 'InputDir' in dir():
   InputDir = "/eos/atlas/atlastier0/rucio/data23_calib/"

if not 'SCIgnoreBarrelChannels' in dir():
   SCIgnoreBarrelChannels=False

if not 'SCIgnoreEndcapChannels' in dir():
   SCIgnoreEndcapChannels=False
   
if not 'FullFileName' in dir():
   if not 'Trigger' in dir():
      if (int(RunNumberList[0]))<99800 :
         Trigger = "*"+Partition
      else :
         if 'Partition' in dir():
            Trigger = "calibration_"+".*"+Partition   
         else:   
            Trigger = "calibration_"+"*"   
   
   from LArCalibProcessing.GetInputFiles import GetInputFilesFromTokens,GetInputFiles
   FullFileName = []
   for RunNumber in RunNumberList :
       FullFileName+=GetInputFilesFromTokens(InputDir,int(RunNumber),FilePrefix,Trigger)

   
if not 'EvtMax' in dir():
   EvtMax=-1

if not 'WriteNtuple' in dir():
   WriteNtuple = LArCalib_Flags.WriteNtuple

if not 'Gain' in dir():
   if SuperCells:
      Gain = "SC"
   else:
      Gain = "HIGH"

if SuperCells:
   from AthenaCommon.GlobalFlags import globalflags
   globalflags.DetGeo.set_Value_and_Lock('atlas')
   globalflags.Luminosity.set_Value_and_Lock('zero')
   globalflags.DataSource.set_Value_and_Lock('data')
   globalflags.InputFormat.set_Value_and_Lock('bytestream')
   globalflags.DatabaseInstance.set_Value_and_Lock('CONDBR2')
   
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
   
   #Get identifier mapping
   include( "LArConditionsCommon/LArIdMap_comm_jobOptions.py" )
   include( "LArIdCnv/LArIdCnv_joboptions.py" )
   include( "ByteStreamCnvSvc/BSEventStorageEventSelector_jobOptions.py" )
else:
   include ("LArConditionsCommon/LArMinimalSetup.py")

from LArCabling.LArCablingAccess import  LArOnOffIdMapping,LArFebRodMapping,LArCalibIdMapping
LArOnOffIdMapping()
LArFebRodMapping()
LArCalibIdMapping()
if SuperCells:
   from LArCabling.LArCablingAccess import  LArOnOffIdMappingSC, LArCalibIdMappingSC,LArLATOMEMappingSC
   LArOnOffIdMappingSC()
   LArCalibIdMappingSC()
   LArLATOMEMappingSC()


svcMgr.IOVDbSvc.GlobalTag=LArCalib_Flags.globalFlagDB
svcMgr.IOVDbSvc.DBInstance=""

if 'BadChannelsFolder' not in dir():
   BadChannelsFolder="/LAR/BadChannels/BadChannels"
if 'MissingFEBsFolder' not in dir():
   MissingFEBsFolder="/LAR/BadChannels/MissingFEBs"

if not 'ReadBadChannelFromCOOL' in dir():
   ReadBadChannelFromCOOL = True   

if ( ReadBadChannelFromCOOL ):      
   if 'InputBadChannelSQLiteFile' in dir():
      InputDBConnectionBadChannel = DBConnectionFile(InputBadChannelSQLiteFile)
   else:
      if 'InputDBConnectionBadChannel' not in dir():
         InputDBConnectionBadChannel = "COOLONL_LAR/" + conddb.dbname

if 'BadChannelsLArCalibFolderTag' in dir() :
   BadChannelsTagSpec = LArCalibFolderTag (BadChannelsFolder,BadChannelsLArCalibFolderTag) 
   conddb.addFolder("",BadChannelsFolder+"<tag>"+BadChannelsTagSpec+"</tag>"+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",
                    className="CondAttrListCollection")
else :
   conddb.addFolder("",BadChannelsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className="CondAttrListCollection")


if 'MissingFEBsLArCalibFolderTag' in dir() :
   MissingFEBsTagSpec = LArCalibFolderTag (MissingFEBsFolder,MissingFEBsLArCalibFolderTag)   
   conddb.addFolder("",MissingFEBsFolder+"<tag>"+MissingFEBsTagSpec+"</tag>"+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className='AthenaAttributeList')
else :
   conddb.addFolder("",MissingFEBsFolder+"<dbConnection>"+InputDBConnectionBadChannel+"</dbConnection>",className='AthenaAttributeList')


from LArBadChannelTool.LArBadChannelToolConf import LArBadChannelCondAlg, LArBadFebCondAlg
theLArBadChannelCondAlg=LArBadChannelCondAlg(ReadKey=BadChannelsFolder)
condSeq+=theLArBadChannelCondAlg

theLArBadFebCondAlg=LArBadFebCondAlg(ReadKey=MissingFEBsFolder)
condSeq+=theLArBadFebCondAlg

from AthenaCommon.AlgSequence import AlgSequence 
topSequence = AlgSequence()  


## get a handle to the ApplicationManager, to the ServiceManager and to the ToolSvc
from AthenaCommon.AppMgr import (theApp, ServiceMgr as svcMgr,ToolSvc)

theByteStreamInputSvc=svcMgr.ByteStreamInputSvc
if not 'FullFileName' in dir():
   RampLog.info( "No FullFileName! Please give a FullFileName list." )
   theApp.exit(-1)

else :   
   svcMgr.EventSelector.Input=FullFileName
   
svcMgr.EventSelector.MaxBadEvents = 0

svcMgr.ByteStreamCnvSvc.InitCnvs += [ "EventInfo"]

theByteStreamAddressProviderSvc =svcMgr.ByteStreamAddressProviderSvc

if SuperCells:
  from LArByteStream.LArByteStreamConf import LArLATOMEDecoder
  theLArLATOMEDecoder = LArLATOMEDecoder("LArLATOMEDecoder")

  from LArByteStream.LArByteStreamConf import LArRawSCCalibDataReadingAlg
  LArRawSCCalibDataReadingAlg = LArRawSCCalibDataReadingAlg()
  LArRawSCCalibDataReadingAlg.LArSCAccDigitKey = Gain
  LArRawSCCalibDataReadingAlg.LATOMEDecoder = LArLATOMEDecoder("LArLATOMEDecoder")
  LArRawSCCalibDataReadingAlg.LATOMEDecoder.IgnoreBarrelChannels = SCIgnoreBarrelChannels
  LArRawSCCalibDataReadingAlg.LATOMEDecoder.IgnoreEndcapChannels = SCIgnoreEndcapChannels
  LArRawSCCalibDataReadingAlg.LATOMEDecoder.OutputLevel = WARNING

  topSequence+=LArRawSCCalibDataReadingAlg

else:  
  # Need to debug the raw data reading algo for legacy
  from LArByteStream.LArByteStreamConf import LArRawCalibDataReadingAlg

  theLArRawCalibDataReadingAlg=LArRawCalibDataReadingAlg()
  theLArRawCalibDataReadingAlg.LArAccDigitKey=Gain
  theLArRawCalibDataReadingAlg.LArFebHeaderKey="LArFebHeader"
  #temporarily
  theLArRawCalibDataReadingAlg.FailOnCorruption=False

  topSequence+=theLArRawCalibDataReadingAlg

from LArCalibTools.LArCalibToolsConf import LArAccumulatedDigits2Ntuple

LArAccDigits2Ntuple=LArAccumulatedDigits2Ntuple("LArAccDigits2Ntuple")

LArAccDigits2Ntuple.ContainerKey = Gain
LArAccDigits2Ntuple.AddFEBTempInfo=False
if 'FTlist' in dir():
   LArAccDigits2Ntuple.FTlist=FTlist

LArAccDigits2Ntuple.isSC = SuperCells

#LArAccDigits2Ntuple.RealGeometry = True
LArAccDigits2Ntuple.OffId = True

topSequence+= LArAccDigits2Ntuple

theApp.HistogramPersistency = "ROOT"
from GaudiSvc.GaudiSvcConf import NTupleSvc
svcMgr += NTupleSvc()
svcMgr.NTupleSvc.Output = [ "FILE1 DATAFILE='"+OutputDir + "/" +OutputRootFileName+"' OPT='NEW'" ]

theApp.EvtMax=EvtMax

