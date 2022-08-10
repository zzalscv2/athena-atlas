# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Test stop/start for conditions that are updated during the run.
#

from AthenaCommon.AlgSequence import AlgSequence, AthSequencer

#--------------------------------------------------------------
# Conditions setup.
#--------------------------------------------------------------
from IOVDbSvc.CondDB import conddb
from AthenaCommon.GlobalFlags import globalflags
globalflags.DataSource = 'data'
globalflags.ConditionsTag = 'CONDBR2-HLTP-2018-01'
conddb.setGlobalTag(globalflags.ConditionsTag())

# These folders are filled in Testing/condStopStart.trans
conddb.addFolder ('cond.db', '/DMTest/TestAttrList <tag>HEAD</tag>',
                  className='AthenaAttributeList')
conddb.addFolder ('cond.db', '/DMTest/TestAttrListTime <tag>HEAD</tag>',
                  className='AthenaAttributeList')

#--------------------------------------------------------------
# Setup AlgSequence
#--------------------------------------------------------------
from DataModelTestDataCommon.DataModelTestDataCommonConf import (DMTest__CondReaderAlg,
                                                                 DMTest__CondAlg1)
topSequence = AlgSequence()
condSeq = AthSequencer("AthCondSeq")

# Required by CondReaderAlg
topSequence.SGInputLoader.Load += [( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )]

# Reader for run-based and time-based folders
topSequence += DMTest__CondReaderAlg( "CondReaderAlg1",
                                      AttrListKey = "/DMTest/TestAttrList",
                                      S2Key = "")

topSequence += DMTest__CondReaderAlg( "CondReaderAlg2",
                                      AttrListKey = "/DMTest/TestAttrListTime",
                                      S2Key = "")

# A dummy CondAlg
condSeq += DMTest__CondAlg1()
