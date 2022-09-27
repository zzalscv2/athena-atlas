#!/bin/env python

# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

# TODO: check for overflows of counters!
# TODO: option to print out counters instead of rates

import sys,getopt,os
from PyCool import cool,coral
from CoolConvUtilities import AtlCoolTool
from ROOT import TH1F, TFile, TProfile, TObject, TGraph

def usage():
    print "%s" % sys.argv[0]
    print "  --help                                this printout"
    print "  --run <runNo>                         run number"
    print "  --lb <lb1>-<lb2>,<lb3>-<lb4>,         Range of LBs"


try:
    longopts=['run=','lb=','item=','help']
    opts,args=getopt.getopt(sys.argv[1:],'',longopts)
except getopt.GetoptError,e:
    usage()
    sys.exit(2)

lbList=[]
run=int(-1)
liveTime = 0

for o,a in opts:
    if o in ('-h','--help'):
        usage()
        sys.exit(0)
    if o == '--run':
        run = int(a)
    if o == '--lb':
        list1 = str(a).split(',')
        for i in list1:
            list2 = i.split('-')
            if len(list2) > 1 : lbList += range(int(list2[0]),int(list2[1])+1)
            else : lbList.append(int(list2[0]))

if (run == -1) : 
    usage()
    sys.exit(2)

# get database service and open database
dbSvc=cool.DatabaseSvcFactory.databaseService()
# Get user and passwd from coral auth file - see: https://its.cern.ch/jira/browse/ATR-25572
# see this package in case the following doesn't work for you: Database/ConnectionManagement/AtlasAuthentication/data/dblookup.xml
connName="oracle://ATLAS_COOLPROD/ATLAS_COOLONL_TRIGGER" # aka. schema
dbauthPath=os.environ.get('CORAL_AUTH_PATH')
dbpw = None
dbuser = None
# Get username and password by parsing authentication XML file
authenticationXmlDoc = minidom.parse(dbauthPath + "authentication.xml")
authenticationList = authenticationXmlDoc.getElementsByTagName("connection")
for entry in authenticationList:
    if entry.attributes["name"].value == connName:
        parameters = entry.getElementsByTagName("parameter")
        # Set username and password from authentication parameters
        for parameter in parameters:
            if parameter.attributes["name"].value == "user":
                dbuser = parameter.attributes["value"].value
            elif parameter.attributes["name"].value == "password":
                dbpw = parameter.attributes["value"].value
dbstring="oracle://ATLAS_COOLPROD;schema=ATLAS_COOLONL_TRIGGER;dbname=COMP200;user="+dbuser+";password="+dbpw
#olddbstring="oracle://ATLAS_COOLPROD;schema=ATLAS_COOLONL_TRIGGER;dbname=COMP200;user=ATLAS_COOL_READER;password=PAssWDusedTObeHEREinPLAINtext"

try:
    db=dbSvc.openDatabase(dbstring,False)
except Exception,e:
    print e
    sys.exit(-1)

# set folders all run-event index
# AthenaAttributeList as type 0, CondAttributeList collection as type 1
# timestamp for folders bit 1

lblb_foldername='/TRIGGER/LUMI/LBLB'

if not db.existsFolder(lblb_foldername) :
    print "Folder",lblb_foldername,"not found"
    db.closeDatabase()
    sys.exit(-1)
         
folder=db.getFolder(lblb_foldername)
try:
    itr=folder.browseObjects(run << 32,(run+1) << 32,cool.ChannelSelection.all())
    while itr.goToNext() :
        obj=itr.currentRef()
        payload=obj.payload()
        lb=(obj.since() & 0xffff)

        if lb in lbList:
            liveTime += 1e-9*(payload['EndTime'] - payload['StartTime'])
            
    itr.close()
except Exception,e:
    print "Reading data from",lvl1lbdata_foldername,"failed:",e

print "Live time",liveTime,"s"

db.closeDatabase()
