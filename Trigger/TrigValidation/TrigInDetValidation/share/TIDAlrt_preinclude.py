
from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAlrt_preinclude.py" ) 

from IOVDbSvc.CondDB import conddb
conddb.addOverride('/PIXEL/PixelModuleFeMask','PixelModuleFeMask-SIM-MC16-000-03')
conddb.addOverride("/TRT/Calib/PID_NN", "TRTCalibPID_NN_v1")
conddb.addOverride("/PIXEL/PixelClustering/PixelNNCalibJSON", "PixelNNCalibJSON-SIM-RUN2-000-02")

from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.doR3LargeD0.set_Value_and_Lock(True)
#InDetFlags.storeSeparateLargeD0Container.set_Value_and_Lock(False) 

#ATR-25582 - FSLRT is now excluded from the default dev menu so need to change to the full dev menu rather than the filtered versions
from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.Trigger.triggerMenuSetup='Dev_pp_run3_v1'
ConfigFlags.Trigger.enableL1CaloPhase1=False
