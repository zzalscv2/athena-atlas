# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GetLCWCfg(flags):

   result = ComponentAccumulator()

   GetLCW = CompFactory.GetLCWeights("GetLCW",ClusterCollectionName = "CaloTopoClusters")
   GetLCW.OutputFileName = flags.LCW.outFileNameLCW
   GetLCW.UseInversionMethod = True
   GetLCW.CalibrationHitContainerNames = ["LArCalibrationHitInactive"
                                       ,"LArCalibrationHitActive"
                                       ,"TileCalibHitInactiveCell"
                                       ,"TileCalibHitActiveCell"]

   #
   # Example how to set dimensions for two samplings (here EMB1 & 2):
   #
   #from AthenaCommon.SystemOfUnits import deg, GeV, MeV, TeV
   #from math import ( pi as m_pi, log10 as m_log10 )
   # GetLCW.SamplingDimensions = {
   # 'EMB1:EMB1': ('EMB1',0.5,1.5,1),
   # 'EMB1:side': ('side',-1.5,1.5,1),
   # 'EMB1:|eta|': ('|eta|',0.,1.6,16),
   # 'EMB1:phi': ('phi',-m_pi,m_pi,1),
   # 'EMB1:log10(E_clus (MeV))': ('log10(E_clus (MeV))',m_log10(200*MeV),m_log10(1*TeV),22),
   # 'EMB1:log10(rho_cell (MeV/mm^3))': ('log10(rho_cell (MeV/mm^3))',-7.0,1.0,20),
   # 'EMB1:weight': ('weight',-2.0,3.0,1),
   # 'EMB2:EMB2': ('EMB2',1.5,2.5,1),
   # 'EMB2:side': ('side',-1.5,1.5,1),
   # 'EMB2:|eta|': ('|eta|',0.,1.6,16),
   # 'EMB2:phi': ('phi',-m_pi,m_pi,1),
   # 'EMB2:log10(E_clus (MeV))': ('log10(E_clus (MeV))',m_log10(200*MeV),m_log10(1*TeV),22),
   # 'EMB2:log10(rho_cell (MeV/mm^3))': ('log10(rho_cell (MeV/mm^3))',-7.0,1.0,20),
   # 'EMB2:weight': ('weight',-2.0,3.0,1)}
   #

   result.addEventAlgo(GetLCW)

   return result
