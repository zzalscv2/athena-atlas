#!/bin/env python

# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# testcurrent_tag.py
#==================================================

import sys, getopt

options, remainder = getopt.getopt(sys.argv[1:], 'h', ['help','folder=','globaltag=','instance=','server='])
# defaults
folder=''
globaltag='CURRENT'
instance = 'CONDBR2'
server = ''
help = 0
for opt, arg in options:
    arg = arg.strip()
    if opt in ('-h'):
        help = 1
    elif opt in ('--help'):
        help = 1
    elif opt in ('--folder'):
        folder = arg
    elif opt in ('--globaltag'):
        globaltag = arg
    elif opt in ('--instance'):
        instance = arg
    elif opt in ('--server'):
        server = arg

if help:
    print (' this script prints aliases for CURRENT and NEXT global tags')
    print (' if no options are provided and also resolves leaf tag if')
    print (' folder is provided. By default, if global tag is not specified,')
    print ('  globaltag=CURRENT')
    print ('  default instance=CONDBR2')
    print ('  default server=FRONTIER')
    print (' usage:')
    print (' testcurrent_tag.py --folder=foldername --globaltag=tagname --instance=instancename --server=servername')
    sys.exit()


from TileCalibBlobPython import TileCalibTools
current = TileCalibTools.getAliasFromFile('Current')
nexttag = TileCalibTools.getAliasFromFile('Next')
#--------------------------------
from TileCalibBlobPython.TileCalibLogger import getLogger
log = getLogger("resolve_Tag")
import logging
log.setLevel(logging.DEBUG)

if instance == 'CONDBR2' :
    log.info("alias CURRENT = %s alias NEXT = %s", current, nexttag)

if folder == '':
    sys.exit()

#=================================================
connStr='COOLOFL_TILE/'+instance

#=== open the database
db = TileCalibTools.openDbConn(connStr, server)

#=== resolve folder tag from global tag
foldertag = TileCalibTools.getFolderTag(db, folder, globaltag)
# log.info("leaf TAG %s" % foldertag)
