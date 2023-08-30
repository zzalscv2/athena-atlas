#
# File: share/xAODTestRead_jo.py
# Author: snyder@bnl.gov
# Date: May 2014
# Purpose: Test reading xAOD objects data.
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
# Define input
#--------------------------------------------------------------
svcMgr.EventSelector.InputCollections        = [ "xaoddata.root" ]

#--------------------------------------------------------------
# Define output
#--------------------------------------------------------------
# ItemList:
include( "EventAthenaPool/EventAthenaPoolItemList_joboptions.py" )
fullItemList+=["DMTest::CVec#cvec"]
fullItemList+=["xAOD::AuxContainerBase!#cvecAux."]
fullItemList+=["DMTest::CVecWithData#cvecWD"]
fullItemList+=["DMTest::CView#cview"]
fullItemList+=["DMTest::CAuxContainer#cvecWDAux."]
fullItemList+=["DMTest::GVec#gvec"]
fullItemList+=["DMTest::GAuxContainer#gvecAux."]
fullItemList+=["DMTest::CVec#ctrig"]
fullItemList+=["DMTest::CTrigAuxContainer#ctrigAux."]
fullItemList+=["DMTest::C#cinfo"]
fullItemList+=["DMTest::CInfoAuxContainer#cinfoAux."]
fullItemList+=["DMTest::HVec#hvec"]
fullItemList+=["DMTest::HAuxContainer#hvecAux."]
fullItemList+=["DMTest::HView#hview"]

fullItemList+=["DMTest::CVec#copy_cvec"]
fullItemList+=["DMTest::CAuxContainer#copy_cvecAux."]
fullItemList+=["DMTest::CVecWithData#copy_cvecWD"]
fullItemList+=["DMTest::CView#copy_cview"]
fullItemList+=["DMTest::CAuxContainer#copy_cvecWDAux."]
fullItemList+=["DMTest::GVec#copy_gvec"]
fullItemList+=["DMTest::GAuxContainer#copy_gvecAux."]
fullItemList+=["DMTest::CVec#copy_ctrig"]
fullItemList+=["DMTest::CTrigAuxContainer#copy_ctrigAux."]
fullItemList+=["DMTest::C#copy_cinfo"]
fullItemList+=["DMTest::CInfoAuxContainer#copy_cinfoAux."]
fullItemList+=["DMTest::HVec#copy_hvec"]
fullItemList+=["DMTest::HAuxContainer#copy_hvecAux."]
fullItemList+=["DMTest::HView#copy_hview"]

fullItemList+=["DMTest::CVec#scopy_cvec"]
fullItemList+=["xAOD::ShallowAuxContainer#scopy_cvecAux."]
fullItemList+=["DMTest::CVecWithData#scopy_cvecWD"]
fullItemList+=["xAOD::ShallowAuxContainer#scopy_cvecWDAux."]
fullItemList+=["DMTest::CVec#scopy_ctrig"]
fullItemList+=["xAOD::ShallowAuxContainer#scopy_ctrigAux."]
fullItemList+=["DMTest::C#scopy_cinfo"]
fullItemList+=["xAOD::ShallowAuxInfo#scopy_cinfoAux."]
fullItemList+=["DMTest::HVec#scopy_hvec"]
fullItemList+=["xAOD::ShallowAuxContainer#scopy_hvecAux."]

fullItemList+=["xAOD::EventInfo#EventInfo"]
fullItemList+=["xAOD::EventAuxInfo#EventInfoAux."]

ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += ["DEFAULT_SPLITLEVEL='1'"]

from OutputStreamAthenaPool.MultipleStreamManager import MSMgr


#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax = 20

#--------------------------------------------------------------
# Application:
#--------------------------------------------------------------

from DataModelTestDataCommon.DataModelTestDataCommonConf import \
     DMTest__xAODTestReadCVec, \
     DMTest__xAODTestDecor, \
     DMTest__xAODTestShallowCopy
from DataModelTestDataRead.DataModelTestDataReadConf import \
     DMTest__xAODTestReadCInfo, \
     DMTest__xAODTestRead, \
     DMTest__xAODTestReadCView, \
     DMTest__xAODTestReadHVec, \
     DMTest__xAODTestClearDecor, \
     DMTest__xAODTestShallowCopyHVec


topSequence += DMTest__xAODTestReadCVec ('xAODTestReadCVec',
                                         WriteKey = 'copy_cvec')
topSequence += DMTest__xAODTestReadCInfo ('xAODTestReadCInfo',
                                          WriteKey = 'copy_cinfo')
topSequence += DMTest__xAODTestRead ('xAODTestRead',
                                     CTrigWriteKey = 'copy_ctrig',
                                     GVecWriteKey = 'copy_gvec',
                                     CVecWDWriteKey = 'copy_cvecWD')
topSequence += DMTest__xAODTestReadCView ('xAODTestReadCView',
                                          WriteKey = 'copy_cview')
topSequence += DMTest__xAODTestReadHVec ('xAODTestReadHVec',
                                         VecWriteKey = 'copy_hvec',
                                         ViewWriteKey = 'copy_hview')

topSequence += DMTest__xAODTestDecor ('AuxDataTestDecor1',
                                      DecorName = 'dInt100',
                                      Offset = 100)

topSequence += DMTest__xAODTestShallowCopy ('xAODTestShallowCopy')
topSequence += DMTest__xAODTestShallowCopyHVec ('xAODTestShallowCopyHVec')

topSequence += DMTest__xAODTestDecor ('AuxDataTestDecor1_scopy',
                                      ReadPrefix = 'scopy_',
                                      DecorName = 'dInt150',
                                      Offset = 300)

topSequence += DMTest__xAODTestReadCVec ('xAODTestReadFwdLink',
                                         CVecKey = 'CVecFwdLink')

# Stream's output file
Stream1_Augmented = MSMgr.NewPoolStream ('Stream1', 'xaoddata2.root',asAlg=True, noTag=True)
Stream1 = Stream1_Augmented.GetEventStream()
Stream1.WritingTool.SubLevelBranchName = '<key>'
Stream1.ItemList   += fullItemList # List of DO's to write out

from xAODEventFormatCnv.xAODEventFormatCnvConf import xAODMaker__EventFormatStreamHelperTool
for tool in Stream1.HelperTools:
    if isinstance(tool, xAODMaker__EventFormatStreamHelperTool):
        tool.TypeNames += [
            'DataVector<DMTest::C_v1>',
            'DMTest::CAuxContainer_v1',
            'DMTest::CVecWithData_v1',
            'DMTest::CTrigAuxContainer_v1',
            'DMTest::C_v1',
            'DMTest::CInfoAuxContainer_v1',
        ]
        break

# Avoid races when running tests in parallel.
FILECATALOG = 'xAODTestRead_catalog.xml'

include ('DataModelRunTests/commonTrailer.py')
