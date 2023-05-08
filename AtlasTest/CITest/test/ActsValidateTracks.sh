#!/usr/bin/bash
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/inputs/ATLAS-P2-RUN4-01-01-00_ttbar_mu200.RDO.root
n_events=1

Reco_tf.py --CA \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile test.AOD.pool.root  \
  --maxEvents ${n_events}
