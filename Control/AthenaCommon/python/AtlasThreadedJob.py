# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

## @file AtlasThreadedJob.py
## @brief py-module to configure the Athena AppMgr for threaded (Hive) jobs
## @author Charles Leggett
###############################################################

def _setupAtlasThreadedJob():
    from AthenaCommon.AppMgr import theApp
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    from AthenaCommon import Constants

    from AthenaCommon.ConcurrencyFlags import jobproperties as jps

    if (jps.ConcurrencyFlags.NumProcs() == 0) :
        theApp.MessageSvcType = "InertMessageSvc"
    else:
        # InertMessageSvc doesn't play nice with MP
        theApp.MessageSvcType = "MessageSvc"

    svcMgr.MessageSvc.Format = "% F%50W%C%6W%R%e%s%8W%R%T %0W%M"

    numStores = jps.ConcurrencyFlags.NumConcurrentEvents()

    from StoreGate.StoreGateConf import SG__HiveMgrSvc
    svcMgr += SG__HiveMgrSvc("EventDataSvc")
    svcMgr.EventDataSvc.NSlots = numStores


    from GaudiHive.GaudiHiveConf import AlgResourcePool
    arp=AlgResourcePool( OutputLevel = Constants.INFO )
    arp.TopAlg=["AthMasterSeq"] #this should enable control flow
    svcMgr += arp

    from AthenaCommon.AlgScheduler import AlgScheduler
    AlgScheduler.ShowDataDependencies(False)
    AlgScheduler.ShowControlFlow(False)

    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()
    from SGComps.SGCompsConf import SGInputLoader
    # FailIfNoProxy=False makes it a warning, not an error, if unmet data
    # dependencies are not found in the store.  It should probably be changed
    # to True eventually.
    topSequence += SGInputLoader (FailIfNoProxy = False)
    AlgScheduler.setDataLoaderAlg ('SGInputLoader' )

    if (theApp._opts.mtes or jps.ConcurrencyFlags.NumProcs()>0):
        # Either multi-threaded Event Service or hybrid MP+MT
        from AthenaServices.AthenaServicesConf import AthenaMtesEventLoopMgr
        
        svcMgr += AthenaMtesEventLoopMgr()
        svcMgr.AthenaMtesEventLoopMgr.WhiteboardSvc = "EventDataSvc"
        svcMgr.AthenaMtesEventLoopMgr.SchedulerSvc = AlgScheduler.getScheduler().getName()

    if theApp._opts.mtes:
        svcMgr.AthenaMtesEventLoopMgr.EventRangeChannel = theApp._opts.mtes_channel
        theApp.EventLoop = "AthenaMtesEventLoopMgr"
    else:
        from AthenaServices.AthenaServicesConf import AthenaHiveEventLoopMgr

        svcMgr += AthenaHiveEventLoopMgr()
        svcMgr.AthenaHiveEventLoopMgr.WhiteboardSvc = "EventDataSvc"
        svcMgr.AthenaHiveEventLoopMgr.SchedulerSvc = AlgScheduler.getScheduler().getName()

        theApp.EventLoop = "AthenaHiveEventLoopMgr"

    #
    ## Setup SGCommitAuditor to sweep new DataObjects at end of Alg execute
    #
    
    theAuditorSvc = svcMgr.AuditorSvc
    theApp.AuditAlgorithms=True 
    from SGComps.SGCompsConf import SGCommitAuditor
    theAuditorSvc += SGCommitAuditor()
    
    
## load basic services configuration at module import
_setupAtlasThreadedJob()

## clean-up: avoid running multiple times this method
del _setupAtlasThreadedJob
