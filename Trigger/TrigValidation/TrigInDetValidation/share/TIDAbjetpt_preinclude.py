from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAbjetpt_preinclude.py" ) 

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
from AthenaCommon.SystemOfUnits import GeV

getInDetTrigConfig("bjet")._pTmin = 0.8*GeV

from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
flags.Trigger.InDetTracking.bjet.Xi2max=12.

log.info( "ID Trigger pTmin: "+str(getInDetTrigConfig("bjet").pTmin) )
log.info(f"ID Trigger Xi2max: {flags.Trigger.InDetTracking.bjet.Xi2max}")

