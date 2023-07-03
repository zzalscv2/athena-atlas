#!/bin/env python

# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# ReadFloatFromCool.py
# Lukas Pribyl <lukas.pribyl@cern.ch>, 2008-11-18

import getopt,sys,os
os.environ['TERM'] = 'linux'

def usage():
    print ("Usage: ",sys.argv[0]," [OPTION] ... ")
    print ("Dumps the TileCal constants from various schemas / folders / tags")
    print ("")
    print ("-h, --help      shows this help")
    print ("-f, --folder=   specify status folder to use f.i. /TILE/OFL02/CALIB/CIS/LIN ")
    print ("-t, --tag=      specify tag to use, f.i. UPD1 or UPD4 or full suffix like RUN2-HLT-UPD1-00")
    print ("-r, --run=      specify run  number, by default uses latest iov")
    print ("-l, --lumi=     specify lumi block number, default is 0")
    print ("-p, --ros=      specify partition (ros number), default is 1")
    print ("-d, --drawer=   specify drawer number, default is 0")
    print ("-c, --channel=  specify channel number, default is 0")
    print ("-g, -a, --adc=  specify gain (adc number), default is 0")
    print ("-n, --nval=     specify number of values to output, default is all")
    print ("-s, --schema=   specify schema to use, like 'COOLOFL_TILE/CONDBR2' or 'sqlite://;schema=tileSqlite.db;dbname=CONDBR2'")
    print ("-S, --server=   specify server - ORACLE or FRONTIER, default is FRONTIER")

letters = "hr:l:S:s:t:f:p:d:c:a:g:n:"
keywords = ["help","run=","lumi=","server=","schema=","tag=","folder=","ros=","drawer=","channel=","adc=","gain=","nval="]

try:
    opts, extraparams = getopt.getopt(sys.argv[1:],letters,keywords)
except getopt.GetoptError as err:
    print (str(err))
    usage()
    sys.exit(2)

# defaults
run = 2147483647
lumi = 0
server = ''
schema = 'COOLOFL_TILE/CONDBR2'
folderPath =  "/TILE/OFL02/CALIB/CIS/LIN"
tag = "UPD4"
ros     = 1
drawer  = 0
channel = 0
adc     = 0
nval    = 0
# note: for MUID there are 40 values stored at every channel 0

for o, a in opts:
    a = a.strip()
    if o in ("-f","--folder"):
        folderPath = a
    elif o in ("-t","--tag"):
        tag = a
    elif o in ("-S","--server"):
        server = a
    elif o in ("-s","--schema"):
        schema = a
    elif o in ("-p","--ros"):
        ros = int(a)
    elif o in ("-d","--drawer"):
        drawer = int(a)
    elif o in ("-c","--channel"):
        channel = int(a)
    elif o in ("-a","--adc","-g","--gain"):
        adc = int(a)
    elif o in ("-n","--nval"):
        nval = int(a)
    elif o in ("-r","--run"):
        run = int(a)
    elif o in ("-l","--lumi"):
        lumi = int(a)
    elif o in ("-h","--help"):
        usage()
        sys.exit(2)
    else:
        assert False, "unhandeled option"


if schema=='COOLONL_TILE/COMP200':
    if '/TILE/ONL01' not in folderPath and '/TILE/OFL01' not in folderPath:
        print ("Folder %s doesn't exist in schema %s " % (folderPath,schema) )
        sys.exit(2)

if schema=='COOLONL_TILE/CONDBR2':
    if '/TILE/ONL01' not in folderPath:
        print ("Folder %s doesn't exist in schema %s " % (folderPath,schema) )
        sys.exit(2)

if schema=='COOLOFL_TILE/COMP200' or schema=='COOLOFL_TILE/CONDBR2':
    if '/TILE/OFL02' not in folderPath:
        print ("Folder %s doesn't exist in schema %s " % (folderPath,schema) )
        sys.exit(2)


from TileCalibBlobPython import TileCalibTools
from TileCalibBlobObjs.Classes import TileCalibUtils

from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("ReadFloat")
import logging
log.setLevel(logging.DEBUG)


#=== set database
db = TileCalibTools.openDbConn(schema,server)
folderTag = TileCalibTools.getFolderTag(db, folderPath, tag)
log.info("Initializing folder %s with tag %s", folderPath, folderTag)

#=== initialize blob reader
blobReader = TileCalibTools.TileBlobReader(db,folderPath, folderTag)
#blobReader.log().setLevel(logging.DEBUG)

#=== get drawer with status at given run
log.info("Initializing ros %d, drawer %d for run %d, lumiblock %d", ros,drawer,run,lumi)
log.info("... %s", blobReader.getComment((run,lumi)))
log.info( "\n" )

#=== get float for a given ADC
flt = blobReader.getDrawer(ros, drawer,(run,lumi))
modName = TileCalibUtils.getDrawerString(ros,drawer)
msg = "%s ch %i gn %i :" % ( modName, channel, adc )
if nval<1:
    nval = flt.getObjSizeUint32()
for val in range(0,nval):
    msg += "  %f" % flt.getData(channel, adc, val)
print (msg)

#=== close DB
db.closeDatabase()
