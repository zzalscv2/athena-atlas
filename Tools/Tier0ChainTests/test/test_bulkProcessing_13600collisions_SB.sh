#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-athena-mt: 8

Reco_tf.py  \
--AMI f1259  \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00429782.physics_Main.daq.RAW._lb0562._SFO-11._0001.data" \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
--outputHISTFile="HIST.root" \
--outputDAOD_IDTIDEFile="DAOD_IDTIDE.pool.root" \
--outputDRAW_ZMUMUFile="myDRAW_ZMUMU.data" \
--outputDESDM_MCPFile="myDESDM_MCP.pool.root" \
--outputDRAW_EGZFile="myDRAW_EGZ.data" \
--outputDAOD_L1CALO5File="myDAOD_L1CALO5.pool.root" \
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

rc3=-9999
if [ ${rc1} -eq 0 ]
then
  ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3-22.0_references_for_comparison/test_bulkProcessing_13600collisions_SB
  cat $ArtRef/version.txt
  art.py compare ref . $ArtRef \
  --entries 10 --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc3=$?
fi
echo  "art-result: ${rc3} (against reference)"

kill $MY_ART_HEARTBEAT_PID || true
