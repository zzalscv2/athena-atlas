## @file L1Topo_ReadBS_test.py
## @brief Example job options file to read BS file to test a converter
## $Id: $
###############################################################
#
# This Job option:
# ----------------
# 1. Read ByteStream test data file and decode the L1Topo part
#
#==============================================================

## basic job configuration
import AthenaCommon.AtlasUnixStandardJob

include( "ByteStreamCnvSvc/BSEventStorageEventSelector_jobOptions.py" )

## get a handle on the ServiceManager
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

## get a handle on the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# older file, has wrong ROD id: svcMgr.ByteStreamInputSvc.FullFileName = [ "/afs/cern.ch/user/m/mzinser/public/l1calo-event.sim"]
#svcMgr.ByteStreamInputSvc.FullFileName = [ "/afs/cern.ch/user/m/mzinser/public/InputSimon/Mode7/l1calo-event.sim" ]
svcMgr.ByteStreamInputSvc.FullFileName = [ "/afs/cern.ch/user/m/mzinser/public/InputSimon/Mode8/l1calo-event.sim" ]
#svcMgr.ByteStreamInputSvc.FullFileName = [ "/afs/cern.ch/user/s/sgeorge/atlaspublic/L1TopoCnv/mergedsim._0001.data" ]

svcMgr.ByteStreamInputSvc.ValidateEvent = True
svcMgr.EventSelector.ProcessBadEvent = True


from ByteStreamCnvSvc.ByteStreamCnvSvcConf import ByteStreamCnvSvc    
ByteStreamCnvSvc = ByteStreamCnvSvc("ByteStreamCnvSvc", IsSimulation = True)
# not needed offline? 
ByteStreamCnvSvc.InitCnvs += ["L1TopoRDOCollection"]

# set source IDs for L1Topo DAQ RODs 

#   Not needed unless changing something:
from L1TopoByteStream.L1TopoByteStreamConf import L1TopoByteStreamTool
l1TopoByteStreamTool=L1TopoByteStreamTool()
# l1TopoByteStreamTool.ROBSourceIDs=[0x00910000,0x00910010]
#   Not needed unless to change the default, which are generated by 
#   L1TopoByteStreamTool::sourceIDs called from L1TopoByteStreamCnv
# Other options:
# l1TopoByteStreamTool.decodeDAQROBs=True  # default
# l1TopoByteStreamTool.decodeROIROBs=False # default
l1TopoByteStreamTool.OutputLevel=1
svcMgr.ToolSvc+=l1TopoByteStreamTool

if not hasattr( svcMgr, "ByteStreamAddressProviderSvc" ):
    from ByteStreamCnvSvcBase.ByteStreamCnvSvcBaseConf import ByteStreamAddressProviderSvc 
    svcMgr += ByteStreamAddressProviderSvc()

# L1Topo  Source IDs:
# The lower 8-bits of the ModuleIDs are specified by the bitfield rmmmssss, where ssss is the Slink fibre number (four bits allowing values 0-15) and mmm is the L1Topo module number (three bits allowing values 0-7) and r is a one bit flag distingushing RoI fibres (1) from DAQ fibres (0).
# Bits 8-15 are zero.
# For the full source ID, add the detector id 0x91 in byte 2.
#  
# L1Topo module#0, link #0 (DAQ) := 0x91 + 0x00 + 0 000 0000 [ie:0x00910000]
# L1Topo module#0, link #1 (RoI) := 0x91 + 0x00 + 1 000 0001 [ie:0x00910081]
# L1Topo module#1, link #0 (DAQ) := 0x91 + 0x00 + 0 001 0000 [ie:0x00910010]
# L1Topo module#1, link #1 (RoI) := 0x91 + 0x00 + 1 001 0001 [ie:0x00910091] 
# links 2 are planned for use with the new RoIB; they would also be ROI type
#
# Test with ROI modules - not needed as it is the default
# svcMgr.ByteStreamAddressProviderSvc.TopoProcModuleID=[0x81,0x91]
# Test with DAQ modules - debugging only!
# svcMgr.ByteStreamAddressProviderSvc.TopoProcModuleID=[0x00,0x10]

# This is the list of proxies to set up so that retrieval attempt will trigger the BS conversion
svcMgr.ByteStreamAddressProviderSvc.TypeNames += [
    "ROIB::RoIBResult/RoIBResult",
    "MuCTPI_RDO/MUCTPI_RDO",
    "CTP_RDO/CTP_RDO",
    "MuCTPI_RIO/MUCTPI_RIO",
    "L1TopoRDOCollection/L1TopoRDOCollection"
    ]

#--------------------------------------------------------------
# Private Application Configuration options
#--------------------------------------------------------------
# Load "user algorithm" top algorithms to be run, and the libraries that house them


from L1TopoMonitoring.L1TopoMonitoringConf import L1TopoTestAlg
topSequence += L1TopoTestAlg("MyL1TopoTest")

# dump content of StoreGate event store; useful for debugging
from AthenaPoolMultiTest.AthenaPoolMultiTestConf import StoreDump
sd = StoreDump("StoreDump")
sd.StoreName="StoreGateSvc"
topSequence += sd

#--------------------------------------------------------------
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
#--------------------------------------------------------------
svcMgr.MessageSvc.OutputLevel = 1
svcMgr.MessageSvc.Format = "% F%42W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.verboseLimit = 0
topSequence.MyL1TopoTest.OutputLevel = 1
svcMgr.StoreGateSvc.OutputLevel = 1
svcMgr.ByteStreamAddressProviderSvc.OutputLevel = 1
svcMgr.ByteStreamCnvSvc.OutputLevel = 1
svcMgr.ByteStreamInputSvc.OutputLevel = 1
svcMgr.AthDictLoaderSvc.OutputLevel = 1
svcMgr.EventPersistencySvc.OutputLevel = 1
svcMgr.ROBDataProviderSvc.OutputLevel = 1
#svcMgr.DataModelCompatSvc.OutputLevel = 4
print svcMgr


print "ZZZ2 topSequence:", topSequence
#
# End of job options file
#
###############################################################
