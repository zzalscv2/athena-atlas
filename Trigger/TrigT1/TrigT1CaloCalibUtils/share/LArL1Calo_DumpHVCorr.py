#################################################################
# 
# example jobOptions to read HV from Cool/DCS in athena
#  and compute cell level correction factors to store in conditions 
#  database.  Uses the next physics run after the time given.
#
#  Usage:
#
#  athena.py -c 'date="2012-04-28:10:01:00";GlobalTag="COMCOND-BLKPA-006-04"' LArL1Calo_DumpHVCorr.py >& job.out
#
#  output is the same sqlite file as for CAF LArL1CaloHV.
#
##################################################################

from time import strptime,time
from calendar import timegm

#set date to compute the correction

if "date" not in dir():
    date="2011-01-31:00:00:00"


if "TimeStamp" not in dir():
   try:
      ts=strptime(date+'/UTC','%Y-%m-%d:%H:%M:%S/%Z')
      TimeStamp=int(timegm(ts))*1000000000
   except ValueError:
      printfunc ("ERROR in time specification, use e.g. 2007-05-25:14:01:00")
      

from LArCalibProcessing.TimeStampToRunLumi import TimeStampToRunLumi

rlb=TimeStampToRunLumi(TimeStamp)
if rlb is None:
   printfunc ("WARNING: Failed to convert time",TimeStamp,"into a run/lumi number")
   RunNumber=999999
   LumiBlock=0
else:
   RunNumber=rlb[0]
   LumiBlock=rlb[1]


printfunc ("Working on run",RunNumber,"LB",LumiBlock,"Timestamp:",TimeStamp)

# name of output local sql file
OutputSQLiteFile = 'myDB200_hvDummy.db'

# name of output Pool file
PoolFileName = "LArHVScaleCorr_dummy.pool.root"

# database folder
LArHVScaleCorrFolder = "/LAR/ElecCalibOfl/HVScaleCorr"

# output key
keyOutput = "LArHVScaleCorr"

# tag suffix
#LArCalibFolderOutputTag = "-UPD3-00"

# write IOV
WriteIOV      = True

# global tag to read other conditions if needed
if "GlobalTag" not in dir():
    GlobalTag     = 'COMCOND-BLKPST-004-05'

# begin run IOV
IOVBegin = 0

###################################################################

from RecExConfig.RecFlags import rec
rec.RunNumber.set_Value_and_Lock(int(RunNumber))

from PerfMonComps.PerfMonFlags import jobproperties
jobproperties.PerfMonFlags.doMonitoring = True

from AthenaCommon.DetFlags import DetFlags
DetFlags.all_setOff()
DetFlags.LAr_setOn()
DetFlags.Tile_setOn()

from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock('atlas')
globalflags.DataSource.set_Value_and_Lock('data')

# Get a handle to the default top-level algorithm sequence
from AthenaCommon.AppMgr import ToolSvc
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# Get a handle to the ServiceManager
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

# Get a handle to the ApplicationManager
from AthenaCommon.AppMgr import theApp

# Setup Db stuff
import AthenaPoolCnvSvc.AthenaPool

from AthenaCommon.GlobalFlags import jobproperties
jobproperties.Global.DetDescrVersion='ATLAS-R2-2015-04-00-00'

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import SetupRecoGeometry

svcMgr.IOVDbSvc.GlobalTag = GlobalTag
try:
   svcMgr.IOVDbSvc.DBInstance=""
except: 
   pass

include( "AthenaCommon/Atlas_Gen.UnixStandardJob.py" )

include( "CaloConditions/CaloConditions_jobOptions.py")
include( "CaloIdCnv/CaloIdCnv_joboptions.py" )
include( "TileIdCnv/TileIdCnv_jobOptions.py" )
include( "LArDetDescr/LArDetDescr_joboptions.py" )
include("TileConditions/TileConditions_jobOptions.py" )
include("LArConditionsCommon/LArConditionsCommon_comm_jobOptions.py")

include( "LArCondAthenaPool/LArCondAthenaPool_joboptions.py" )

