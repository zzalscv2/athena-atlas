#!/usr/bin/env python
#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Script to create an AthenaAttributeList with a single attribute "xint".
# Usage example: dmtest_condwriter.py --rs=1 --ls=1 'sqlite://;schema=test.db;dbname=OFLP200' AttrList_noTag 42
#

import sys,os
os.environ['CLING_STANDARD_PCH'] = 'none' #See bug ROOT-10789
from PyCool import cool
from CoolConvUtilities import AtlCoolLib, AtlCoolTool

class createTestDB(AtlCoolLib.coolTool):

    def setup(self,args):
        # set values of non-optional parameters
        self.tag=str(args[0])
        self.xint=int(args[1])
        self.folder=args[2] if len(args)>2 else '/DMTest/TestAttrList'

    def usage(self):
        """ Define the additional syntax for options """
        self._usage1()
        print ('TAG xint [Folder]')
        self._usage2()
        
    def execute(self):

        # do update - setup folder specification and create if needed
        spec = cool.RecordSpecification()
        spec.extend("xint", cool.StorageType.Int32)
        print (">== Store object in folder", self.folder)
        cfolder = AtlCoolLib.ensureFolder(self.db, self.folder, spec,
                                          AtlCoolLib.athenaDesc(self.runLumi, 'AthenaAttributeList'),
                                          cool.FolderVersioning.MULTI_VERSION)
        if (cfolder is None): sys.exit(1)
        # now write data
        payload = cool.Record(spec)
        payload['xint'] = self.xint
        print ('>== Store object with IOV [',self.since,',',self.until,'] and tag',self.tag,'xint',self.xint)
        try:
            if (self.tag=="HEAD"):
                cfolder.storeObject(self.since,self.until,payload,0)
            else:
                cfolder.storeObject(self.since,self.until,payload,0,self.tag)
            print (">== Storing COOL object succeeded. Current content:")
        except Exception:
            import traceback
            traceback.print_exc()
            print ('>== Storing COOL object FAILED')
            sys.exit(1)

        # print full content
        act = AtlCoolTool.AtlCoolTool(self.db)
        print (act.more(self.folder))

mytool = createTestDB('dmtest_condwriter.py',False,3,4,[])
