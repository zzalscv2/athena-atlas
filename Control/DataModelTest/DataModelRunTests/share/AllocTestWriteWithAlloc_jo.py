# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
#
# File: share/AllocTestWriteWithAlloc_jo.py
# Author: snyder@bnl.gov
# Date: Nov 2022
# Purpose: Testing an xAOD object with a non-standard memory allocator.
#

## basic job configuration (for generator)
import AthenaCommon.AtlasUnixGeneratorJob

## get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

## get a handle to the ServiceManager
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

## get a handle to the ApplicationManager
from AthenaCommon.AppMgr import theApp

#--------------------------------------------------------------
# Load POOL support
#--------------------------------------------------------------
import AthenaPoolCnvSvc.WriteAthenaPool

include ('DataModelRunTests/loadReadDicts.py')


#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax = 20


#--------------------------------------------------------------
# Set up the algorithm.
#--------------------------------------------------------------

from DataModelTestDataRead.DataModelTestDataReadConf import \
     DMTest__AllocTestWriteWithAlloc
topSequence += DMTest__AllocTestWriteWithAlloc ("AllocTestWriteWithAlloc")


#--------------------------------------------------------------
# Output options
#--------------------------------------------------------------

# ItemList:
include( "EventAthenaPool/EventAthenaPoolItemList_joboptions.py" )
fullItemList+=["DMTest::AllocTestContainer#AllocTest"]
fullItemList+=["DMTest::AllocTestAuxContainer#AllocTestAux."]
fullItemList+=["xAOD::EventInfo#EventInfo"]
fullItemList+=["xAOD::EventAuxInfo#EventInfoAux."]


# Stream's output file
from OutputStreamAthenaPool.MultipleStreamManager import MSMgr
Stream1_Augmented = MSMgr.NewPoolStream ('Stream1', 'alloctestWithAlloc.root', asAlg=True, noTag=True)
Stream1 = Stream1_Augmented.GetEventStream()
Stream1.WritingTool.SubLevelBranchName = '<key>'
# List of DO's to write out
Stream1.ItemList   += fullItemList
ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += ["DEFAULT_SPLITLEVEL='1'"]

from xAODEventFormatCnv.xAODEventFormatCnvConf import xAODMaker__EventFormatStreamHelperTool
for tool in Stream1.HelperTools:
    if isinstance(tool, xAODMaker__EventFormatStreamHelperTool):
        tool.TypeNames += [
            'DataVector<DMTest::AllocTest_v1>',
            'DMTest::AllocTest_v1',
            'DMTest::AllocTestAuxContainer_v1',
        ]
        break


# Avoid races when running tests in parallel.
FILECATALOG = 'AllocTestWriteWithAlloc_catalog.xml'

include ('DataModelRunTests/commonTrailer.py')
