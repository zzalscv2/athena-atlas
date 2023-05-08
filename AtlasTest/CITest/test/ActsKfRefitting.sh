#!/usr/bin/bash
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/inputs/ATLAS-P2-RUN4-01-01-00_ttbar_mu200.RDO.root
n_events=5

Reco_tf.py --CA \
   --detectors Bpipe ITkPixel ITkStrip \
   --preExec "flags.Reco.EnableHGTDExtension=False;" \
   --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
   --postInclude "ActsConfig.ActsTrkFittingAlgsConfig.ActsReFitterAlgCfg" \
   --inputRDOFile ${input_rdo} \
   --outputESDFile ESD.pool.root \
   --outputAODFile AOD.pool.root \
   --maxEvents ${n_events}
