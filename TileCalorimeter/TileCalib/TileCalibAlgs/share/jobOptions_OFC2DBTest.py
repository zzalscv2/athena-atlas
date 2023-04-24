#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
#**************************************************************
#
# jopOptions file to create COOL DB with OFC
#
#==============================================================
import AthenaCommon.AtlasUnixGeneratorJob


#============================================================
#=== get user options or set default
#============================================================
if not 'nSamples' in dir():
    nSamples = 7

if not 'nPhases' in dir():
    nPhases = 1000

if not 'phaseStep' in dir():
    phaseStep = 0.1

if not 'run' in dir():
    run = 400000

if not 'runType' in dir():
    runType = 'PHY' # 'PHY' or 'LAS' or 'CIS'

if not 'pulseType' in dir():
    pulseType='CISPULSE100' if runType=='CIS' else runType # 'PHY' or 'LAS' or 'CISPULSE100'

if not 'module' in dir():
    module = 'AUX01' # modules for which OFC are created, can be single module of list of modules

if not 'maxChan' in dir():
    maxChan = 1 # number of channels in a module for which OFC are created

if not 'zero' in dir():
    zero = False # create or not zero-sized blobs for missing modules in DB

if not 'update' in dir():
    update = False # update existing DB or create new one

if not 'OutputLevel' in dir():
    OutputLevel = INFO

if not 'dbACR' in dir():
    dbACR = None # name of sqlite file, e.g. tileSqlite_CovMatrix.db

if not 'dbPulse' in dir():
    dbPulse = None # name of sqlite file, e.g. tileSqlite_Pulse.db

if dbACR or dbPulse:
    from AthenaCommon.GlobalFlags import globalflags
    globalflags.DetGeo.set_Value_and_Lock('atlas')
    globalflags.DataSource.set_Value_and_Lock('data')
    globalflags.DatabaseInstance="CONDBR2"

    from TileConditions.TileCoolMgr import tileCoolMgr

    if dbACR:
        tileCoolMgr.setFolder("oflNoiseAcr","/TILE/OFL02/NOISE/AUTOCR")
        tileCoolMgr.setTag(   "oflNoiseAcr","RUN2-HLT-UPD1-00")
        tileCoolMgr.setDbConn("oflNoiseAcr",dbACR)

    if dbPulse:
        tileCoolMgr.setFolder("oflPlsPhy","/TILE/OFL02/PULSESHAPE/PHY")
        tileCoolMgr.setTag(   "oflPlsPhy","RUN2-HLT-UPD1-00")
        tileCoolMgr.setDbConn("oflPlsPhy",dbPulse)
        tileCoolMgr.setFolder("oflPlsLas","/TILE/OFL02/PULSESHAPE/LAS")
        tileCoolMgr.setTag(   "oflPlsLas","RUN2-HLT-UPD1-01")
        tileCoolMgr.setDbConn("oflPlsLas",dbPulse)
        tileCoolMgr.setFolder("oflPlsCisPl100","/TILE/OFL02/PULSESHAPE/CIS/PULSE100")
        tileCoolMgr.setTag(   "oflPlsCisPl100","RUN2-HLT-UPD1-01")
        tileCoolMgr.setDbConn("oflPlsCisPl100",dbPulse)


#============================================================
#=== Global Flags and Geometry setup
#============================================================
from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock('atlas')
globalflags.Luminosity.set_Value_and_Lock('zero')
globalflags.DataSource.set_Value_and_Lock('data')
globalflags.InputFormat.set_Value_and_Lock('bytestream')
globalflags.DatabaseInstance="CONDBR2"

#--- Geometry setup
from AthenaCommon.DetFlags import DetFlags
DetFlags.detdescr.ID_setOff()
DetFlags.detdescr.Muon_setOff()
DetFlags.detdescr.LAr_setOff()
DetFlags.detdescr.Tile_setOn()

#--- see https://aiatlas003.cern.ch:5443/tag_hierarchy_browser.php
#--- for the geometry updates
from AthenaCommon.JobProperties import jobproperties
jobproperties.Global.DetDescrVersion = "ATLAS-R3S-2021-02-00-00"
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit


