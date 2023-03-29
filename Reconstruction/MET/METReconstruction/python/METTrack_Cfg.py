# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from METReconstruction.METRecoCfg import BuildConfig, RefConfig, METConfig,getMETRecoAlg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def METTrack_Cfg(configFlags):

    components = ComponentAccumulator()
    cfg_trk = METConfig('Track',configFlags,[BuildConfig('SoftTrk','Track')],
                    [RefConfig('TrackFilter','PVTrack')],
                    doTracks=configFlags.MET.UseTracks)
    cfg_trk.refiners['TrackFilter'].DoLepRecovery=True
    cfg_trk.refiners['TrackFilter'].DoVxSep=configFlags.MET.UseTracks
    cfg_trk.refiners['TrackFilter'].DoEoverPSel=True
    components.merge(cfg_trk.accumulator)

    recoAlg=getMETRecoAlg(algName='METRecoAlg_Track',configs={"Track":cfg_trk})
    components.addEventAlgo(recoAlg)
    return components
