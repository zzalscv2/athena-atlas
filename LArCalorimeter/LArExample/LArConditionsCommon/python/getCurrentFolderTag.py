#!/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys

from PyCool import cool
sys.path.append('/afs/cern.ch/user/a/atlcond/utils22')
from CondUtilsLib.AtlCoolBKLib import resolveAlias

def getCurrentFolderTag(dbname,folderName,ES=False):
    current=None
    next=None

    #1. Get current and next global tags using resolver class in ~atlcond
    resolver=resolveAlias()
    if(ES):
       currentGlobal=resolver.getCurrentES().replace("*","ST")
       nextGlobal=resolver.getNextES().replace("*","ST")
    else:   
       currentGlobal=resolver.getCurrent().replace("*","ST")
       nextGlobal=resolver.getNext().replace("*","ST")

    print('currentGlobal: ',currentGlobal)
    #2. Open the DB to resolve this gobal tag for the given folder
    dbSvc = cool.DatabaseSvcFactory.databaseService() 
    db = dbSvc.openDatabase(dbname)
    f=db.getFolder(folderName)
    try:
        current=f.resolveTag(currentGlobal)
    except Exception:
        print('Warning: could not resolve ',currentGlobal,' in db: ',dbname)
        if "DBR2" in dbname:
           print('resolving for the global CONDBR2-BLKPA-2022-10')
           tmpGlobal='CONDBR2-BLKPA-2022-10'
        else:
           print('resolving for the global COMCOND-BLKPA-RUN1-06')
           tmpGlobal='COMCOND-BLKPA-RUN1-06'

        try:
           current=f.resolveTag(tmpGlobal)
        except Exception:
           print('Also not working, giving up')
           pass
        pass

    if len(nextGlobal)>2:
        # NEXT exists, try to resolve it
        try:
            next=f.resolveTag(nextGlobal)
        except Exception:
            pass
        pass
    
    db.closeDatabase()
    return (current,next)


if __name__=="__main__":
    if len(sys.argv)<3:
        sys.stderr.write("Usage: %s dbname folder\n" %  sys.argv[0].split("/")[-1])
        sys.exit(-1)

    dn=sys.argv[1]
    fn=sys.argv[2]
    if len(sys.argv)>3:
       estag=sys.argv[3]
    else:   
       estag=False

    currTag=getCurrentFolderTag(dn,fn,estag)[0]
    
    if currTag is None:
        sys.stderr.write("Failed to resolve current folder-level tag for folder %s in db %s\n" % (fn,dn))
        sys.exit(-1)

    print(currTag)
    
