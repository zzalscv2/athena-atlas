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
--maxEvents=300 \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
--outputHISTFile="HIST.root" \
--imf False


rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 200 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} (against previous nightly)"

rc3=-9999
if [ ${rc1} -eq 0 ]
then
  art.py compare ref . /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3-22.0_references_for_comparison/test_bulkProcessing_13600collisions_SB_reduced \
  --entries 100 --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc3=$?
fi
echo  "art-result: ${rc3} (against reference)"
