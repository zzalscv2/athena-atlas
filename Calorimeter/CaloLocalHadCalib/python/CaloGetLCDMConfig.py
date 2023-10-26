# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GetLCDMCfg(flags):

  cfg=ComponentAccumulator()

  GetLCDM = CompFactory.GetLCDeadMaterial("GetLCDM")

  GetLCDM.HadDMCoeffInputFile = "CaloHadDMCoeff_init_v2.txt"

  GetLCDM.HadDMCoeffOutputFile = flags.LCW.outDirDM+"/CaloLocalHadCoeff_output_"+flags.LCW.outsfxDM+".txt"
  GetLCDM.ReportProfiles = flags.LCW.outDirDM+"/report_CaloLocalHadCoeff_profiles_"+flags.LCW.outsfxDM+".ps"
  GetLCDM.ReportMinimization = flags.LCW.outDirDM+"/report_CaloLocalHadCoeff_minim_"+flags.LCW.outsfxDM+".ps"
  GetLCDM.ReportCheck = flags.LCW.outDirDM+"/report_CaloLocalHadCoeff_check_"+flags.LCW.outsfxDM+".ps"


  GetLCDM.DoFit = True
  GetLCDM.DoMinimization = True
  GetLCDM.DoPool = True
  GetLCDM.DoCheck = True
  # new feature to run on dmtrees from di-jets
  #GetLCDM.NormalizationTypeForFit="const"
  #GetLCDM.NormalizationTypeForMinim="const"
  #GetLCDM.ClassificationType = "particleid"

  GetLCDM.InputRootFiles  = flags.LCW.inRootDM

  GetLCDM.CorrectionKey="HadDMCoeff2"

  cfg.addEventAlgo(GetLCDM)

  # Output pool+sqlite
  from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
  cfg.merge(OutputConditionsAlgCfg(flags,
                                   outputFile=flags.LCW.outDirDM+"/"+flags.LCW.outsfxDM+".pool.root",
                                   ObjectList=["CaloLocalHadCoeff#HadDMCoeff2#/CALO/HadCalibration2/CaloDMCorr2",],
                                   IOVTagList=[flags.LCW.outTagDM,],
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
    flags.LCW.inRootDM = ["dmc.root"]

    # output sqlite
    flags.IOVDb.DatabaseInstance=""
    flags.IOVDb.DBConnection = "sqlite://;schema=myDm.db;dbname=OFLP200"

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    cfg.merge(IOVDbSvcCfg(flags))

    cfg.merge(GetLCDMCfg(flags))

    cfg.getService("AthenaPoolCnvSvc").PoolAttributes += [ "STREAM_MEMBER_WISE = '0'" ]

    # for debugging purposes:
    cfg.getService("StoreGateSvc").Dump=True
    cfg.getService("MessageSvc").defaultLimit=999999

    cfg.run(1)
