# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    
    from RecExRecoTest.RecExReco_setupData22 import RecExReco_setupData22
    RecExReco_setupData22(ConfigFlags)
    ConfigFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(ConfigFlags)

    from MuonConfig.MuonReconstructionConfig import MuonReconstructionCfg
    acc.merge(MuonReconstructionCfg(ConfigFlags))

    acc.run(100)

