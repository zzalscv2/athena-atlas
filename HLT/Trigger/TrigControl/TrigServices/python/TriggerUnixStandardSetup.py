# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

## @file TriggerUnixStandardSetup.py
## @brief py-module to configure the Athena AppMgr for trigger
## @author Werner Wiedenmann <Werner.Wiedenmann@cern.ch>
###############################################################

class _Conf:
    """Some configuration flags for this module with defaults"""
    useOnlineTHistSvc = True    # set in athenaHLT.py


def setupCommonServices(flags):
    from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
    CAtoGlobalWrapper(commonServicesCfg, flags)


def commonServicesCfg(flags):
    from AthenaCommon.Constants import INFO
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    # set ROOT to batch mode (ATR-21890)
    from PyUtils.Helpers import ROOT6Setup
    ROOT6Setup(batch=True)

    # Basic services
    cfg = ComponentAccumulator()
    cfg.addService(CompFactory.ClassIDSvc(CLIDDBFiles = ['clid.db','Gaudi_clid.db']))

    cfg.addService(CompFactory.AlgContextSvc(BypassIncidents=True))
    cfg.addAuditor(CompFactory.AlgContextAuditor())

    cfg.addService(CompFactory.StoreGateSvc())
    cfg.addService(CompFactory.StoreGateSvc("DetectorStore"))
    cfg.addService(CompFactory.StoreGateSvc("HistoryStore"))
    cfg.addService(CompFactory.StoreGateSvc("ConditionStore"))

    cfg.addService( CompFactory.SG.HiveMgrSvc(
        "EventDataSvc",
        NSlots = flags.Concurrency.NumConcurrentEvents) )

    cfg.addService( CompFactory.AlgResourcePool(
        OutputLevel = INFO,
        TopAlg=["AthSequencer/AthMasterSeq"]) )

    from AthenaConfiguration.MainServicesConfig import AvalancheSchedulerSvcCfg
    cfg.merge( AvalancheSchedulerSvcCfg(flags) )

    # SGCommitAuditor to sweep new DataObjects at end of Alg execute
    cfg.addAuditor( CompFactory.SGCommitAuditor() )

    # Error if unmet data dependencies are met
    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    cfg.merge( SGInputLoaderCfg(flags, FailIfNoProxy = True), sequenceName="AthAlgSeq")

    # CoreDumpSvc
    cfg.addService( CompFactory.CoreDumpSvc(
        CoreDumpStream = "stdout",
        CallOldHandler = False,  # avoid calling e.g. ROOT signal handler
        FastStackTrace = True,   # first produce a fast stacktrace
        StackTrace = True,       # then produce full stacktrace using gdb
        DumpCoreFile = True,     # also produce core file (if allowed by ulimit -c)
        FatalHandler = 0,        # no extra fatal handler
        TimeOut = 120e9) )       # timeout for stack trace generation changed to 120s (ATR-17112,ATR-25404)

    # IOVSvc
    cfg.addService( CompFactory.IOVSvc(
        updateInterval = "RUN",
        preLoadData = True,
        preLoadExtensibleFolders = False,  # ATR-19392
        forceResetAtBeginRun = False) )

    from TrigServices.TrigServicesConfig import TrigServicesCfg
    cfg.merge( TrigServicesCfg(flags) )

    # ApplicationMgr properties
    cfg.setAppProperty('AuditAlgorithms', True)
    cfg.setAppProperty('InitializationLoopCheck', False)

    return cfg


def setupCommonServicesEnd():
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr, athCondSeq
    from AthenaCommon.Logging import logging
    from AthenaCommon.AlgSequence import AlgSequence

    log = logging.getLogger( 'TriggerUnixStandardSetup::setupCommonServicesEnd:' )
    topSequence = AlgSequence()

    # Make sure no THistSvc output/input stream is defined for online running
    if _Conf.useOnlineTHistSvc:
        svcMgr.THistSvc.Output = []
        if len(svcMgr.THistSvc.Input)>0:
            log.error('THistSvc.Input = %s. Input not allowed for online running. Disabling input.', svcMgr.THistSvc.Input)
            svcMgr.THistSvc.Input = []

    # For offline running make sure at least the EXPERT stream is defined
    else:
        if 1 not in [ o.count('EXPERT') for o in svcMgr.THistSvc.Output ]:
            svcMgr.THistSvc.Output += ["EXPERT DATAFILE='expert-monitoring.root' OPT='RECREATE'"]

    # Basic operational monitoring
    from TrigOnlineMonitor.TrigOnlineMonitorConfig import TrigOpMonitor
    topSequence += TrigOpMonitor()

    # Set default properties for some important services after all user job options
    log.info('Configure core services for online running')

    from TrigServices.TrigServicesConfig import setupMessageSvc
    setupMessageSvc()

    from TrigServices.TrigServicesConfig import enableCOOLFolderUpdates
    enableCOOLFolderUpdates()

    if hasattr(svcMgr,'IOVDbSvc'):
        svcMgr.IOVDbSvc.CacheAlign = 0  # VERY IMPORTANT to get unique queries for folder udpates (see Savannah #81092)
        svcMgr.IOVDbSvc.CacheRun = 0
        svcMgr.IOVDbSvc.CacheTime = 0

    if hasattr(athCondSeq, 'AtlasFieldMapCondAlg'):
        athCondSeq.AtlasFieldMapCondAlg.LoadMapOnStart = True

    return
