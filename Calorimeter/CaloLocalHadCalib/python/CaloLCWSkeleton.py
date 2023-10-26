# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from CaloLocalHadCalib.CaloLCWConfig import GetLCWCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg


def fromRunArgs(runArgs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    from CaloLocalHadCalib.LCWConfigFlags import addLCWFlags
    addLCWFlags(flags)

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    # general flags 
    flags.Input.Files=runArgs.inputESDFile
    
    # redo topo clusters on EM scale
    flags.Calo.TopoCluster.doTopoClusterLocalCalib = runArgs.doLocalCalib
    flags.Calo.TopoCluster.doCellWeightCalib = runArgs.doCellWeight
    flags.Calo.TopoCluster.doCalibHitMoments = runArgs.doHitMoments

    # choose which part to do
    flags.LCW.doClassification = runArgs.doClassification
    flags.LCW.doWeighting = runArgs.doWeighting
    flags.LCW.doOutOfCluster = runArgs.doOOC
    flags.LCW.doDeadMaterial = runArgs.doDeadM

    flags.lock()

    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg

    topoAcc=CaloTopoClusterCfg(flags,clustersnapname=runArgs.ClusKey)
    topoAlg = topoAcc.getPrimary()
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").MatchDmType = 1 # 1=loose, 2=medium (default), 3=tight
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").CalibrationHitContainerNames = ["LArCalibrationHitInactive","LArCalibrationHitActive","TileCalibHitActiveCell","TileCalibHitInactiveCell"]
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").DMCalibrationHitContainerNames = ["LArCalibrationHitDeadMaterial_DEAD","LArCalibrationHitInactive_DEAD","LArCalibrationHitActive_DEAD","TileCalibHitDeadMaterial"]

    cfg.merge(topoAcc)

    cfg.merge(GetLCWCfg(flags))
    
    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())

