#==============================================================
#
# Job options file to produce ROOT file with electronic noise
#
# Full ATLAS setup, TileCal only digitization
#
#==============================================================

from AthenaCommon.AppMgr import theApp
svcMgr = theApp.serviceMgr()

from AthenaCommon.Logging import logging
logDigitization_flags = logging.getLogger( 'Tile_Digitization' )

#---  Output printout level -----------------------------------
#output threshold (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
if not 'OutputLevel' in dir():
    OutputLevel = 4
svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.MessageSvc.defaultLimit = 1000000
svcMgr.MessageSvc.Format = "% F%60W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.useColors = False

# get a handle on topalg
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

#--------------------------------------------------------------
# configuration flags
#--------------------------------------------------------------

# - Number of events to be processed
if not 'EvtMax' in dir():
    EvtMax = 100000

# location of sqlite files, input pool files and output root files
SqliteDirectoryCesium="."
SqliteDirectoryNoise="."
SqliteDirectoryBch="."
InputDirectory="."
OutputDirectory="."

# name of sqlite files with new sample noise and new bad channel list
if not 'DBFileBch' in dir():
    DBFileBch=""
if not 'DBFileNoise' in dir():
    DBFileNoise=""
if not 'DBFileCesium' in dir():
    DBFileCesium=""
if not 'BchTag' in dir():
    BchTag=""
if not 'NoiseTag' in dir():
    NoiseTag=""
if not 'CesiumTag' in dir():
    CesiumTag=""
if not 'RunNumber' in dir():
    RunNumber = 410000

# Version number which is appended to output file name
if not 'Version' in dir():
    Version = 0
Suffix=("%d" % Version )

# - input file with hits
if not 'PoolHitsInput' in dir():
    PoolHitsInput = 'HITS.pool.root'

# - output file with digits
if not 'PoolRDOOutput' in dir():
    PoolRDOOutput = 'DIGITS.pool_%s.root' % Suffix

if not 'ConddbTag' in dir():
    ConddbTag = 'OFLCOND-MC23-SDR-RUN3-01'

if not 'DetDescrVersion' in dir():
    DetDescrVersion = 'ATLAS-R3S-2021-03-00-00'

# adding directory names to all input/output files
if not "/" in DBFileBch   and DBFileBch!=""   and SqliteDirectoryBch!=".":   DBFileBch=SqliteDirectoryBch+"/"+DBFileBch
if not "/" in DBFileNoise and DBFileNoise!="" and SqliteDirectoryNoise!=".": DBFileNoise=SqliteDirectoryNoise+"/"+DBFileNoise
if not "/" in DBFileCesium and DBFileCesium!="" and SqliteDirectoryCesium!=".": DBFileCesium=SqliteDirectoryCesium+"/"+DBFileCesium
if not "/" in PoolHitsInput and InputDirectory!="." : PoolHitsInput=InputDirectory+"/"+PoolHitsInput
if not "/" in PoolRDOOutput and OutputDirectory!="." : PoolRDOOutput=OutputDirectory+"/"+PoolRDOOutput

from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock('atlas')
globalflags.Luminosity.set_Value_and_Lock('zero')
globalflags.DataSource.set_Value_and_Lock('geant4')
globalflags.InputFormat.set_Value_and_Lock('pool')
#globalflags.DatabaseInstance="OFLP200"

from TileConditions.TileCoolMgr import tileCoolMgr
if BchTag!="":
    tileCoolMgr.isOfflineOnly('oflStatAdc')
    tileCoolMgr.setFolder('oflStatAdc','/TILE/OFL02/STATUS/ADC')
    tileCoolMgr.setTag(   "oflStatAdc",BchTag)
    if DBFileBch!="":
        tileCoolMgr.setDbConn("oflStatAdc", DBFileBch)

if NoiseTag!="":
    tileCoolMgr.setFolder("onlNoiseAdc","/TILE/OFL02/NOISE/SAMPLE")
    tileCoolMgr.setTag("onlNoiseAdc",NoiseTag)
    tileCoolMgr.setFolder("oflNoiseAdc","/TILE/OFL02/NOISE/SAMPLE")
    tileCoolMgr.setTag("oflNoiseAdc",NoiseTag)
    if DBFileNoise!="":
        tileCoolMgr.setDbConn("onlNoiseAdc", DBFileNoise)
        tileCoolMgr.setDbConn("oflNoiseAdc", DBFileNoise)

if CesiumTag!="":
    tileCoolMgr.setFolder("onlCes","/TILE/OFL02/CALIB/CES")
    tileCoolMgr.setTag("onlCes",CesiumTag)
    tileCoolMgr.setFolder("oflCes","/TILE/OFL02/CALIB/CES")
    tileCoolMgr.setTag("oflCes",CesiumTag)
    if DBFileCesium!="":
        tileCoolMgr.setDbConn("onlCes", DBFileCesium)
        tileCoolMgr.setDbConn("oflCes", DBFileCesium)



#--------------------------------------------------------------
# AthenaCommon configuration
#--------------------------------------------------------------
from AthenaCommon.AthenaCommonFlags import jobproperties
#jobproperties.AthenaCommonFlags.AllowIgnoreConfigError=False #This job will stop if an include fails.

jobproperties.AthenaCommonFlags.EvtMax = EvtMax
# Set input file
jobproperties.AthenaCommonFlags.PoolHitsInput=[]
for i in range (0,100): # one file contains few events only, use it 100 times
    jobproperties.AthenaCommonFlags.PoolHitsInput+=[PoolHitsInput]
jobproperties.AthenaCommonFlags.PoolRDOOutput=PoolRDOOutput

#--------------------------------------------------------------------
# DetFlags. Use to turn on/off individual subdetector or LVL1 trigger
#--------------------------------------------------------------------
from AthenaCommon.DetFlags import DetFlags

