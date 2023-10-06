# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GetLCCCfg(flags):

   result = ComponentAccumulator()

   GetLCC = CompFactory.GetLCClassification("GetLCC",ClusterCollectionName = "CaloTopoClusters")
   GetLCC.OutputFileName = flags.LCW.outFileNameLCC

   #
   # Example how to set dimensions for classiciation:
   #
   #from AthenaCommon.SystemOfUnits import deg, GeV, MeV
   #from math import ( pi as m_pi, log10 as m_log10 )
   #GetLCC.ClassificationDimensions = {
   #    'side': ('side',-1.5,1.5,1),
   #    '|eta|': ('|eta|',0.,5.,25),
   #    'phi': ('phi',-m_pi,m_pi,1),
   #    'log10(E_clus (MeV))': ('log10(E_clus (MeV))',m_log10(200*MeV),m_log10(1*TeV),13),
   #    'log10(<rho_cell (MeV/mm^3)>)-log10(E_clus (MeV))': ('log10(<rho_cell (MeV/mm^3)>)-log10(E_clus (MeV))',-9.0,-4.0,20),
   #    'log10(lambda_clus (mm))': ('log10(lambda_clus (mm))',0.0,4.0,20)}
   #

   result.addEventAlgo(GetLCC)

   return result

