#=============================================================
# JopOptions file to test TileMuId2DBAlg
# File: TileCalibAlgs/jobOptions_TileMuId2DBTest.py
#=============================================================

#=== get user options or set default
if not 'RUN' in dir():
    RUN = 999999
RunNumber = RUN

import AthenaCommon.AtlasUnixGeneratorJob
from AthenaCommon import CfgMgr
from AthenaCommon.AppMgr import theApp
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
from AthenaCommon.AppMgr import ToolSvc

#=============================================================
# Global Flags
#=============================================================
from AthenaCommon.GlobalFlags import globalflags
globalflags.DetGeo.set_Value_and_Lock('atlas')
globalflags.Luminosity.set_Value_and_Lock('zero')
globalflags.DataSource.set_Value_and_Lock('data')
globalflags.InputFormat.set_Value_and_Lock('bytestream')
globalflags.DatabaseInstance="CONDBR2"

TileUseDCS = False

#=============================================================
# Geometry setup
#=============================================================
from AthenaCommon.DetFlags import DetFlags
DetFlags.detdescr.ID_setOff()
DetFlags.detdescr.Muon_setOff()
DetFlags.detdescr.LAr_setOn()
DetFlags.detdescr.Tile_setOn()

#--- see https://atlas-geometry-db.web.cern.ch/atlas-geometry-db/
#--- for the geometry updates
from AthenaCommon.JobProperties import jobproperties
jobproperties.Global.DetDescrVersion = "ATLAS-R3S-2021-02-00-00"
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

#=============================================================
# set global tag
#=============================================================
from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag("CONDBR2-BLKPA-2023-01")

#=============================================================
# Add includes
#=============================================================
include( "CaloConditions/CaloConditions_jobOptions.py")
include( "CaloIdCnv/CaloIdCnv_joboptions.py" )
include( "TileIdCnv/TileIdCnv_jobOptions.py" )
include( "LArDetDescr/LArDetDescr_joboptions.py" )
include( "TileConditions/TileConditions_jobOptions.py" )
include( "LArConditionsCommon/LArConditionsCommon_comm_jobOptions.py")

#=============================================================
# Add CaloNoise
#=============================================================
from CaloTools.CaloNoiseCondAlg import CaloNoiseCondAlg
CaloNoiseCondAlg ('totalNoise')

#============================================================
# Add TileMuId2DBAlg
#============================================================
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()
from TileCalibAlgs.TileCalibAlgsConf import TileMuId2DBAlg
theTileMuId2DBAlg = TileMuId2DBAlg("TileMuId2DBAlg")
topSequence += theTileMuId2DBAlg

#============================================================
# MessageSvc setup
#============================================================
MessageSvc = Service( "MessageSvc" )
MessageSvc.OutputLevel = INFO

#============================================================
# Dummy event loop setup
#============================================================
svcMgr.EventSelector.RunNumber         = RUN
svcMgr.EventSelector.EventsPerRun      = 5
svcMgr.EventSelector.FirstEvent        = 1
svcMgr.EventSelector.EventsPerLB       = 5
svcMgr.EventSelector.FirstLB           = 1
svcMgr.EventSelector.InitialTimeStamp  = 0
svcMgr.EventSelector.TimeStampInterval = 5
theApp.EvtMax                          = 1

#============================================================
# Print out job summary
#============================================================
print(svcMgr)
print(topSequence)
