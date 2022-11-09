#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

from sys import argv  

if __name__=="__main__":
  
  FPEFlag=0
  if len(argv)>1:
    FPEFlag=int(argv[1])

  from AthenaConfiguration.AllConfigFlags import ConfigFlags
  ConfigFlags.Input.RunNumber = 284500 # dummay value
  ConfigFlags.Input.TimeStamp = 1 # dummy value
  ConfigFlags.Exec.FPE=FPEFlag
  cfg = MainEvgenServicesCfg(ConfigFlags)
  
  cfg.addEventAlgo(CompFactory.AthExAlgWithFPE(),sequenceName="AthAlgSeq")
  
  cfg.run(3)
