# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
#
# File: share/AllocTestReadWithAlloc_jo.py
# Author: snyder@bnl.gov
# Date: Nov 2022
# Purpose: Testing an xAOD object with a non-standard memory allocator.
#

## basic job configuration (for generator)
import AthenaCommon.AtlasUnixStandardJob

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
import AthenaPoolCnvSvc.ReadAthenaPool

include ('DataModelRunTests/loadReadDicts.py')

#--------------------------------------------------------------
# Input defined on the command line.
#--------------------------------------------------------------
#svcMgr.EventSelector.InputCollections        = [ "alloctest2.root" ]

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax = 20

#--------------------------------------------------------------
# Application:
#--------------------------------------------------------------

from DataModelTestDataRead.DataModelTestDataReadConf import \
     DMTest__AllocTestReadWithAlloc


topSequence += DMTest__AllocTestReadWithAlloc ('AllocTestReadWithAlloc')


# Avoid races when running tests in parallel.
FILECATALOG = 'AllocTestReadWithAlloc' + os.path.splitext (svcMgr.EventSelector.InputCollections[0])[0] +'_catalog.xml'

include ('DataModelRunTests/commonTrailer.py')
