# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def HImode(flags):
    flags.Reco.EnableHI = True
    flags.Reco.EnableTau = False
    flags.DQ.Steering.doTauMon = False
    flags.Reco.EnableJet = False
    flags.Reco.EnableMet = False
    flags.DQ.Steering.doMissingEtMon = False
    flags.Reco.EnableCaloRinger = False
    flags.Reco.EnableBTagging = True
    flags.Jet.WriteToAOD = True  # this is to save btagging to xAOD
    flags.Calo.TopoCluster.skipWriteList = [
        "CaloCalTopoClusters", "CaloTopoClusters"]
    flags.Egamma.doForward = False
    flags.Calo.FwdTower.WriteToAOD = False
    flags.HeavyIon.Egamma.doSubtractedClusters = True
    flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection = True
    flags.HeavyIon.Jet.HarmonicsForSubtraction = [2, 3, 4]
    flags.HeavyIon.Jet.SeedPtMin = 25000
    flags.HeavyIon.Jet.RecoOutputPtMin = 25000
    flags.HeavyIon.Jet.TrackJetPtMin = 7000


def _HIP_UPC_common(flags):
    flags.Reco.EnableHI = True
    flags.Reco.EnableTau = True
    flags.Reco.EnableJet = True
    flags.Reco.EnableMet = True
    flags.Reco.EnableCaloRinger = False
    flags.Reco.EnableBTagging = True
    flags.Jet.WriteToAOD = True  # this is to save "standard" jets and btagging to xAOD
    flags.MET.WritetoAOD = True
    flags.HeavyIon.Egamma.doSubtractedClusters = False
    flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection = False
    flags.HeavyIon.Jet.HarmonicsForSubtraction = []
    flags.HeavyIon.Jet.SeedPtMin = 8000
    flags.HeavyIon.Jet.RecoOutputPtMin = 8000
    flags.HeavyIon.Jet.TrackJetPtMin = 4000


def HIPmode(flags):
    _HIP_UPC_common(flags)
    # HIP dedicated flags
    flags.Tracking.doHIP = True


def UPCmode(flags):
    _HIP_UPC_common(flags)
    # UPC dedicated flags
    flags.Tracking.doUPC = True
    flags.Egamma.doLowMu = True
    flags.HeavyIon.Jet.WriteHIClusters = False
