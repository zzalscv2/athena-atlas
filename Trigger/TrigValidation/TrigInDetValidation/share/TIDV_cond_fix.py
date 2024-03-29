from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.IOVDb.GlobalTag='OFLCOND-RUN12-SDR-31'
ConfigFlags.Trigger.enableL1CaloPhase1=False
from IOVDbSvc.CondDB import conddb
conddb.addOverride('/PIXEL/PixelModuleFeMask','PixelModuleFeMask-SIM-MC16-000-03')
conddb.addOverride("/TRT/Calib/PID_NN", "TRTCalibPID_NN_v1")
conddb.addOverride("/PIXEL/PixelClustering/PixelNNCalibJSON", "PixelNNCalibJSON-SIM-RUN2-000-02")
