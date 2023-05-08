#!/usr/bin/env python
"""Run tests on LArG4SD configuration

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


if __name__ == '__main__':

  # Set up logging
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import DEBUG
  log.setLevel(DEBUG)


  #import config flags
  from AthenaConfiguration.AllConfigFlags import ConfigFlags
  ConfigFlags.Sim.ISFRun = True

  #Provide input
  from AthenaConfiguration.TestDefaults import defaultTestFiles
  inputDir = defaultTestFiles.d
  ConfigFlags.Input.Files = defaultTestFiles.EVNT

  # Finalize
  ConfigFlags.lock()


  ## Initialize a new component accumulator
  cfg = ComponentAccumulator()



  from LArG4SD.LArG4SDToolConfig import LArEMBSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArEMECSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArFCALSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArHECSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArDeadSensitiveDetectorToolCfg
  from LArG4SD.LArG4SDToolConfig import LArActiveSensitiveDetectorToolCfg
  from LArG4SD.LArG4SDToolConfig import LArInactiveSensitiveDetectorToolCfg

  acc1 = LArEMBSensitiveDetectorCfg(ConfigFlags)
  tool1 = cfg.popToolsAndMerge(acc1)
  #cfg.setPrivateTools(tool1)
  
  acc2 = LArEMECSensitiveDetectorCfg(ConfigFlags)
  tool2 = cfg.popToolsAndMerge(acc2)
  #cfg.setPrivateTools(tool2)


  acc3 = LArFCALSensitiveDetectorCfg(ConfigFlags)
  tool3 = cfg.popToolsAndMerge(acc3)
  #cfg.setPrivateTools(tool3)
  
  acc4 = LArHECSensitiveDetectorCfg(ConfigFlags)
  tool4 = cfg.popToolsAndMerge(acc4)
  #cfg.setPrivateTools(tool4)

  acc5 = LArDeadSensitiveDetectorToolCfg(ConfigFlags)
  tool5 = cfg.popToolsAndMerge(acc5)
  #cfg.setPrivateTools(toolDeadSensitiveDetector)

  toolActiveSensitiveDetector = LArActiveSensitiveDetectorToolCfg(ConfigFlags)
  #cfg.setPrivateTools(toolActiveSensitiveDetector)

  toolInactiveSensitiveDetector = LArInactiveSensitiveDetectorToolCfg(ConfigFlags)
  #cfg.setPrivateTools(toolInactiveSensitiveDetector)


  cfg.printConfig(withDetails=True, summariseProps = True)
  ConfigFlags.dump()

  f=open("test.pkl","wb")
  cfg.store(f)
  f.close()

  print("-----------------finished----------------------")