from LArConditionsCommon import LArHVDB

from TrigT1CaloCalibUtils.TrigT1CaloCalibUtilsConf import L1CaloHVDummyContainers
theL1CaloHVDummyContainers = L1CaloHVDummyContainers("L1CaloHVDummyContainers")
topSequence += theL1CaloHVDummyContainers

# setup l1calo database
# FIXME:
#include('TrigT1CaloCalibConditions/L1CaloCalibConditionsRun2_jobOptions.py')
# hack because of deleted files from trigger:
from IOVDbSvc.CondDB import conddb
from TrigT1CaloCondSvc.TrigT1CaloCondSvcConf import L1CaloCondSvc
ServiceMgr += L1CaloCondSvc()
L1CaloFolderList = []
L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Calibration/PpmDeadChannels"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/DisabledTowers"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/RunParameters"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Configuration/PprChanDefaults"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Configuration/PprChanDefaults"]

L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Configuration/PprChanStrategy"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Calibration/Physics/PprChanCalib"]

L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanCalib"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanCommon"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanLowMu"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanHighMu"]

L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanCalib"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanCommon"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanLowMu"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanHighMu"]

L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanCalib"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanCommon"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanLowMu"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanHighMu"]


L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/RunParameters"]
L1CaloFolderList += ["/TRIGGER/L1Calo/V1/Conditions/DerivedRunPars"]
L1CaloFolderList += ["/TRIGGER/Receivers/Conditions/VgaDac"]
L1CaloFolderList += ["/TRIGGER/Receivers/Conditions/Strategy"]


for l1calofolder in L1CaloFolderList:
   conddb.addFolderWithTag("TRIGGER", l1calofolder, "HEAD", className="CondAttrListCollection")
# end of the hack

svcMgr.IOVDbSvc.overrideTags += ["<prefix>/LAR/Identifier/LArTTCellMapAtlas</prefix> <tag>LARIdentifierLArTTCellMapAtlas-RUN2-HadFcalFix2</tag>"]
from IOVDbSvc.CondDB import conddb
conddb.addFolderWithTag("CALO_ONL", "/CALO/Identifier/CaloTTOnOffIdMapAtlas", "CALOIdentifierCaloTTOnOffIdMapAtlas-RUN2-0002")
conddb.addFolderWithTag("CALO_ONL", "/CALO/Identifier/CaloTTOnAttrIdMapAtlas", "CALOIdentifierCaloTTOnAttrIdMapAtlas-RUN2-0001")
conddb.addFolderWithTag("CALO_ONL", "/CALO/Identifier/CaloTTPpmRxIdMapAtlas", "CALOIdentifierCaloTTPpmRxIdMapAtlas-RUN2-0000")

from LArCabling.LArCablingAccess import LArOnOffIdMapping
LArOnOffIdMapping()

# set up tools
from TrigT1CaloTools.TrigT1CaloToolsConf import LVL1__L1TriggerTowerTool
ToolSvc += LVL1__L1TriggerTowerTool("L1TriggerTowerTool")
from TrigT1CaloCalibTools.TrigT1CaloCalibToolsConf import LVL1__L1CaloLArTowerEnergy
ToolSvc += LVL1__L1CaloLArTowerEnergy("L1CaloLArTowerEnergy")
from TrigT1CaloCalibTools.TrigT1CaloCalibToolsConf import LVL1__L1CaloCells2TriggerTowers
ToolSvc += LVL1__L1CaloCells2TriggerTowers("L1CaloCells2TriggerTowers")
from TrigT1CaloCalibTools.TrigT1CaloCalibToolsConf import LVL1__L1CaloOfflineTriggerTowerTools
ToolSvc += LVL1__L1CaloOfflineTriggerTowerTools("L1CaloOfflineTriggerTowerTools")

# configure actual db maker algorithm
from TrigT1CaloCalibUtils.TrigT1CaloCalibUtilsConf import L1CaloHVCorrectionsForDB
topSequence += L1CaloHVCorrectionsForDB()

