#!/bin/sh
#
# art-description: Express processing at Tier0
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-athena-mt: 8

# Not doing diff-root comparison for DAOD_IDTIDE at the moment (because 0 events are selected). To enable comparison for DAOD_IDTIDE, rename the output to DAOD_IDTIDE.pool.root

Reco_tf.py  \
--AMI x681  \
--inputBSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00423433.express_express.merge.RAW._lb0300._SFO-ALL._0001.1" \
--outputAODFile="AOD.pool.root" \
--outputESDFile="ESD.pool.root" \
--outputDAOD_L1CALO2File="L1CALO2.pool.root" \
--outputHISTFile="HIST.root" \
--outputDAOD_IDTIDEFile="DAOD_IDTIDE.root" \
--outputDRAW_ZMUMUFile="myDRAW_ZMUMU.data" \
--imf False

rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtPackage=$1
  ArtJobName=$2
  art.py compare grid --entries 30 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} Diff"