DetFlags.ID_setOff()
DetFlags.Calo_setOff()
DetFlags.Muon_setOff()
DetFlags.LVL1_setOff()
#DetFlags.sTGCMicromegas_setOff()

DetFlags.Truth_setOn()

DetFlags.Tile_setOn()

# - switch off tasks
DetFlags.pileup.all_setOff()
DetFlags.simulate.all_setOff()
DetFlags.makeRIO.all_setOff()
DetFlags.writeBS.all_setOff()
DetFlags.readRDOBS.all_setOff()
DetFlags.readRIOBS.all_setOff()
DetFlags.readRDOPool.all_setOff()
DetFlags.writeRDOPool.all_setOff()
DetFlags.readRIOPool.all_setOff()
DetFlags.writeRIOPool.all_setOff()
DetFlags.simulateLVL1.all_setOff()

# - print flags
DetFlags.Print()


#--------------------------------------------------------------
# Global flags. Like eg the DD version:
#--------------------------------------------------------------
from AthenaCommon.BeamFlags import jobproperties
jobproperties.Beam.beamType.set_Value_and_Lock('collisions')

#from IOVDbSvc.CondDB import conddb
#conddb.setGlobalTag(ConddbTag);
#log.info( "ConddbTag = %s" % (ConddbTag) )

from AthenaCommon.GlobalFlags import jobproperties
jobproperties.Global.DetDescrVersion = DetDescrVersion
log.info( "DetDescrVersion = %s" % (jobproperties.Global.DetDescrVersion()) )

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
GeoModelSvc = GeoModelSvc()
GeoModelSvc.IgnoreTagDifference = True
log.info( "GeoModelSvc.AtlasVersion = %s" % (GeoModelSvc.AtlasVersion) )
#GeoModelSvc.TileVersionOverride = "TileCal-GEO-13"
#log.info( "GeoModelSvc.TileVersionOverride = %s" % (GeoModelSvc.TileVersionOverride) )

#--------------------------------------------------------------
# Digitiziation and Pileup configuration
#--------------------------------------------------------------
from Digitization.DigitizationFlags import jobproperties
# jobproperties.Digitization.doInDetNoise=True
jobproperties.Digitization.doCaloNoise = True
# This tag must be specified for dowstream jobs
jobproperties.Digitization.IOVDbGlobalTag = ConddbTag
jobproperties.Digitization.simRunNumber = RunNumber
jobproperties.Digitization.dataRunNumber = RunNumber
# jobproperties.Digitization.doMuonNoise=True
# jobproperties.Digitization.doMinimumBias=True
# jobproperties.Digitization.numberOfCollisions=2.3
# jobproperties.Digitization.minBiasInputCols=["", "", "" ]
# jobproperties.Digitization.doCavern=True
# jobproperties.Digitization.numberOfCavern=2
# jobproperties.Digitization.cavernInputCols=["", ""]
# jobproperties.Digitization.doBeamGas=True
# jobproperties.Digitization.numberOfBeamGas=0.5
# jobproperties.Digitization.beamGasInputCols=["", ""]

include.block ( "TileL2Algs/TileL2Algs_jobOptions.py" )

from TileRecUtils.TileRecFlags import jobproperties
jobproperties.TileRecFlags.doTileOptATLAS = True

include( "Digitization/Digitization.py" )

# don't need any hits at input
topSequence.StandardPileUpToolsAlg.PileUpTools[0].TileHitVectors=[]

# Set this to False for monogain
# True  => Bigain, use this to derive constants
# False => Monogain, use this to digitize a simulation containing signal
topSequence.TileDigitsMaker.CalibrationRun=True
#topSequence.TileDigitsMaker.OutputLevel=VERBOSE

# don't need any L1 or L2 algorithms
del topSequence.TileHitToTTL1
del topSequence.TileRawChannelToL2

# add Noise Calib Tool
from TileCalibAlgs.TileCalibAlgsConf import TileRawChNoiseCalibAlg

theTileRawChNoiseCalibAlg = TileRawChNoiseCalibAlg( "theTileRawChNoiseCalibAlg" )
theTileRawChNoiseCalibAlg.doFixed = True
theTileRawChNoiseCalibAlg.doFit = False
theTileRawChNoiseCalibAlg.doOpt = False
theTileRawChNoiseCalibAlg.doDsp = False
theTileRawChNoiseCalibAlg.doOF1 = False
theTileRawChNoiseCalibAlg.doMF  = False
theTileRawChNoiseCalibAlg.UseforCells = 1
theTileRawChNoiseCalibAlg.TileRawChannelContainerFixed = "TileRawChannelCnt"
theTileRawChNoiseCalibAlg.RunNumber = RunNumber
theTileRawChNoiseCalibAlg.MaskBadChannels = True
Prefix = '%(Dir)s/RawCh_NoiseCalib_%(Version)s'  %  {'Dir': OutputDirectory, 'Version': Suffix }
theTileRawChNoiseCalibAlg.FileNamePrefix = Prefix

topSequence += theTileRawChNoiseCalibAlg

if not 'OutputLevel' in dir():
    OutputLevel = 4
svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.MessageSvc.defaultLimit = 1000000
svcMgr.MessageSvc.Format = "% F%60W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.useColors = False

from AthenaServices.AthenaServicesConf import AthenaEventLoopMgr
svcMgr += AthenaEventLoopMgr()
svcMgr.AthenaEventLoopMgr.EventPrintoutInterval = 100

from AthenaCommon.AlgSequence import AthSequencer
condSequence = AthSequencer("AthCondSeq")
if hasattr(condSequence, 'TileSamplingFractionCondAlg'):
    condSequence.TileSamplingFractionCondAlg.G4Version = -1

print (topSequence)
