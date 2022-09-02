from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAbjetpt_preinclude.py" ) 

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from AthenaCommon.SystemOfUnits import GeV

getInDetTrigConfig("bjet")._pTmin = 0.8*GeV
getInDetTrigConfig("bjet")._Xi2max = 12.0

log.info( "ID Trigger pTmin: "+str(getInDetTrigConfig("bjet").pTmin) )
log.info( "ID Trigger Xi2max: "+str(getInDetTrigConfig("bjet").Xi2max) )

