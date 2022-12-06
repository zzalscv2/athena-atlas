# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from PyCool import cool
from AthenaCommon.Logging import logging


class FolderTagResolver:

    _globalTag=""

    def __init__(self,dbname="COOLOFL_LAR/CONDBR2"):
        dbSvc = cool.DatabaseSvcFactory.databaseService()
        self._db = dbSvc.openDatabase(dbname)
        self._msg=logging.getLogger('FolderTagResolver')
        return

    def __del__(self):
        self._db.closeDatabase()
        return

    def getFolderTag(self,foldername,globalTag=None):
        if globalTag is None:
            globalTag=self.__class__._globalTag

        self._msg.info("Looking up folder level tag for %s using globalTag %s",foldername,globalTag)
        try:
          folder=self._db.getFolder(foldername)
          return folder.resolveTag(globalTag)
        except Exception as e:
          self._msg.error("Could not resolve tag %s for folder %s in database %s",globalTag,foldername,self._db.databaseId())
          print (e)
          # new folder, should "create a tag"
          return ''.join(foldername.split('/')) + '-RUN2-UPD3-00'
          
            
    def getFolderTagSuffix(self,foldername,globalTag=None):
        ft=self.getFolderTag(foldername,globalTag)
        p=ft.find("-")
        if p==-1:
            return "-Default"
        else: 
            return ft[p:]
