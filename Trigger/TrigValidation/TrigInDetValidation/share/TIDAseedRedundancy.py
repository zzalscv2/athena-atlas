#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAseedRedundancy.py" ) 

from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
flags.Trigger.InDetTracking.fullScan.doSeedRedundancyCheck = True
flags.Trigger.InDetTracking.jet.doSeedRedundancyCheck      = True
flags.Trigger.InDetTracking.jetSuper.doSeedRedundancyCheck = True

log.info( "Setting fullScan doSeedRedundancyCheck: "+str(flags.Trigger.InDetTracking.fullScan.doSeedRedundancyCheck) )
log.info( "Setting jet      doSeedRedundancyCheck: "+str(flags.Trigger.InDetTracking.jet.doSeedRedundancyCheck) )
log.info( "Setting jetSuper doSeedRedundancyCheck: "+str(flags.Trigger.InDetTracking.jetSuper.doSeedRedundancyCheck) )


