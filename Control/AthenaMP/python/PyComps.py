# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#-----Python imports---#
import os, sys, shutil

#-----Athena imports---#
from AthenaCommon.Logging import log as msg

from AthenaMP.AthenaMPConf import AthMpEvtLoopMgr
class MpEvtLoopMgr(AthMpEvtLoopMgr):
    def __init__(self, name='AthMpEvtLoopMgr', **kw):

        from AthenaCommon.AppMgr import theApp
        self.nThreads = theApp._opts.threads

        ## init base class
        kw['name'] = name
        super(MpEvtLoopMgr, self).__init__(**kw)

        os.putenv('XRD_ENABLEFORKHANDLERS','1')
        os.putenv('XRD_RUNFORKHANDLER','1')

        from .AthenaMPFlags import jobproperties as jp
        self.WorkerTopDir = jp.AthenaMPFlags.WorkerTopDir()
        self.OutputReportFile = jp.AthenaMPFlags.OutputReportFile()
        self.CollectSubprocessLogs = jp.AthenaMPFlags.CollectSubprocessLogs()
        self.Strategy = jp.AthenaMPFlags.Strategy()
        self.PollingInterval = jp.AthenaMPFlags.PollingInterval()
        self.MemSamplingInterval = jp.AthenaMPFlags.MemSamplingInterval()

        from AthenaCommon.DetFlags import DetFlags
        if DetFlags.pileup.any_on() or DetFlags.overlay.any_on():
            self.IsPileup = True
        else:
            self.IsPileup = False
        self.EventsBeforeFork = jp.AthenaMPFlags.EventsBeforeFork()

        if self.Strategy=='EventService':
            self.EventsBeforeFork = 0

        from AthenaCommon.AppMgr import theApp as app
        app.EventLoop = self.getFullJobOptName()

        # Enable FileMgr logging
        from GaudiSvc.GaudiSvcConf import FileMgr
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        svcMgr+=FileMgr(LogFile="FileManagerLog")

        # Save PoolFileCatalog.xml if exists in the run directory
        if os.path.isfile('PoolFileCatalog.xml'):
            shutil.copyfile('PoolFileCatalog.xml','PoolFileCatalog.xml.AthenaMP-saved')

        self.configureStrategy(self.Strategy,self.IsPileup,self.EventsBeforeFork)
        
    def configureStrategy(self,strategy,pileup,events_before_fork) -> None :
        from .AthenaMPFlags import jobproperties as jp
        import AthenaCommon.ConcurrencyFlags # noqa: F401
        event_range_channel = jp.AthenaMPFlags.EventRangeChannel()

        chunk_size = getChunkSize()
        if chunk_size < 1:
            msg.warning('Nonpositive ChunkSize (%i) caught, setting it to 1', chunk_size)
            chunk_size = 1
        
        debug_worker = jp.ConcurrencyFlags.DebugWorkers()
        use_shared_reader = jp.AthenaMPFlags.UseSharedReader()
        use_shared_writer = jp.AthenaMPFlags.UseSharedWriter()
        use_parallel_compression = jp.AthenaMPFlags.UseParallelCompression()

        if strategy=='SharedQueue' or strategy=='RoundRobin':
            if use_shared_reader:
                from AthenaCommon.AppMgr import ServiceMgr as svcMgr
                svcMgr.PoolSvc.MaxFilesOpen = 2
                from AthenaIPCTools.AthenaIPCToolsConf import AthenaSharedMemoryTool
                svcMgr.EventSelector.SharedMemoryTool = AthenaSharedMemoryTool("EventStreamingTool", SharedMemoryName="EventStream"+str(os.getpid()))
                if 'AthenaPoolCnvSvc.ReadAthenaPool' in sys.modules:
                    svcMgr.AthenaPoolCnvSvc.InputStreamingTool = AthenaSharedMemoryTool("InputStreamingTool", SharedMemoryName="InputStream"+str(os.getpid()))
            if use_shared_writer:
                from AthenaCommon.AppMgr import ServiceMgr as svcMgr
                if 'AthenaPoolCnvSvc.WriteAthenaPool' in sys.modules:
                    from AthenaIPCTools.AthenaIPCToolsConf import AthenaSharedMemoryTool
                    svcMgr.AthenaPoolCnvSvc.OutputStreamingTool += [ AthenaSharedMemoryTool("OutputStreamingTool_0", SharedMemoryName="OutputStream"+str(os.getpid())) ]
                svcMgr.AthenaPoolCnvSvc.ParallelCompression=use_parallel_compression

            from AthenaMPTools.AthenaMPToolsConf import SharedEvtQueueProvider
            self.Tools += [ SharedEvtQueueProvider(UseSharedReader=use_shared_reader,
                                                   IsPileup=pileup,
                                                   EventsBeforeFork=events_before_fork,
                                                   ChunkSize=chunk_size) ]

            if (self.nThreads >= 1):
                if(pileup):
                    raise Exception('Running pileup digitization in mixed MP+MT currently not supported')
                from AthenaMPTools.AthenaMPToolsConf import SharedHiveEvtQueueConsumer
                self.Tools += [ SharedHiveEvtQueueConsumer(UseSharedWriter=use_shared_writer, 
                                                           EventsBeforeFork=events_before_fork,
                                                           Debug=debug_worker)   ]
            else:
                from AthenaMPTools.AthenaMPToolsConf import SharedEvtQueueConsumer
                self.Tools += [ SharedEvtQueueConsumer(UseSharedReader=use_shared_reader,
                                                       UseSharedWriter=use_shared_writer,
                                                       IsPileup=pileup,
                                                       IsRoundRobin=(strategy=='RoundRobin'),
                                                       EventsBeforeFork=events_before_fork,
                                                       ReadEventOrders=jp.AthenaMPFlags.ReadEventOrders(),
                                                       EventOrdersFile=jp.AthenaMPFlags.EventOrdersFile(),
                                                       Debug=debug_worker)   ]
            if use_shared_writer:
                from AthenaMPTools.AthenaMPToolsConf import SharedWriterTool
                self.Tools += [ SharedWriterTool(MotherProcess=(events_before_fork>0),
                                                 IsPileup=pileup,
                                                 Debug=debug_worker) ]

            # Enable seeking
            if not use_shared_reader:
                setupEvtSelForSeekOps()

        elif strategy=='EventService':
            channelScatterer2Processor = "AthenaMP_Scatterer2Processor"
            channelProcessor2EvtSel = "AthenaMP_Processor2EvtSel"

            from AthenaMPTools.AthenaMPToolsConf import EvtRangeScatterer
            self.Tools += [ EvtRangeScatterer(ProcessorChannel = channelScatterer2Processor,
                                              EventRangeChannel = event_range_channel,
                                              DoCaching=jp.AthenaMPFlags.EvtRangeScattererCaching()) ]

            from AthenaMPTools.AthenaMPToolsConf import EvtRangeProcessor
            self.Tools += [ EvtRangeProcessor(IsPileup=pileup,
                                              Channel2Scatterer = channelScatterer2Processor,
                                              Channel2EvtSel = channelProcessor2EvtSel,
                                              Debug=debug_worker) ]
            # Enable seeking
            setupEvtSelForSeekOps()

        else:
            msg.warning("Unknown strategy. No MP tools will be configured")

