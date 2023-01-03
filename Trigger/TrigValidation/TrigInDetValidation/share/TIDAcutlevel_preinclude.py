from AthenaCommon.Logging import logging
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAcutlevel_preinclude.py" )

from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags
InDetTrigFlags.cutLevel.set_Value_and_Lock(15)

log.info( "Defining InDetTrigFlags cutLevel: "+str(InDetTrigFlags.cutLevel) )