#============================================================
#=== set global conditions tag
#============================================================
#--- check what is the latest tag using command CheckTagAssociation.py
from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag("CONDBR2-BLKPA-2022-10")


#============================================================
#=== set Tile conditions
#============================================================
from TileConditions.TileInfoConfigurator import TileInfoConfigurator
tileInfoConfigurator = TileInfoConfigurator()
tileInfoConfigurator.setupCOOL(type = runType)
tileInfoConfigurator.setupCOOLOFC(type = runType, ofcType = 'OF2')
tileInfoConfigurator.setupCOOLPULSE(type = runType)
tileInfoConfigurator.setupCOOLAutoCr()
tileInfoConfigurator.setupCOOLTIME(type = runType, online=False)

tileInfoConfigurator.NSamples = nSamples
tileInfoConfigurator.TrigSample = (nSamples-1)//2 # Floor division


#============================================================
#=== configure TileCondToolOfc
#============================================================
from TileConditions.TileConditionsConf import TileCondToolOfc
tileCondToolOfc = TileCondToolOfc()
tileCondToolOfc.nSamples = nSamples
tileCondToolOfc.OptFilterDeltaCorrelation = (not dbACR) # True - unity matrix, False - use matrix from DB
tileCondToolOfc.OutputLevel = INFO if OutputLevel < INFO else OutputLevel

from TileConditions.TileCondToolConf import *
tileCondToolOfc.TileCondToolPulseShape = getTileCondToolPulseShape('COOL',pulseType)


#============================================================
#=== add OFC2DB Algorithm, options
#============================================================
from TileCalibAlgs.TileCalibAlgsConf import TileOFC2DBAlg
tileOFC2DBAlg = TileOFC2DBAlg()
tileOFC2DBAlg.FixedPhases = True # True - 2*nPhases + 1 phases for default channel only
tileOFC2DBAlg.OF2 = True # True - default
tileOFC2DBAlg.RunType = runType # PHY (default) or LAS or CIS
tileOFC2DBAlg.RunIOVSince = run # 0 - default
tileOFC2DBAlg.LbnIOVSince = 0
tileOFC2DBAlg.OutputLevel = OutputLevel

tileOFC2DBAlg.FixedPhasesNumber = nPhases
tileOFC2DBAlg.PhaseStep = phaseStep
tileOFC2DBAlg.Modules = module if type(module) is list else [module]
tileOFC2DBAlg.MaxChan = maxChan
tileOFC2DBAlg.CreateAllModules = zero # If True, zero-sized blobs will be created for all missing modules

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()
from xAODEventInfoCnv.xAODEventInfoCreator import xAODMaker__EventInfoCnvAlg
topSequence += xAODMaker__EventInfoCnvAlg()
topSequence += tileOFC2DBAlg


#============================================================
# Options for IOVRegistrationSvc
#============================================================
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
import RegistrationServices.IOVRegistrationSvc
regSvc = svcMgr.IOVRegistrationSvc
regSvc.RecreateFolders = (not update) # True - to recreate DB completely, False - insert new IOV in existing DB
regSvc.SVFolder = True
regSvc.OverrideNames+=['TileCalibBlobOfc']
regSvc.OverrideTypes+=['Blob16M']
svcMgr.IOVDbSvc.dbConnection="sqlite://;schema=tileSqlite.db;dbname=CONDBR2"


#============================================================
#=== MessageSvc setup
#============================================================
MessageSvc = Service( "MessageSvc" )
MessageSvc.OutputLevel = INFO if OutputLevel < INFO else OutputLevel
svcMgr.MessageSvc.defaultLimit = 10000000


#============================================================
#=== Dummy event loop setup
#============================================================
from AthenaCommon.AppMgr import theApp
theApp.EvtMax                          = 1
svcMgr.EventSelector.RunNumber         = run
svcMgr.EventSelector.EventsPerRun      = 5
svcMgr.EventSelector.FirstEvent        = 1
svcMgr.EventSelector.EventsPerLB       = 5
svcMgr.EventSelector.FirstLB           = 1
svcMgr.EventSelector.InitialTimeStamp  = 0
svcMgr.EventSelector.TimeStampInterval = 5


#============================================================
#=== print out job summary
#============================================================
print(svcMgr)
print(topSequence)
