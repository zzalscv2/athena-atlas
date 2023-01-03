from AthenaCommon.Logging import logging
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAjetdEdx_preinclude.py" )

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
getInDetTrigConfig("jet")._dodEdxTrk = True

log.info( "Defining ConfigSettings, jet dodEdxTrk: "+str(getInDetTrigConfig("jet").dodEdxTrk) )
