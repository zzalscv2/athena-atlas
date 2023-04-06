from AthenaCommon.Logging import logging
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDtemp_preinclude.py" )

from IOVDbSvc.CondDB import conddb
conddb.addOverride('/PIXEL/PixelModuleFeMask','PixelModuleFeMask-SIM-MC16-000-03')
conddb.addOverride("/TRT/Calib/PID_NN", "TRTCalibPID_NN_v1")
conddb.addOverride("/PIXEL/PixelClustering/PixelNNCalibJSON", "PixelNNCalibJSON-SIM-RUN2-000-02")

from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.Trigger.enableL1CaloPhase1=False

