# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def _HIcommon(flags):
    flags.Reco.EnableHI = True
    flags.Reco.EnableCaloRinger = False #AutoConfig not prevFlags.Reco.EnableHI)
    flags.Reco.EnableBTagging = True  #anyway auto-configured based on EnableJets
    # disable TopoCluster out of time pileup cut
    flags.Calo.TopoCluster.doTimeCut = False #AutoConfig not prevFlags.Reco.EnableHI
    flags.Calo.TopoCluster.extendTimeCut = False #anyway auto-configured on doTimeCut 
    flags.Calo.TopoCluster.useUpperLimitForTimeCut = False #anyway auto-configured on doTimeCut 
    flags.Reco.PostProcessing.ThinNegativeClusters = False #auto-config not prevFlags.Reco.EnableHI



def HImode(flags):
    _HIcommon(flags)
    flags.Reco.EnableTau = False #anyway dependent to EnableJet
    flags.DQ.Steering.doTauMon = False  #switch on DQType.HeavyIon FIXME might not work for UPC
    flags.Reco.EnableJet = False # ..and prevFlags.HIMode is not HIMode.HI
    flags.Reco.EnableMet = False  #anyway dependent to EnableJet
    flags.DQ.Steering.doMissingEtMon = False  #switch on DQType.HeavyIon FIXME might not work for UPC
    flags.Jet.WriteToAOD = True  #prevFlags.HIMode is HIMode.HI # this is to save btagging to xAOD
    flags.Calo.TopoCluster.skipWriteList = [
        "CaloCalTopoClusters", "CaloTopoClusters"] #if prevFlags.HIMode is HIMode.HI
    flags.Egamma.doForward = False  # and prevFlags.HIMode is not HIMode.HI)
    flags.Calo.FwdTower.WriteToAOD = False  # ..and prevFlags.HIMode is not HIMode.HI
    flags.HeavyIon.Egamma.doSubtractedClusters = True #prevFlags.HIMode is HIMode.HI
    flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection = True #lambda prevFlags: prevFlags.HIMode is HIMode.HI)
    flags.HeavyIon.Jet.HarmonicsForSubtraction = [2, 3, 4] #if prevFlags.HIMode is HIMode.HI else []))
    flags.HeavyIon.Jet.SeedPtMin = 25000 # if prevFlags.HIMode is HIMode.HI else 8000))
    flags.HeavyIon.Jet.RecoOutputPtMin = 25000 #if prevFlags.HIMode is HIMode.HI else 8000))
    flags.HeavyIon.Jet.TrackJetPtMin = 7000#if prevFlags.HIMode is HIMode.HI else 4000))


def _HIP_UPC_common(flags):
    _HIcommon(flags)
    flags.Reco.EnableTau = True  #anyway dependent of jet
    flags.Reco.EnableJet = True  #..and prevFlags.HIMode is not HIMode.HI
    flags.Reco.EnableMet = True  #anyway dependent of jet
    flags.Jet.WriteToAOD = True  # this is to save "standard" jets and btagging to xAOD prevFlags.Reco.HIMode in [HIMode.HI,HIMode.UPC,HIMode.HIP )
    flags.MET.WritetoAOD = True  #prevFlags.Reco.HIMode in [HIMode.HI,HIMode.UPC,HIMode.HIP )
    flags.HeavyIon.Egamma.doSubtractedClusters = False #lambda prevFlags: prevFlags.HIMode is HIMode.HI)
    flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection = False #lambda prevFlags: prevFlags.HIMode is HIMode.HI)
    flags.HeavyIon.Jet.HarmonicsForSubtraction = [] #if prevFlags.HIMode is HIMode.HI else
    flags.HeavyIon.Jet.SeedPtMin = 8000 #if prevFlags.HIMode is HIMode.HI else
    flags.HeavyIon.Jet.RecoOutputPtMin = 8000 #if prevFlags.HIMode is HIMode.HI else
    flags.HeavyIon.Jet.TrackJetPtMin = 4000 #if prevFlags.HIMode is HIMode.HI else


def HIPmode(flags):
    _HIP_UPC_common(flags)
    # HIP dedicated flags
    flags.Tracking.doHIP = True #Now AutoConfig


def UPCmode(flags):
    _HIP_UPC_common(flags)
    # UPC dedicated flags
    flags.Tracking.doUPC = True # prevFlags.Reco.HIMode is HIMode.UPC)
    flags.Egamma.doLowMu = True # prevFlags.Reco.HIMode is HIMode.UPC)
    flags.HeavyIon.Jet.WriteHIClusters = False #prevFlags.HIMode is not UPC
