#!/usr/bin/bash
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=1

Reco_tf.py --CA \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --steering doRAWtoALL \
  --postExec 'from InDetGNNTracking.InDetGNNTrackingConfig import DumpObjectsCfg; cfg.merge(DumpObjectsCfg(flags))' \
  --preInclude 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude' \
  --maxEvents ${n_events}
