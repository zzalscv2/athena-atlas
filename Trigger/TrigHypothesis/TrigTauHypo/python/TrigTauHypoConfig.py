# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


@AccumulatorCache
def tauCaloRoiUpdaterCfg(flags, inputRoIs, clusters):
    acc = ComponentAccumulator()
    alg                               = CompFactory.TrigTauCaloRoiUpdater("TauCaloRoiUpdater",
                                        RoIInputKey     = inputRoIs,
                                        RoIOutputKey    = 'UpdatedCaloRoI',
                                        CaloClustersKey = clusters)
    acc.addEventAlgo(alg)
    return acc

@AccumulatorCache
def tauTrackRoiUpdaterCfg(flags, inputRoIs, tracks):
    acc = ComponentAccumulator()
    newflags = flags.Trigger.InDetTracking.tauIso
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdater",
                                        etaHalfWidth                  = newflags.etaHalfWidth,
                                        phiHalfWidth                  = newflags.phiHalfWidth,
                                        z0HalfWidth                   = newflags.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackRoI",
                                        fastTracksKey                 = tracks,
                                        Key_trigTauJetInputContainer  = "" )
    acc.addEventAlgo(alg)
    return acc

@AccumulatorCache
def tauTrackBDTRoiUpdaterCfg(flags, inputRoIs, tracks):
    acc                               = ComponentAccumulator()
    newflags = flags.Trigger.InDetTracking.tauIsoBDT
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdaterBDT",
                                        etaHalfWidth                  = newflags.etaHalfWidth,
                                        phiHalfWidth                  = newflags.phiHalfWidth,
                                        z0HalfWidth                   = newflags.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackBDTRoI",
                                        fastTracksKey                 = tracks,
                                        BDTweights                    = f"{flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath}/{flags.Trigger.Offline.Tau.FTFTauCoreBDTConfig}",
                                        Key_trigTauJetInputContainer  = "HLT_TrigTauRecMerged_CaloMVAOnly" )
    acc.addEventAlgo(alg)
    return acc

@AccumulatorCache
def tauLRTRoiUpdaterCfg(flags, inputRoIs, tracks):
    acc                               = ComponentAccumulator()
    newflags = flags.Trigger.InDetTracking.tauLRT
    alg                               = CompFactory.TrigTauTrackRoiUpdater("TrackRoiUpdaterLRT",
                                        etaHalfWidth                  = newflags.etaHalfWidth,
                                        phiHalfWidth                  = newflags.phiHalfWidth,
                                        z0HalfWidth                   = newflags.zedHalfWidth,
                                        RoIInputKey                   = inputRoIs,
                                        RoIOutputKey                  = "UpdatedTrackLRTRoI",
                                        fastTracksKey                 = tracks,
                                        Key_trigTauJetInputContainer  = "" )
    acc.addEventAlgo(alg)
    return acc