# configure writing of calib database
from RegistrationServices.OutputConditionsAlg import OutputConditionsAlg
HVCorrectionsOutput = OutputConditionsAlg("HVCorrectionsOutput", "dummy.root")
HVCorrectionsOutput.ObjectList = [ "CondAttrListCollection#/TRIGGER/L1Calo/V1/Results/RxLayers",
                                   "CondAttrListCollection#/TRIGGER/L1Calo/V1/Results/HVCorrections"]
HVCorrectionsOutput.WriteIOV = True
HVCorrectionsOutput.Run1 = RunNumber
svcMgr.IOVDbSvc.dbConnection="sqlite://;schema=hvcorrections.sqlite;dbname=L1CALO"
#from LArCalibTools.LArCalibToolsConf import LArHVScaleCorr2Ntuple
#theLArHVScaleCorr2Ntuple = LArHVScaleCorr2Ntuple("LArHVScaleCorr2Ntuple")
#theLArHVScaleCorr2Ntuple.AddFEBTempInfo = False
#topSequence += theLArHVScaleCorr2Ntuple

#from LArCalibTools.LArCalibToolsConf import LArWFParams2Ntuple
#LArWFParams2Ntuple = LArWFParams2Ntuple("LArWFParams2Ntuple")
#LArWFParams2Ntuple.DumpTdrift = True
#topSequence += LArWFParams2Ntuple

#theApp.HistogramPersistency = "ROOT"
#from GaudiSvc.GaudiSvcConf import NTupleSvc
#svcMgr += NTupleSvc()
#svcMgr.NTupleSvc.Output = [ "FILE1 DATAFILE='hvcorr_ntuple.root' OPT='NEW'" ]

# deal with DB output
#OutputObjectSpec = "LArHVScaleCorrComplete#"+keyOutput+"#"+LArHVScaleCorrFolder
#from string import *
#OutputObjectSpecTag = join(split(LArHVScaleCorrFolder, '/'),'') + LArCalibFolderOutputTag
#OutputObjectSpecTag = ''
#OutputDB = "sqlite://;schema="+OutputSQLiteFile+";dbname=COMP200"

#from RegistrationServices.OutputConditionsAlg import OutputConditionsAlg
#theOutputConditionsAlg=OutputConditionsAlg("OutputConditionsAlg",PoolFileName,
#                          [OutputObjectSpec],[OutputObjectSpecTag],WriteIOV)
#theOutputConditionsAlg.Run1 = IOVBegin

#svcMgr.IOVDbSvc.dbConnection  = OutputDB

#from RegistrationServices.RegistrationServicesConf import IOVRegistrationSvc
#svcMgr += IOVRegistrationSvc()
#svcMgr.IOVRegistrationSvc.OutputLevel = DEBUG 
#svcMgr.IOVRegistrationSvc.RecreateFolders = False 



#--------------------------------------------------------------
#--- Dummy event loop parameters
#--------------------------------------------------------------
svcMgr.EventSelector.RunNumber         = RunNumber
svcMgr.EventSelector.EventsPerRun      = 1
svcMgr.EventSelector.FirstEvent        = 1
svcMgr.EventSelector.EventsPerLB       = 1
svcMgr.EventSelector.FirstLB           = LumiBlock
svcMgr.EventSelector.InitialTimeStamp  = int(TimeStamp/1e9)
svcMgr.EventSelector.TimeStampInterval = 5
svcMgr.EventSelector.OverrideRunNumber=True
theApp.EvtMax                          = 1

#--------------------------------------------------------------
# Set output level threshold (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
#--------------------------------------------------------------
svcMgr.MessageSvc.OutputLevel      = INFO
svcMgr.MessageSvc.debugLimit       = 100000
svcMgr.MessageSvc.infoLimit        = 100000
svcMgr.MessageSvc.Format           = "% F%30W%S%7W%R%T %0W%M"
svcMgr.IOVDbSvc.OutputLevel        = INFO

#theLArHVCorrMaker.OutputLevel = INFO
