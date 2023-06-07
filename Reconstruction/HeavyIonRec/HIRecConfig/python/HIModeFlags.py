# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def HImode(flags):
    flags.Reco.EnableHI=True
    flags.Reco.EnableTau=False
    flags.Reco.EnableJet=False
    flags.Reco.EnableMet=False
    flags.Reco.EnableCaloRinger=False
    flags.HeavyIon.Egamma.doSubtractedClusters=True
    flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection=True
    flags.HeavyIon.Jet.HarmonicsForSubtraction=[2,3,4]
    flags.HeavyIon.Jet.SeedPtMin=25000
    flags.HeavyIon.Jet.RecoOutputPtMin=25000
    flags.HeavyIon.Jet.TrackJetPtMin=7000

def HIPmode(flags):
    flags.Reco.EnableHI=True
    flags.Reco.EnableTau=False
    flags.Reco.EnableJet=True
    flags.Jet.WriteToAOD=True # this is to save "standard" jets to xAOD
    flags.Reco.EnableMet=False
    flags.Reco.EnableCaloRinger=False
    flags.HeavyIon.Egamma.doSubtractedClusters=False
    flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection=False
    flags.HeavyIon.Jet.HarmonicsForSubtraction=[]
    flags.HeavyIon.Jet.SeedPtMin=8000
    flags.HeavyIon.Jet.RecoOutputPtMin=8000
    flags.HeavyIon.Jet.TrackJetPtMin=4000
