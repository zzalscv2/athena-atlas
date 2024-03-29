
# Author : Ketevi A. Assamagan

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthenaCommon.AppMgr import ServiceMgr

include( "AthenaCommon/Atlas_Gen.UnixStandardJob.py" )
include( "PartPropSvc/PartPropSvc.py" )

# AthenaPool converters
include( "EventAthenaPool/EventAthenaPool_joboptions.py" )
include( "GeneratorObjectsAthenaPool/GeneratorObjectsAthenaPool_joboptions.py" )

#include( "AthenaPoolCnvSvc/WriteAthenaPool_jobOptions.py" )
from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
Stream1 = AthenaPoolOutputStream("StreamEVGEN")

Stream1.ItemList  += [ "EventInfo#*", "McEventCollection#*" ]
Stream1.OutputFile = "mc13.007901.cavernbg_sf01_25ns.evgen._0001.pool.root"

from CavernBkgGenerator.CavernBkgGeneratorConf import MuonBackgroundConverter
topSequence += MuonBackgroundConverter( "MuonBackgroundConverter" )

MuonCavern = topSequence.MuonBackgroundConverter

#--------------------------------------------------------------
# Set output level threshold (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
#--------------------------------------------------------------
ServiceMgr.MessageSvc.OutputLevel  = INFO
ServiceMgr.MessageSvc.defaultLimit = 100000

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
# Number of events to be processed (default is 10)
# theApp.EvtMax = 28166
theApp.EvtMax = 25000

#--------------------------------------------------------------
# MuonBackgroundConverter parameters
#--------------------------------------------------------------
#
MuonCavern.eventfile         = "/afs/cern.ch/user/k/ketevi/w0/data/partflux.fvpn21n-37n.txt"
MuonCavern.safetyFactor      = 1
MuonCavern.firstHitOnly      = True
MuonCavern.BunchCrossingTime = 25*ns
MuonCavern.PhiSymN           = 8
MuonCavern.OutputLevel       = INFO

