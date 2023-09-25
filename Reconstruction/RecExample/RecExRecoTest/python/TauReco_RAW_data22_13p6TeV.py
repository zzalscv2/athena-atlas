# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    from RecExRecoTest.RecExReco_setupData22 import RecExReco_setupData22
    RecExReco_setupData22(ConfigFlags)

    # enable tau reconstruction
    ConfigFlags.Reco.EnableTrigger = False
    ConfigFlags.Reco.EnableCombinedMuon = True
    ConfigFlags.Reco.EnablePFlow = True
    ConfigFlags.Reco.EnableTau = True
    ConfigFlags.Reco.EnableJet = True
    ConfigFlags.Reco.EnableBTagging = False
    ConfigFlags.Reco.EnableCaloRinger = False
    ConfigFlags.Reco.PostProcessing.GeantTruthThinning = False
    ConfigFlags.Reco.PostProcessing.TRTAloneThinning = False

    ConfigFlags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(ConfigFlags)

    #with open("config.pkl", "wb") as file:
    #  acc.store(file)

    acc.run(100)

