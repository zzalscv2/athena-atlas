# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GetLCOOCCfg(flags):

   result = ComponentAccumulator()

   GetLCO = CompFactory.GetLCOutOfCluster("GetLCO", ClusterCollectionName = "CaloTopoClusters")
   GetLCO.OutputFileName = flags.LCW.outFileNameLCO

   #
   # Example how to set dimensions for out-of-cluster corrections:
   #
   #from AthenaCommon.SystemOfUnits import deg, GeV, MeV
   #from AthenaCommon.AlgSequence import AlgSequence
   #from math import ( pi as m_pi, log10 as m_log10 )
   #GetLCO.OutOfClusterDimensions = {
   #    'side': ('side',-1.5,1.5,1),
   #    '|eta|': ('|eta|',0.,5.,50),
   #    'phi': ('phi',-m_pi,m_pi,1),
   #    'log10(E_clus (MeV))': ('log10(E_clus (MeV))',m_log10(200*MeV),m_log10(1*TeV),22),
   #    'log10(lambda_clus (mm))': ('log10(lambda_clus (mm))',0.0,4.0,20),
   #    'weight': ('weight',0.,5.,1)}
   #

   result.addEventAlgo(GetLCO)

   return result
