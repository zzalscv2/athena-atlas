#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: master/Athena
# art-include: 23.0/Athena
# art-athena-mt: 8

Reco_tf.py --CA \
--AMI f1350  \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data" \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
--outputHISTFile="HIST.root" \
--outputDAOD_IDTIDEFile="DAOD_IDTIDE.pool.root" \
--outputDRAW_ZMUMUFile="myDRAW_ZMUMU.data" \
--outputDESDM_MCPFile="myDESDM_MCP.pool.root" \
--outputDRAW_EGZFile="myDRAW_EGZ.data" \
--outputDESDM_ALLCELLSFile="myDESDM_ALLCELLS.pool.root" \
--outputDAOD_L1CALO1File="myDAOD_L1CALO1.pool.root" \
--outputDESDM_PHOJETFile="myDESDM_PHOJET.pool.root" \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

# keep the job alive for up to 5 hours since diff-root doesn't display anything while running.
(for i in 1 1 1 1 1 ; do sleep 3600 ; touch ./my_art_heartbeat ; done) &
MY_ART_HEARTBEAT_PID=$!

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 100 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} (against previous nightly)"

kill $MY_ART_HEARTBEAT_PID || true
