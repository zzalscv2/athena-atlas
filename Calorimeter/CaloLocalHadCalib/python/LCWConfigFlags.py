# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def addLCWFlags(flags):

   flags.addFlag("LCW.doClassification",False)
   flags.addFlag("LCW.doWeighting",False)
   flags.addFlag("LCW.doOutOfCluster",False)
   flags.addFlag("LCW.doDeadMaterial",False)

   flags.addFlag("LCW.outFileNameLCC","classify.root")
   flags.addFlag("LCW.outFileNameLCW","weights.root")
   flags.addFlag("LCW.outFileNameLCO","ooc.root")
   flags.addFlag("LCW.outFileNameLCDM","dmc.root")

   flags.addFlag("LCW.inRootDM",["dmc.root"])
   flags.addFlag("LCW.inRootCL","classify.root")
   flags.addFlag("LCW.inRootW","weights.root")
   flags.addFlag("LCW.inRootOOC","ooc.root")
   flags.addFlag("LCW.inRootOOCPI0","oocpi0.root")
   flags.addFlag("LCW.outTagCL","CaloEMFrac2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60")
   flags.addFlag("LCW.outTagW","CaloH1CellWeights2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60")
   flags.addFlag("LCW.outTagOOC","CaloHadOOCCorr2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60")
   flags.addFlag("LCW.outTagOOCPI0","CaloHadOOCCorrPi02-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60")
   flags.addFlag("LCW.outDirCLWOOC","./")
   flags.addFlag("LCW.outFileNameCLWOOC","mc16_13TeV.428000-2.LCW_mu60_dt25_Run3.cl_w_ooc.v1.pool.root")
   flags.addFlag("LCW.outsfxCLWOOC","myclwooc")

   flags.addFlag("LCW.outDirDM","./")
   flags.addFlag("LCW.outsfxDM","mysfx")
   flags.addFlag("LCW.outTagDM","CaloHadDMCorr2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60")

   flags.addFlag("LCW.outFileNamePerf","LCSinglePionsPerformance.root")
