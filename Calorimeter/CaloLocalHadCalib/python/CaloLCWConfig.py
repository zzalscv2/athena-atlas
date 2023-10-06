# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def GetLCWCfg(flags):

  cfg=ComponentAccumulator()

  if flags.LCW.doClassification:
     from CaloLocalHadCalib.LCClassificationConfig import GetLCCCfg
     cfg.merge(GetLCCCfg(flags))

  if flags.LCW.doWeighting:
     from CaloLocalHadCalib.LCWeightingConfig import GetLCWCfg
     cfg.merge(GetLCWCfg(flags))

  if flags.LCW.doOutOfCluster:
     from CaloLocalHadCalib.LCOOCConfig import GetLCOOCCfg
     cfg.merge(GetLCOOCCfg(flags))

  if flags.LCW.doDeadMaterial:
     from CaloLocalHadCalib.LCDMConfig import GetLCDMCfg
     cfg.merge(GetLCDMCfg(flags))

  return cfg


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from CaloLocalHadCalib.LCWConfigFlags import addLCWFlags
    addLCWFlags(flags)

    #flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"]  
    #flags.Input.Files = ["/home/pavol/mc10_7TeV/ESD.371532._001990.pool.root.1"]  
    flags.Input.Files = ["/home/pavol/mc16_13TeV/ESD.29275544._000648.pool.root.1"]  
    flags.Output.ESDFileName="esdOut.pool.root"
    from AthenaConfiguration.Enums import LHCPeriod
    flags.GeoModel.Run = LHCPeriod.Run1
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN1_2010


    flags.fillFromArgs()

    # for debugging
    from AthenaCommon.Constants import DEBUG
    flags.Exec.OutputLevel=DEBUG

    # redo topo clusters on EM scale
    flags.Calo.TopoCluster.doTopoClusterLocalCalib = False
    flags.Calo.TopoCluster.doCellWeightCalib = False
    flags.Calo.TopoCluster.doCalibHitMoments = True

    flags.Calo.TopoCluster.doCalibHitMoments = True

    # choose which part to do
    flags.LCW.doClassification = True
    flags.LCW.doWeighting = True
    flags.LCW.doOutOfCluster = True
    flags.LCW.doDeadMaterial = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    theKey="CopyCaloTopoCluster"

    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg

    topoAcc=CaloTopoClusterCfg(flags,clustersnapname=theKey)
    topoAlg = topoAcc.getPrimary()
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").MatchDmType = 1 # 1=loose, 2=medium (default), 3=tight
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").CalibrationHitContainerNames = ["LArCalibrationHitInactive","LArCalibrationHitActive","TileCalibHitActiveCell","TileCalibHitInactiveCell"]
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").DMCalibrationHitContainerNames = ["LArCalibrationHitDeadMaterial_DEAD","LArCalibrationHitInactive_DEAD","LArCalibrationHitActive_DEAD","TileCalibHitDeadMaterial"]

    cfg.merge(topoAcc)

    cfg.merge(GetLCWCfg(flags))


    # for debugging purposes:
    cfg.getService("StoreGateSvc").Dump=True
    cfg.getService("MessageSvc").defaultLimit=999999

    cfg.run(10)
