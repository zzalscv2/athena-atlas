#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid
# art-output: log.*

Reco_tf.py \
--AMI=q220 \
--conditionsTag 'all:CONDBR2-BLKPA-RUN2-09' \
--athenaopts='--threads=1' \
--maxEvents=100 \
--outputAODFile=myAOD.pool.root --outputESDFile=myESD.pool.root --imf False

echo "art-result: $?"

rc2=-9999
if [ ${rc1} -eq 0 ]
then
  ArtRef=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/$1/TCT_22.0_references/$2
  art.py compare ref . $ArtRef --entries 30 --mode=semi-detailed --order-trees --ignore-exit-code diff-pool
  rc2=$?
fi
echo  "art-result: ${rc2} Comparison with the latest result"
