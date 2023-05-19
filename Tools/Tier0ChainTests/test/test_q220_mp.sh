#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-include: 21.0/Athena
# art-include: 21.0-TrigMC/Athena
# art-include: master/Athena
# art-include: 22.0/Athena
# art-include: 22.0-mc20/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-athena-mt: 8
# art-output: log.*

Reco_tf.py \
--AMI=q220 \
--athenaopts='--nprocs=2' \
--maxEvents=500 \
--conditionsTag 'all:CONDBR2-BLKPA-RUN2-09' \
--outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --imf False


rc1=$?
echo "art-result: $rc1 Reco"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_22.0_references/$2
  art.py compare ref . $ArtRef --entries 20 --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} Comparison with the latest result"
