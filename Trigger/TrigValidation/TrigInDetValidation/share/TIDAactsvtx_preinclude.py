
from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAactsvtx_preinclude.py" ) 

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

getInDetTrigConfig("fullScan")._actsVertex = True

log.info( "ID Trigger actsVertex:  "+str(getInDetTrigConfig("fullScan").actsVertex) )



