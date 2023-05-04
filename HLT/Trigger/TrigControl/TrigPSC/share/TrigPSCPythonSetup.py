# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###############################################################
## @file   TrigPSCPythonSetup.py
## @brief  Python setup for online running (similar to athena.py)
## @author Werner Wiedenmann <Werner.Wiedenmann@cern.ch>
###############################################################
import builtins
printfunc = getattr(builtins,'print')

### setup from a .pkl file
from TrigPSC import PscConfig
joPath = PscConfig.optmap['JOBOPTIONSPATH']
if joPath[-4:] == '.pkl':
   print("Configuring the job from a pickle file")
   import AthenaCommon.ConfigurationShelve
   AthenaCommon.ConfigurationShelve.loadJobOptionsCatalogue( joPath )
   del AthenaCommon.ConfigurationShelve
   print("Job configuration loaded successfully from a pickle file")

else:
   import sys
   import os

   ### Set up some common flags --------------------------------------------------
   from TrigPSC.PscDefaultFlags import defaultOnlineFlags
   flags = defaultOnlineFlags()

   ### Athena configuration -----------------------------------------------------
   from GaudiPython import *                   # noqa: F401, F403
   from AthenaCommon.Configurable import *     # noqa: F401, F403
   from AthenaCommon.OldStyleConfig import *   # noqa: F401, F403
   from AthenaCommon.Logging import *          # noqa: F401, F403
   from AthenaCommon.Constants import *        # noqa: F401, F403
   import AthenaCommon.ExitCodes as ExitCodes

   ## create the application manager and start in a non-initialised state
   from AthenaCommon.AppMgr import theApp
   from AthenaCommon.AppMgr import ToolSvc, ServiceMgr, theAuditorSvc   # noqa: F401

   ## These properties have already been set on the C++ ApplicationMgr in the Psc
   ## but the configurable needs to have them set as well
   theApp.EventLoop         = "HltEventLoopMgr"
   theApp.MessageSvcType    = PscConfig.optmap["MESSAGESVCTYPE"]
   theApp.JobOptionsSvcType = PscConfig.optmap["JOBOPTIONSSVCTYPE"]

   ## add the MessageSvc and the JobOptionsSvc to the ServiceMgr
   from AthenaConfiguration.ComponentFactory import CompFactory
   from TrigServices.TrigServicesConfig import getMessageSvc
   ServiceMgr += CompFactory.getComp(theApp.JobOptionsSvcType)("JobOptionsSvc")

   ## set OutputLevel
   logLevel = PscConfig.optmap['LOGLEVEL'].upper().split(',')

   if len(logLevel) > 0:
      from AthenaCommon import Constants
      flags.Exec.OutputLevel = getattr(Constants, logLevel[0])

   if 'POOL_OUTMSG_LEVEL' not in os.environ and len(logLevel)>1:
      os.environ['POOL_OUTMSG_LEVEL'] = logLevel[1]

   del logLevel

   ServiceMgr += getMessageSvc(flags, theApp.MessageSvcType)
   theApp.setOutputLevel(flags.Exec.OutputLevel)

   from AthenaCommon.Logging import logging
   psclog = logging.getLogger('TrigPSCPythonSetup')

   ## file inclusion and tracing
   from AthenaCommon.Include import IncludeError, include

   ## set resource limits
   from AthenaCommon.ResourceLimits import SetMaxLimits
   SetMaxLimits()
   del SetMaxLimits

   ### run optional command before user job options script ----------------------
   if PscConfig.optmap['PRECOMMAND']:
      print("\n")
      print(" +------------------------------------------------+ ")
      print(" | Execute command before jobOptions script START.| ")
      print(" +------------------------------------------------+ ")
      print(" ---> Command = %s" % PscConfig.optmap['PRECOMMAND'])
      try:
         exec(PscConfig.optmap['PRECOMMAND'])
      except Exception as e:
         if isinstance( e, IncludeError ):
            print(sys.exc_info()[0], e)
            theApp._exitstate = ExitCodes.INCLUDE_ERROR
            sys.exit( theApp._exitstate )         
         elif isinstance( e, ImportError ):
            print(sys.exc_info()[0], e)
            theApp._exitstate = ExitCodes.IMPORT_ERROR
            sys.exit( theApp._exitstate )
         raise
      print(" +------------------------------------------------+ ")
      print(" | Execute command before jobOptions script END.  | ")
      print(" +------------------------------------------------+ ")
      print("\n")

   ## Now clone and use locked flags for services configuration
   flags = flags.clone()
   flags.lock()

   ### basic job configuration before user configuration ------------------------
   from TrigServices.TriggerUnixStandardSetup import setupCommonServices
   setupCommonServices(flags)

   ### run user jobOptions file -------------------------------------------------
   try:
      include( "%s" % PscConfig.optmap['JOBOPTIONSPATH'] )
   except Exception as e:
      if isinstance(e,SystemExit):
         raise

      # "sanitize" the traceback under the assumption that it is the user's fault
      import traceback
      exc_info = sys.exc_info()
      tb = traceback.extract_tb( exc_info[2] )

      short_tb = []
      for frame_info in tb:
         if 'AthenaCommon' not in frame_info[0]:
            short_tb.append( frame_info )

      print('Shortened traceback (most recent user call last):')
      print(''.join( traceback.format_list( short_tb )), end=' ')
      print(''.join( traceback.format_exception_only( exc_info[0], exc_info[1] )), end= '')
      sys.stdout.flush()

      # additional processing to get right error codes
      import AthenaCommon.ExitCodes as ExitCodes
      if isinstance( e, IncludeError ):
         theApp._exitstate = ExitCodes.INCLUDE_ERROR
         sys.exit( theApp._exitstate )
      elif isinstance( e, ImportError ):
         theApp._exitstate = ExitCodes.IMPORT_ERROR
         sys.exit( theApp._exitstate )

      # Throw exception to stop application
      theApp._exitstate = ExitCodes.UNKNOWN_EXCEPTION
      raise

   ### run optional command after user job options script -----------------------
   if PscConfig.optmap['POSTCOMMAND']:
      print("\n")
      print(" +------------------------------------------------+ ")
      print(" | Execute command after jobOptions script START. | ")
      print(" +------------------------------------------------+ ")
      print(" ---> Command = ", PscConfig.optmap['POSTCOMMAND'])
      try:
         exec(PscConfig.optmap['POSTCOMMAND'])
      except Exception as e:
         if isinstance( e, IncludeError ):
            print(sys.exc_info()[0], e)
            theApp._exitstate = ExitCodes.INCLUDE_ERROR
            sys.exit( ExitCodes.INCLUDE_ERROR )
         elif isinstance( e, ImportError ):
            print(sys.exc_info()[0], e)
            theApp._exitstate = ExitCodes.IMPORT_ERROR
            sys.exit( ExitCodes.IMPORT_ERROR )
         raise
      print(" +------------------------------------------------+ ")
      print(" | Execute command after jobOptions script END.   | ")
      print(" +------------------------------------------------+ ")
      print("\n")

   ### Dump properties and convert to JSON if requested -------------------------
   if PscConfig.dumpJobProperties:
      from AthenaCommon import ConfigurationShelve
      from TrigConfIO.JsonUtils import create_joboptions_json
      fname = 'HLTJobOptions'
      ConfigurationShelve.storeJobOptionsCatalogue(fname+".pkl")
      create_joboptions_json(fname+".pkl",fname+".json")

      if PscConfig.exitAfterDump:
         theApp.exit(0)
      elif PscConfig.reloadAfterDump:
         from TrigCommon.AthHLT import reload_from_json
         reload_from_json(fname+".json")

   else:
      # storeJobOptionsCatalogue calls setup() itself, so we only need it here
      theApp.setup()

   ### Cleanup
   del psclog
