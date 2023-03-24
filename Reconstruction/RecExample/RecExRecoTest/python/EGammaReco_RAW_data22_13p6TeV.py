# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    
    from RecExRecoTest.RecExReco_setupData22 import RecExReco_setupData22
    RecExReco_setupData22(ConfigFlags)
    from egammaConfig.egammaOnlyFromRawFlags import egammaOnlyFromRaw
    egammaOnlyFromRaw(ConfigFlags)
    ConfigFlags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(ConfigFlags)

    with open("config.pkl", "wb") as file:
      acc.store(file)

    acc.run(100)
