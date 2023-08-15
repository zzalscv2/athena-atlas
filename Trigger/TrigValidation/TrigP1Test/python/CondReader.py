# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Test stop/start for conditions that are updated during the run.
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders
from pathlib import Path

def run(flags):
   """Configure conditions reader"""

   flags.lock()
   cfg = ComponentAccumulator()

   from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
   cfg.merge(SGInputLoaderCfg(flags, [('xAOD::EventInfo', 'StoreGateSvc+EventInfo')]))

   # addFolders requires at least an empty sqlite file
   Path('cond.db').touch()

   # These folders are filled in Testing/condStopStart.trans
   cfg.merge( addFolders(flags, ['/DMTest/TestAttrList', '/DMTest/TestAttrListTime'],
                         detDb='cond.db',
                         tag='HEAD',
                         className='AthenaAttributeList') )

   # Readers for run-based and time-based folders
   cfg.addEventAlgo( CompFactory.DMTest.CondReaderAlg("CondReaderAlg1",
                                                      AttrListKey = "/DMTest/TestAttrList",
                                                      S2Key = "") )
   cfg.addEventAlgo( CompFactory.DMTest.CondReaderAlg("CondReaderAlg2",
                                                      AttrListKey = "/DMTest/TestAttrListTime",
                                                      S2Key = "") )
   # A dummy CondAlg
   cfg.addCondAlgo( CompFactory.DMTest.CondAlg1() )

   return cfg
