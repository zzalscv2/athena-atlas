#!/usr/bin/env bash
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
set -e

# Compare reco outputs of legacy and CA config
xAODDigest.py --extravars "../${1}/run_${2}/myAOD.pool.root" "${1}_${2}_digest_legacy.txt"
xAODDigest.py --extravars "../${1}_Legacy/run_${2}/myAOD.pool.root" "${1}_${2}_digest_CA.txt"

comparexAODDigest.py "${1}_${2}_digest_legacy.txt" "${1}_${2}_digest_CA.txt" 

acmd.py diff-root "../${1}_Legacy/run_${2}/myAOD.pool.root" "../${1}/run_${2}/myAOD.pool.root" \
  --nan-equal --error-mode resilient --ignore-leaves index_ref \
  TrigNavigationAux EventInfoAux \
  MuonTruthParticlesAuxDyn \
  --order-trees --entries 10 --mode semi-detailed
