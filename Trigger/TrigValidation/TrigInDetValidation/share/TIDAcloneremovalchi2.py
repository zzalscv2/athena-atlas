# leave this here for comparison ...
# ftf = findAlgorithm(topSequence, "TrigFastTrackFinder__electron")
# ftf.doCloneRemoval = True

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAcloneremoval.py" ) 

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
flags.Trigger.InDetTracking.electron.Xi2max=12.


log.info( "Setting clone removal: "+str(getInDetTrigConfig("electron").doCloneRemoval) )
log.info( f"Setting Xi2max: {flags.Trigger.InDetTracking.electron.Xi2max}" )


