# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GetLC_CL_W_OOC_Cfg(flags):

  cfg=ComponentAccumulator()

  cfg.addEventAlgo( CompFactory.CaloReadLCClassificationFile("ReadLCClassification",
                                     LCClassificationFileName  = flags.LCW.inRootCL, ClassificationKey="EMFracClassify"))

  cfg.addEventAlgo( CompFactory.CaloReadLCWeightsFile("ReadLCWeights",
                                     LCWeightFileName  = flags.LCW.inRootW, CorrectionKey="H1ClusterCellWeights"))

  cfg.addEventAlgo( CompFactory.CaloReadLCOutOfClusterFile("ReadLCOutOfCluster",
                                     LCOutOfClusterFileName  = flags.LCW.inRootOOC, CorrectionKey="OOCCorrection"))

  cfg.addEventAlgo( CompFactory.CaloReadLCOutOfClusterFile("ReadLCOutOfClusterPi0",
                                     LCOutOfClusterFileName  = flags.LCW.inRootOOCPI0, CorrectionKey="OOCPi0Correction"))


  # Output pool+sqlite
  from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
  cfg.merge(OutputConditionsAlgCfg(flags,
                           outputFile=flags.LCW.outDirCLWOOC+"/"+flags.LCW.outFileNameCLWOOC,
                           ObjectList=["CaloLocalHadCoeff#EMFracClassify#/CALO/HadCalibration2/CaloEMFrac",
                                       "CaloLocalHadCoeff#H1ClusterCellWeights#/CALO/HadCalibration2/H1ClusterCellWeights",
                                       "CaloLocalHadCoeff#OOCCorrection#/CALO/HadCalibration2/CaloOutOfCluster",
                                       "CaloLocalHadCoeff#OOCPi0Correction#/CALO/HadCalibration2/CaloOutOfClusterPi0"
                                      ],
                           IOVTagList=[flags.LCW.outTagCL, flags.LCW.outTagW, flags.LCW.outTagOOC, flags.LCW.outTagOOCPI0],
                                   WriteIOV = True,
                                   Run1 = 0, 
                                   Run2 = 0x7FFFFFFF
                                    ))

  return cfg


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from CaloLocalHadCalib.LCWConfigFlags import addLCWFlags
    addLCWFlags(flags)


    flags.fillFromArgs()

    # inputless job
    flags.Input.Files=[]

    # for debugging
    from AthenaCommon.Constants import DEBUG
    flags.Exec.OutputLevel=DEBUG

    # choose input file
    flags.LCW.inRootCL = "/afs/cern.ch/user/m/menke/public/LCWeights_2016/user.menke.mc16.piplusminuszero.LCW.Run2_mu30_dt25.v3.classify.root"
    flags.LCW.inRootW = "/afs/cern.ch/user/m/menke/public/LCWeights_2016/user.menke.mc16.piplusminus.LCW.Run2_mu30_dt25.v3.inv_weights.root"
    flags.LCW.inRootOOC = "/afs/cern.ch/user/m/menke/public/LCWeights_2016/user.menke.mc16.piplusminus.LCW.Run2_mu30_dt25.v3_EXT2.root"
    flags.LCW.inRootOOCPI0 = "/afs/cern.ch/user/m/menke/public/LCWeights_2016/user.menke.mc16.pizero.LCW.Run2_mu30_dt25.v3_EXT2.root"

    # output sqlite
    flags.IOVDb.DatabaseInstance=""
    flags.IOVDb.DBConnection = "sqlite://;schema=myclwooc.db;dbname=OFLP200"

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    cfg.merge(IOVDbSvcCfg(flags))

    cfg.merge(GetLC_CL_W_OOC_Cfg(flags))

    cfg.getService("AthenaPoolCnvSvc").PoolAttributes += [ "STREAM_MEMBER_WISE = '0'" ]

    # for debugging purposes:
    cfg.getService("StoreGateSvc").Dump=True
    cfg.getService("MessageSvc").defaultLimit=999999

    cfg.run(1)
