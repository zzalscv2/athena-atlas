#!/usr/bin/bash
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/inputs/ATLAS-P2-RUN4-01-01-00_ttbar_mu200.RDO.root
n_events=2

# Run reconstruction and produce AOD with persistified Acts EDM
Reco_tf.py --CA \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
  --postInclude "ActsConfig.ActsPostIncludes.PersistifyActsEDMCfg" \
  --preExec "flags.Acts.EDM.PersistifyClusters=True;flags.Acts.EDM.PersistifySpacePoints=True;" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events}

# Check we can retrieve the EDM, and related quantities, with our analysis algorithms
ActsReadEDM.py \
   --filesInput AOD.pool.root -- \
   readClusters=True \
   readSpacePoints=True

# Check we can run IDPVM
runIDPVM.py \
   --filesInput AOD.pool.root \
   --outputFile idpvm.root \
   --doActs
