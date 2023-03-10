#**************************************************************
#
# jopOptions file for reading ByteStream
#
#==============================================================

import sys
from subprocess import check_output
from subprocess import CalledProcessError
import six

from AthenaCommon.AppMgr import theApp
svcMgr = theApp.serviceMgr()

from AthenaCommon.Logging import logging
log = logging.getLogger( 'jobOptions_TileTBDump.py' )

#---  Output printout level -----------------------------------
#output threshold (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
if not 'OutputLevel' in dir():
    OutputLevel = 3
svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.MessageSvc.defaultLimit = 1000000
svcMgr.MessageSvc.Format = "% F%60W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.useColors = False

if not 'RunNumber' in dir():
    RunNumber = 0

if not 'FileNameVec' in dir():
    if not 'FileName' in dir() or FileName == "":
        if not 'InputDirectory' in dir():
            InputDirectory = None
        if not 'RunStream' in dir():
            RunStream = None
        if not 'DataProject' in dir():
            DataProject = None
        if not 'DirectorySuffix' in dir():
            DirectorySuffix = None
        if not 'FileFilter' in dir():
            FileFilter = '.data'
        from TileRecEx import TileInputFiles
        FileNameVec = TileInputFiles.findFiles(RunNumber,InputDirectory,FileFilter,RunStream,DataProject,DirectorySuffix,None,False)
    else:
        FileNameVec = [ FileName ]

log.info("InputDirectory is " + str(InputDirectory))
log.info("RunNumber is " + str(RunNumber))
log.info("FullFileName is " + str(FileNameVec))

if len(FileNameVec) < 1:
    log.fatal("Input file not found")
    sys.exit(1)

if not 'RUN3' in dir():
    RUN3 = (RunNumber >= 411938) or (RunNumber<=0) or (RunNumber==3)

if not 'RUN2' in dir():
    RUN2 = not RUN3 and ((RunNumber > 232000) or (RunNumber==2))

if not 'MC' in dir():
    MC = (RunNumber>0) and (RunNumber<10)

if not 'EvtMin' in dir():
    EvtMin = 0
elif (EvtMin < 0):
    EvtMin = 0
    noDump = True

if not 'EvtMax' in dir():
    EvtMax = 3
elif (EvtMax < 1):
    EvtMax = 100000
    dumpOnce = True

if not 'EventNumber' in dir():
    EventNumber = -1

if not 'BCID' in dir():
    BCID = -1

if not 'TriggerType' in dir():
    TriggerType = -1

if not 'Level1ID' in dir():
    Level1ID = -1

#=============================================================
#=== init Det Descr
#=============================================================

from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock('atlas')
globalflags.Luminosity.set_Value_and_Lock('zero')
globalflags.InputFormat.set_Value_and_Lock('bytestream')
if MC:
    globalflags.DataSource.set_Value_and_Lock('geant4')
else:
     globalflags.DataSource.set_Value_and_Lock('data')
if not (RUN2 or RUN3 or MC):
    globalflags.DatabaseInstance="COMP200"

from AthenaCommon.BeamFlags import jobproperties
jobproperties.Beam.beamType.set_Value_and_Lock('cosmics')

from AthenaCommon.DetFlags import DetFlags
DetFlags.Calo_setOff()  #Switched off to avoid geometry
DetFlags.ID_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.digitize.all_setOff()

DetFlags.detdescr.ID_setOff()
DetFlags.detdescr.Muon_setOff()
DetFlags.detdescr.LAr_setOff()
DetFlags.detdescr.Tile_setOn()
DetFlags.readRDOBS.Tile_setOn()
DetFlags.Print()

from AthenaCommon.GlobalFlags import jobproperties
if MC or RUN3: jobproperties.Global.DetDescrVersion = "ATLAS-R3S-2021-03-01-00"
elif RUN2:     jobproperties.Global.DetDescrVersion = "ATLAS-R2-2016-01-00-01"
else:          jobproperties.Global.DetDescrVersion = "ATLAS-R1-2012-03-02-00"
log.info( "DetDescrVersion = %s" % (jobproperties.Global.DetDescrVersion()) )

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
GeoModelSvc = GeoModelSvc()
GeoModelSvc.IgnoreTagDifference = True
log.info( "GeoModelSvc.AtlasVersion = %s" % (GeoModelSvc.AtlasVersion) )

from RecExConfig.RecFlags import rec
if RUN3:   rec.projectName = "data22_tilecomm"
elif RUN2: rec.projectName = "data15_tilecomm"
else:      rec.projectName = "data12_tilecomm"

from IOVDbSvc.CondDB import conddb
if MC:     conddb.setGlobalTag("OFLCOND-MC23-SDR-RUN3-01")
elif RUN3: conddb.setGlobalTag("CONDBR2-BLKPA-2022-09")
elif RUN2: conddb.setGlobalTag("CONDBR2-BLKPA-2018-16")
else:      conddb.setGlobalTag("COMCOND-BLKPA-RUN1-06")

#=============================================================
#=== setup TileConditions
#=============================================================
TileUseCOOL=True
include( "TileConditions/TileConditions_jobOptions.py" )
tileInfoConfigurator.OutputLevel = OutputLevel

#=============================================================
#=== ByteStream Input
#=============================================================
include( "ByteStreamCnvSvc/BSEventStorageEventSelector_jobOptions.py" )
include( "ByteStreamCnvSvcBase/BSAddProvSvc_RDO_jobOptions.py" )
theApp.ExtSvc += [ "ByteStreamCnvSvc" ]
svcMgr.EventSelector.Input += FileNameVec
svcMgr.ByteStreamCnvSvc.ROD2ROBmap = [ "-1" ]
svcMgr.ByteStreamAddressProviderSvc.TypeNames += ["TileBeamElemContainer/TileBeamElemCnt",
                                                  "TileRawChannelContainer/TileRawChannelCnt",
                                                  "TileDigitsContainer/TileDigitsCnt",
                                                  "TileL2Container/TileL2Cnt",
                                                  "TileLaserObject/TileLaserObj" ]


#----------------
# Add Algorithms
#----------------
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

if not 'noDump' in dir():
    from TileTBRec.TileTBRecConf import TileTBDump
    theTileTBDump = TileTBDump()
    topSequence += theTileTBDump
    theTileTBDump.OutputLevel = 2
    if not 'dumpOnce' in dir():
        theTileTBDump.dumpOnce = False
    else:
        theTileTBDump.dumpOnce = True
    theTileTBDump.dumpUnknown = False
    theTileTBDump.global_id = EventNumber
    theTileTBDump.bc_id = BCID
    theTileTBDump.lvl1_id = Level1ID
    theTileTBDump.lvl1_trigger_type = TriggerType

svcMgr.EventSelector.OutputLevel = 2
svcMgr.ByteStreamInputSvc.OutputLevel = 3

if OutputLevel < 2:
    from AthenaCommon.AppMgr import ToolSvc
    from TileByteStream.TileByteStreamConf import TileROD_Decoder
    ToolSvc += TileROD_Decoder()
    ToolSvc.TileROD_Decoder.VerboseOutput = True
    #svcMgr.ByteStreamInputSvc.DumpFlag = True
else:
    svcMgr.ByteStreamInputSvc.DumpFlag = False

svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.EventSelector.MaxBadEvents = 10000
svcMgr.EventSelector.ProcessBadEvent = True
svcMgr.EventSelector.SkipEvents = EvtMin
theApp.EvtMax = EvtMax

