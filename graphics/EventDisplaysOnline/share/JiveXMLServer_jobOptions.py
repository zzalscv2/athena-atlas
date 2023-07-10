 #########################################
#
# Setup for running event displays at point1
# 
#########################################

import os
from random import randrange
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

######## Commented out by sjiggins - 19-02-2015 because of a ############### 
######## writeESD crash with PoolFileCatalog read only error ###############

#randomString=str(randrange(0,100000))
#athenaCommonFlags.PoolESDOutput = 'ESD_' + randomString +'.pool.root'
#os.environ['POOL_CATALOG'] = 'xmlcatalog_file:PoolFileCatalog_'+randomString+'.xml'

############################################################################

#Add the online event display service
from AthenaCommon.AppMgr import theApp
if not hasattr(svcMgr, 'OnlineEventDisplaysSvc'):
    if rec.doHeavyIon:
        maxEvents=200
        projectTags=['data23_hi']
        publicStreams=['physics_MinBias']
    else:
        maxEvents=100
        projectTags=['data23_13p6TeV']
        publicStreams=['physics_Main']



    from EventDisplaysOnline.OnlineEventDisplaysSvc import OnlineEventDisplaysSvc
    svcMgr += OnlineEventDisplaysSvc(
        "OnlineEventDisplaysSvc", 
        OutputLevel = DEBUG,               # Verbosity
        MaxEvents = maxEvents,             # Number of events to keep per stream
        OutputDirectory = OutputDirectory, # Base directory for streams
        ProjectTags = projectTags,         # Project tags that are allowed to be made public
        Public = publicStreams,            # These streams go into public stream when Ready4Physics
    )
    theApp.CreateSvc += [svcMgr.OnlineEventDisplaysSvc.getFullJobOptName()]