def setupEvtSelForSeekOps() -> None:
   """ try to install seek-stuff on the EventSelector side """
   #import sys
   #from AthenaCommon.Logging import log as msg
   msg.debug("setupEvtSelForSeekOps:")
   if 'AthenaRootComps.ReadAthenaRoot' in  sys.modules:
       # athenarootcomps has seeking enabled by default
       msg.info('=> Seeking enabled.')
       return
   
   if 'AthenaPoolCnvSvc.ReadAthenaPool' not in sys.modules:
      ## user did not import that module so we give up
      msg.info( "Cannot enable 'seeking' b/c module "
                "[AthenaPoolCnvSvc.ReadAthenaPool] hasn't been imported..." )
      msg.info( "Modify your jobOptions to import that module "
                "(or just ignore this message)" )
      return

   from AthenaCommon.AppMgr import theApp, AthAppMgr
   if theApp.state() != AthAppMgr.State.OFFLINE:
      msg.info( "C++ ApplicationMgr already instantiated, probably seeking "
                "will be ill-configured..." )
      msg.info( "EventSelector writers should implement updateHandlers" )
   
   from AthenaCommon.AppMgr import ServiceMgr as svcMgr
   from AthenaCommon.Configurable import Configurable
   collectionType = svcMgr.EventSelector.properties()["CollectionType"]

   if collectionType in ( "ImplicitROOT", Configurable.propertyNoValue, ):
      msg.info   ( "=> Seeking enabled." )

   else:
      msg.warning( "Input seeking is not compatible with collection type of %s",
                   svcMgr.EventSelector.properties()["CollectionType"] )
      msg.warning( "=> Seeking disabled." )
   return

def getChunkSize() -> int :
    from .AthenaMPFlags import jobproperties as jp
    from PyUtils.MetaReaderPeeker import metadata
    chunk_size = 1
    if (jp.AthenaMPFlags.ChunkSize() > 0):
        chunk_size = jp.AthenaMPFlags.ChunkSize()
        msg.info('Chunk size set to %i', chunk_size)
    elif metadata['file_size'] is not None:
        #Don't use auto flush for shared reader
        if (jp.AthenaMPFlags.UseSharedReader()):
            msg.info('Shared Reader in use, chunk_size set to default (%i)', chunk_size)
        #Use auto flush only if file is compressed with LZMA, else use default chunk_size
        elif (jp.AthenaMPFlags.ChunkSize() == -1):
            if (metadata['file_comp_alg'] == 2):
                chunk_size = metadata['auto_flush']
                msg.info('Chunk size set to auto flush (%i)', chunk_size)
            else:
                msg.info('LZMA algorithm not in use, chunk_size set to default (%i)', chunk_size)
        #Use auto flush only if file is compressed with LZMA or ZLIB, else use default chunk_size
        elif (jp.AthenaMPFlags.ChunkSize() == -2):
            if (metadata['file_comp_alg'] == 1 or metadata['file_comp_alg'] == 2):
                chunk_size = metadata['auto_flush']
                msg.info('Chunk size set to auto flush (%i)', chunk_size)
            else:
                msg.info('LZMA nor ZLIB in use, chunk_size set to default (%i)', chunk_size)
        #Use auto flush only if file is compressed with LZMA, ZLIB or LZ4, else use default chunk_size
        elif (jp.AthenaMPFlags.ChunkSize() == -3):
            if (metadata['file_comp_alg'] == 1 or metadata['file_comp_alg'] == 2 or metadata['file_comp_alg'] == 4):
                chunk_size = metadata['auto_flush']
                msg.info('Chunk size set to auto flush (%i)', chunk_size)
            else:
                msg.info('LZMA, ZLIB nor LZ4 in use, chunk_size set to (%i)', chunk_size)
        #Use auto flush value for chunk_size, regarldess of compression algorithm
        elif (jp.AthenaMPFlags.ChunkSize() <= -4):
            chunk_size = metadata['auto_flush']
            msg.info('Chunk size set to auto flush (%i)', chunk_size)
        else:
            msg.warning('Invalid ChunkSize, Chunk Size set to default (%i)', chunk_size)

    return chunk_size
