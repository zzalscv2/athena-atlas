#!/usr/bin/env bash
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
set -e

# Compare reco outputs of legacy and CA config
xAODDigest.py --extravars "../${1}/run_${2}/myAOD.pool.root" "${1}_${2}_digest_legacy.txt"
xAODDigest.py --extravars "../${1}_CAConfig/run_${2}/myAOD.pool.root" "${1}_${2}_digest_CA.txt"

comparexAODDigest.py "${1}_${2}_digest_legacy.txt" "${1}_${2}_digest_CA.txt" --ignoreMuons --ignoreMET


acmd.py diff-root "../${1}/run_${2}/myAOD.pool.root" "../${1}_CAConfig/run_${2}/myAOD.pool.root" --nan-equal --error-mode resilient --ignore-leaves RecoTimingObj_p1_HITStoRDO_timings RecoTimingObj_p1_RAWtoESD_mems RecoTimingObj_p1_RAWtoESD_timings RAWtoESD_mems RAWtoESD_timings ESDtoAOD_mems ESDtoAOD_timings HITStoRDO_timings RAWtoALL_mems RAWtoALL_timings RecoTimingObj_p1_RAWtoALL_mems RecoTimingObj_p1_RAWtoALL_timings RecoTimingObj_p1_EVNTtoHITS_timings index_ref MuonsLRTAux MuonTruthParticlesAuxDyn MSOnlyExtrapolatedMuonTrackParticlesAux ExtrapolatedMuonTrackParticlesAux CombinedMuonTrackParticlesAux ExtraPolatedMuonsLRTTrackParticlesAux EMEO_CombinedMuonTrackParticlesAux EMEO_ExtrapolatedMuonTrackParticlesAux CombinedMuonsLRTTrackParticlesAux CombinedStauTrackParticlesAux EMEO_MSOnlyExtrapolatedMuonTrackParticlesAux SlowMuonsAux InDetForwardTrackParticlesAux TrigNavigationAux EventInfo --order-trees --entries 5 --mode semi-detailed
