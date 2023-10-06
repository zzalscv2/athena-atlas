# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GetLCDMCfg(flags):

   result = ComponentAccumulator()

   GetLCDM = CompFactory.GetLCDeadMaterialTree("GetLCDM")
   GetLCDM.HadDMCoeffInitFile = "CaloHadDMCoeff_init_v2.txt"
   GetLCDM.ClusterCollectionName = "CaloTopoClusters"
   GetLCDM.ClusterCollectionNameCalib = "CaloCalTopoClusters"
   GetLCDM.doSaveCalibClusInfo = False # to save additional info from collection with calibrated clusters
   GetLCDM.OutputFileName = flags.LCW.outFileNameLCDM

   result.addEventAlgo(GetLCDM)

   return result
