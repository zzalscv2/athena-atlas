from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAbrem.py" ) 

from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
flags.Trigger.InDetTracking.electron.doBremRecovery=True

log.info(f"ID Trigger doBremRecovery: {flags.Trigger.InDetTracking.electron.doBremRecovery}")

