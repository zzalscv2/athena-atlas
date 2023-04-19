###############################################################################
# Offline AthenaMonitoring configuration included in RecExCommon before       #
# reconstruction is set up                                                    #
#                                                                             #
# $Id: DataQualityInit_jobOptions.py,v 1.3 2009-03-20 16:38:54 sschaetz Exp $ #
###############################################################################

# DQMonFlags
try:
   include("AthenaMonitoring/DQMonFlagsConfig_jobOptions.py")
except Exception:
   treatException("Could not load AthenaMonitoring/DQMonFlagsConfig_jobOptions.py")

if DQMonFlags.doMonitoring() and not DQMonFlags.doNewMonitoring():
   if DQMonFlags.useTrigger():
      # trigger decision tool
      from AthenaConfiguration.AllConfigFlags import ConfigFlags
      from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
      from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg

      # If the flags are not locked yet, we have to do it now
      ConfigFlags.lock()
      CAtoGlobalWrapper(TrigDecisionToolCfg, ConfigFlags)

   # set up first monitoring manager to set static variables
   from AthenaMonitoring.AthenaMonitoringConf import AthenaMonManager
   monManGlobal = AthenaMonManager(name="AthenaMonManager",
                       FileKey             = DQMonFlags.monManFileKey(),
                       ManualDataTypeSetup = DQMonFlags.monManManualDataTypeSetup(),
                       DataType            = DQMonFlags.monManDataType(),
                       Environment         = DQMonFlags.monManEnvironment(),
                      LBsInLowStatInterval = DQMonFlags.monManLBsInLowStatInterval(),
                   LBsInMediumStatInterval = DQMonFlags.monManLBsInMediumStatInterval(),
                     LBsInHighStatInterval = DQMonFlags.monManLBsInHighStatInterval(),
                       ManualRunLBSetup    = DQMonFlags.monManManualRunLBSetup(),
                       Run                 = DQMonFlags.monManRun(),
                       LumiBlock           = DQMonFlags.monManLumiBlock())
   topSequence += monManGlobal

   # set up histogram output file
   if (DQMonFlags.histogramFile() != "") and (DQMonFlags.monManFileKey() != ""):
      from AthenaCommon.AppMgr import ServiceMgr as svcMgr
      if not hasattr(svcMgr, 'THistSvc'):
          from GaudiSvc.GaudiSvcConf import THistSvc
          svcMgr += THistSvc()
      svcMgr.THistSvc.Output += [ DQMonFlags.monManFileKey() + " DATAFILE='" + DQMonFlags.histogramFile() + "' OPT='RECREATE'"]

# inner detector monitoring
if not 'InDetFlags' in dir():
   from InDetRecExample.InDetJobProperties import InDetFlags

InDetFlags.doMonitoringPixel=DQMonFlags.doPixelMon()
InDetFlags.doMonitoringSCT=DQMonFlags.doSCTMon()
InDetFlags.doMonitoringTRT=DQMonFlags.doTRTMon()
InDetFlags.doMonitoringGlobal=DQMonFlags.doInDetGlobalMon()
#InDetFlags.doMonitoringPrimaryVertexingEnhanced=DQMonFlags.doInDetGlobalMon()
InDetFlags.doMonitoringAlignment=DQMonFlags.doInDetAlignMon()

