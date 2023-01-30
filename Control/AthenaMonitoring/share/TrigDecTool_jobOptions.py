tdt_local_logger = logging.getLogger('TrigDecTool_jobOptions')

# Set up the trigger decision tool (for trigger-aware monitoring)
if 'DQMonFlags' not in dir():
   tdt_local_logger.debug("DQMonFlags not yet imported - I import them now")
   from AthenaMonitoring.DQMonFlags import DQMonFlags

if DQMonFlags.useTrigger():
   if 'rec' not in dir():
      from RecExConfig.RecFlags import rec

   if rec.readESD() and (DQMonFlags.monManEnvironment=='tier0ESD'):
      # set up trigger config service
      if 'TriggerConfigGetter' not in dir():
         from TriggerJobOpts.TriggerConfigGetter import TriggerConfigGetter
         cfg = TriggerConfigGetter()

   from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable, CAtoGlobalWrapper
   from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
   tdtAcc = CAtoGlobalWrapper(TrigDecisionToolCfg, ConfigFlags)
   monTrigDecTool = conf2toConfigurable(tdtAcc.getPrimary())
   tdt_local_logger.info('Scheduled monitoring TDT %s', monTrigDecTool)

del tdt_local_logger
