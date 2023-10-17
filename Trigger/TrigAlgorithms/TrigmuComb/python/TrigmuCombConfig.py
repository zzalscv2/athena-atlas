#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#  This file configs the muComb reco alg with the newJO

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

muFastInfo="HLT_MuonL2SAInfo"
muCombInfo="HLT_MuonL2CBInfo"

def muCombCfg(flags, postFix="", useBackExtrp=True, L2StandAloneMuonContainerName = "", L2CombinedMuonContainerName="", TrackParticleContainerName=""):

    acc = ComponentAccumulator()

    # matching windows parameters tuned in 2016 and 2015 data (T&P Z  and J/psi samples)
    winEtaSigma = 7.0
    winPhiSigma = 7.0
    chi2Weight  = 2.0
        
    # pt resolution parameters for ID-SAmuon match from SAmuon developers (2016 data)
    if flags.Detector.EnableCSC:
        idScanBarrelRes  = [0.02169,0.0004186]
        idScanEndcap1Res = [0.03054,0.000325]
        idScanEndcap2Res = [0.03557,0.0005383]
        idScanEndcap3Res = [0.04755,0.0007718]
        idScanEndcap4Res = [0.06035,0.0001145]
    else:
        idScanBarrelRes = [0.017, 0.000000418]
        idScanEndcap1Res = [0.025, 0.0000002]
        idScanEndcap2Res = [0.030, 0.0000002]
        idScanEndcap3Res = [0.036, 0.0000004]
        idScanEndcap4Res = [0.046, 0.0000002]
    from TrigmuComb.TrigmuCombMonitoring import TrigMuCombMonitoring
    muCombAlg = CompFactory.muComb(
                       name                  = "MuComb"+postFix,
                       MuCombStrategy        = 0,
                       UseBackExtrapolatorG4 = useBackExtrp,
                       MinPtTRK              = 0.,
                       WinEtaSigma_g4        = winEtaSigma,
                       WinPhiSigma_g4        = winPhiSigma,
                       Chi2Weight_g4         = chi2Weight,
                       IDSCANBarrelRes       = idScanBarrelRes,
                       IDSCANEndcap1Res      = idScanEndcap1Res,
                       IDSCANEndcap2Res      = idScanEndcap2Res,
                       IDSCANEndcap3Res      = idScanEndcap3Res,
                       IDSCANEndcap4Res      = idScanEndcap4Res,
                       IDalgo                = "InDetTrigTrackingxAODCnv_Muon_FTF",
                       L2StandAloneMuonContainerName = L2StandAloneMuonContainerName,
                       L2CombinedMuonContainerName = L2CombinedMuonContainerName,
                       TrackParticlesContainerName = TrackParticleContainerName,
                       MonTool = TrigMuCombMonitoring(flags))

    acc.addEventAlgo(muCombAlg)

    return acc

