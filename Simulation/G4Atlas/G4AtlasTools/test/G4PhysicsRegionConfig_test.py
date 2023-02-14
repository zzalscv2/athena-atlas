#!/usr/bin/env python

"""Run tests on G4PhysicsRegionConfig configuration

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from __future__ import print_function
"""

if __name__ == '__main__':
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg

  # Set up logging
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import DEBUG
  log.setLevel(DEBUG)


  #import config flags
  from AthenaConfiguration.AllConfigFlags import initConfigFlags

  from AthenaConfiguration.TestDefaults import defaultTestFiles
  inputDir = defaultTestFiles.d
  flags = initConfigFlags()
  flags.Input.Files = defaultTestFiles.EVNT
  # Arbitrary configuration for Twiss Files
  flags.Sim.TwissFileBeam1 = '4.0TeV/0090.00m/nominal/v01/beam1.tfs'
  flags.Sim.TwissFileBeam2 = '4.0TeV/0090.00m/nominal/v01/beam2.tfs'
  flags.Sim.TwissFileBeta = 90000 # in mm
  flags.Sim.TwissFileNomReal = 'nominal'
  flags.Sim.TwissFileVersion = 'v01'
  # Finalize
  flags.lock()

  ## Initialize a new component accumulator
  cfg = MainServicesCfg(flags)

  #import the following tool configs
  from G4AtlasTools.G4PhysicsRegionConfig import *
  #add the tools
  cfg.addPublicTool(BeampipeFwdCutPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(FWDBeamLinePhysicsRegionToolCfg(flags))
  cfg.addPublicTool(FwdRegionPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(PixelPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(SCTPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(TRTPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(TRT_ArPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(TRT_KrPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(EMBPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(EMECPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(HECPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(FCALPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(EMECParaPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(FCALParaPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(FCAL2ParaPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(PreSampLArPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(DeadMaterialPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(DriftWallPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(DriftWall1PhysicsRegionToolCfg(flags))
  cfg.addPublicTool(DriftWall2PhysicsRegionToolCfg(flags))
  cfg.addPublicTool(MuonSystemFastPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(MuonPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(SX1PhysicsRegionToolCfg(flags))
  cfg.addPublicTool(BedrockPhysicsRegionToolCfg(flags))
  cfg.addPublicTool(CavernShaftsConcretePhysicsRegionToolCfg(flags))
  cfg.addPublicTool(SCTSiliconPhysicsRegionToolCfg(flags))

  # Dump config
  # cfg.getService("StoreGateSvc").Dump = True
  # cfg.getService("ConditionStore").Dump = True
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print("-----------------finished----------------------")
