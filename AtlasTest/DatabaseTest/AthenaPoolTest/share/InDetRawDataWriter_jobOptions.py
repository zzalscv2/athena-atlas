###############################################################
#
# Job options file
#
## @file InDetRawDataWriter_jobOptions.py
##
## @brief For Athena POOL test: write out InDetRawDataContainers
##
## @author RD Schaffer <R.D.Schaffer@cern.ch>
#
#==============================================================
# MC Event Selector
## basic job configuration (for generator)
import AthenaCommon.AtlasUnixGeneratorJob

## get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

## get a handle to the ServiceManager
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

## get a handle to the ApplicationManager
from AthenaCommon.AppMgr import theApp

from AthenaCommon.GlobalFlags  import globalflags
from RecExConfig.RecFlags import rec


#--------------------------------------------------------------
# Set flags and load det descr
#--------------------------------------------------------------
from AthenaCommon.GlobalFlags  import globalflags
from AthenaCommon.DetFlags     import DetFlags
from RecExConfig.RecFlags      import rec

# For general flags
rec.doAOD       = False
rec.doTrigger   = False
rec.doWriteTAG  = False
DetDescrVersion = "ATLAS-GEO-17-00-00"

# Set local flags - only need LAr DetDescr
DetFlags.detdescr.ID_setOn()
DetFlags.detdescr.Calo_setOff()
DetFlags.detdescr.Tile_setOff()
DetFlags.detdescr.Muon_setOff()
      
# set up all detector description description 
include ("RecExCond/AllDet_detDescr.py")

# the correct tag should be specified
from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag("OFLCOND-SDR-BS7T-04-00")

#--------------------------------------------------------------
# Load POOL support
#--------------------------------------------------------------
import AthenaPoolCnvSvc.WriteAthenaPool

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax = 20
#--------------------------------------------------------------
# Application: AthenaPoolTest InDetRawData options
#--------------------------------------------------------------

from AthenaPoolTest.AthenaPoolTestConf import InDetRawDataFakeWriter
topSequence += InDetRawDataFakeWriter( "InDetRawDataFakeWriter" )

#--------------------------------------------------------------
# Output options
#--------------------------------------------------------------

# Stream's output file
from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
Stream1 = AthenaPoolOutputStream( "Stream1", noTag=True )
Stream1.OutputFile =   "InDetRDO.root"
# List of DO's to write out
Stream1.ItemList+=["PixelRDO_Container#*"]
Stream1.ItemList+=["SCT_RDO_Container#*"]
Stream1.ItemList+=["TRT_RDO_Container#*"]
Stream1.ItemList+=["EventInfo#*"]
Stream1.ItemList+=["PixelRDOElemLinkVec#*"]
#--------------------------------------------------------------
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
#--------------------------------------------------------------
svcMgr.MessageSvc.OutputLevel = WARNING
svcMgr.MessageSvc.debugLimit  = 100000
svcMgr.MessageSvc.errorLimit  = 100000
InDetRawDataFakeWriter.OutputLevel = DEBUG

from AthenaServices import AthenaServicesConf
AthenaEventLoopMgr = AthenaServicesConf.AthenaEventLoopMgr()
AthenaEventLoopMgr.OutputLevel = INFO

# No stats printout
include( "AthenaPoolTest/NoStats_jobOptions.py" )

#==============================================================
#
# End of job options file
#
###############################################################
